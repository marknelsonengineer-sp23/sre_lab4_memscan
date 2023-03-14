///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get more information about a virtual address
///
/// @see http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html?highlight=pagemap
/// @see https://docs.huihoo.com/doxygen/linux/kernel/3.7/swap_8h_source.html
///
/// @NOLINTBEGIN( readability-magic-numbers ):  Due to the nature of this module, we will allow magic numbers
///
/// @file   pagemap.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

/// Enables declaration of `pread()`
///
/// @see https://man7.org/linux/man-pages/man3/read.3p.html
/// @NOLINTNEXTLINE(bugprone-reserved-identifier, cert-dcl37-c, cert-dcl51-cpp): This is a legitimate use of a reserved identifier
#define _XOPEN_SOURCE 700

#include <assert.h>       // For assert()
#include <fcntl.h>        // For open() O_RDONLY
#include <inttypes.h>     // For PRIu64
#include <stdint.h>       // For uint64_t
#include <stdio.h>        // For printf()
#include <stdlib.h>       // For exit() EXIT_FAILURE
#include <string.h>       // For memset()
#include <sys/syscall.h>  // Definition of SYS_* constants
#include <unistd.h>       // For sysconf() close()

#include "colors.h"       // For ANSI_COLOR_...
#include "config.h"       // For getProgramName()
#include "iomem.h"        // For get_iomem_region_description()
#include "pagecount.h"    // For getPagecount() closePagecount()
#include "pageflags.h"    // For setPageflags() closePageflags()
#include "pagemap.h"      // Just cuz
#include "shannon.h"      // For computeShannonEntropy() getShannonClassification()
#include "version.h"      // For STRINGIFY_VALUE()

/// The `pagemap` file we intend to read from `/proc`
#define PAGEMAP_FILE "/proc/self/pagemap"

/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt),
/// each pagemap entry is `8` bytes long
#define PAGEMAP_ENTRY 8


/// A static file descriptor to #PAGEMAP_FILE (or `-1` if it hasn't been set yet)
///
/// The static file descriptor allows us to efficiently keep #PAGEMAP_FILE open.
/// It's closed in closePagemap()
static int pagemap_fd = -1 ;


inline size_t getPageSizeInBytes() {
   return sysconf( _SC_PAGESIZE ) ;
}


inline unsigned char getPageSizeInBits() {
   /// The pagesize must be a power of 2 (this is what allows us to access the
   /// interior of a page).  This function finds out which bit (which power of 2)
   /// it is.  `FFS` is meant to be used with signed numbers (not what we want), so
   /// we will use `CLZ` which counts leading zeros.
   ///
   /// @see https://en.wikipedia.org/wiki/Find_first_set
   /// @see https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
   ///
   /// @internal
   /// - Start with the size of an address in bits
   /// - Subtract the number of leading 0s
   /// - Subtract one more to get the index of the first bit

   return ( sizeof( size_t ) << 3 ) - __builtin_clzl( getPageSizeInBytes() - 1 ) ;
}


struct PageInfo getPageInfo( void* vAddr ) {
   /// @NOLINTBEGIN( performance-no-int-to-ptr ):  This function mixes `void*` and `size_t`.  C normally warns about this, but in this case, it's OK.
// printf( "%p\n", vAddr ) ;

   struct PageInfo page = {} ;
   memset( &page, 0, sizeof( page ) ) ;

   page.virtualAddress = vAddr ;

   /// Scan for Shannon entropy before reading `pagemap`, `pageflags` and `pagecount`
   if( scanForShannon ) {
      page.shannon = computeShannonEntropy( page.virtualAddress, getPageSizeInBytes() ) ;
   }

   off_t pagemap_offset = (long) ((size_t) vAddr / getPageSizeInBytes() * PAGEMAP_ENTRY ) ;

   if( pagemap_fd < 0 ) {
      pagemap_fd = open( PAGEMAP_FILE, O_RDONLY ) ;
      if( pagemap_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", PAGEMAP_FILE );
      }
   }

   uint64_t pagemap_data;

   page.valid = true ;
   // There's some risk here... some pread functions may return something between
   // 1 and 7 bytes, and we'd continue the read.  There's examples of how to
   // do this in our GitHub history, but I'm simplifying the code for now and
   // just requesting a single 8 byte read -- take it or leave it.
   ssize_t ret = pread(pagemap_fd                  // File descriptor
                      ,((uint8_t*) &pagemap_data)  // Destination buffer
                      ,PAGEMAP_ENTRY               // Bytes to read
                      ,pagemap_offset );           // Read data from this offset  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
   if( ret != PAGEMAP_ENTRY ) {
      page.valid = false ;
      printf( "Unable to read[%s] for [%p]\n", PAGEMAP_FILE, vAddr ) ;
   }

   if( page.valid ) {
      page.swapped = GET_BIT( pagemap_data, 62 );

      if( page.swapped ) {
         page.swap_type = pagemap_data & 0b000011111 ; // Bits 0-4
         page.swap_offset = (void*) ( ( pagemap_data & 0x007FFFFFFFFFFFC0 ) >> 5 ) ; // Bits 5-54 >> 5 bits
      } else {
         page.pfn = (void*) ( pagemap_data & 0x7FFFFFFFFFFFFF );  // Bits 0-54
      }

      page.soft_dirty = GET_BIT( pagemap_data, 55 );
      page.exclusive = GET_BIT( pagemap_data, 56 );
      assert( GET_BIT( pagemap_data, 57 ) == 0 );
      assert( GET_BIT( pagemap_data, 58 ) == 0 );
      assert( GET_BIT( pagemap_data, 59 ) == 0 );
      assert( GET_BIT( pagemap_data, 60 ) == 0 );
      page.file_mapped = GET_BIT( pagemap_data, 61 );
      page.present = GET_BIT( pagemap_data, 63 );
      page.page_count = getPagecount( page.pfn ) ;
      setPageflags( &page ) ;
   }

   return( page ) ;
}  // getPageInfo    /// @NOLINTEND( performance-no-int-to-ptr )


void printPageInfo( const struct PageInfo* page ) {
   assert( page != NULL );

   printf( ANSI_COLOR_GREEN "    %p  " ANSI_COLOR_RESET, page->virtualAddress ) ;

   if( !page->valid ) {
      printf( ANSI_COLOR_RED " virtual address was not read by pagemap" ANSI_COLOR_RESET ) ;
   }

   if( !page->present && !page->swapped ) {
      printf( ANSI_COLOR_RED " page not present" ANSI_COLOR_RESET ) ;
   }

   if( page->swapped ) {
      printf( ANSI_COLOR_RED " swapped: " ANSI_COLOR_RESET );
      printf( "type: %u  ", page->swap_type );
      printf( "offset: 0x%p  ", page->swap_offset );
   }

   if( page->present && !page->swapped ) {
      printf( " pfn: " ANSI_COLOR_GREEN "0x%07zu " ANSI_COLOR_RESET, (size_t) page->pfn ) ;
      printf( "#:%3" PRIu64 " ", page->page_count ) ;
      printf( " Flags: " ) ;
      printf( ANSI_COLOR_YELLOW   "%s" ANSI_COLOR_RESET, page->soft_dirty  ? "S-D" : "   " ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->exclusive   ? 'X' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->file_mapped ? 'F' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%s" ANSI_COLOR_RESET, page->ksm         ? "KSM" : "   " ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->zero_page   ? '0' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->locked      ? 'L' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->slab        ? 'S' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->writeback   ? 'W' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->buddy       ? 'B' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->idle        ? 'I' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->pgtable     ? 'P' : ' ' ) ;
      printf( "\\" ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->huge        ? 'H' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->thp         ? 'T' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->comp_head   ? '<' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->comp_tail   ? '>' : ' ' ) ;
      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->balloon     ? 'B' : ' ' ) ;

      printf( " IO:" ) ;
      printf( ANSI_COLOR_RED      "%c" ANSI_COLOR_RESET, page->error       ? '!' : ' ' ) ;
      printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->uptodate    ? 'U' : ' ' ) ;
      printf( ANSI_COLOR_YELLOW   "%c" ANSI_COLOR_RESET, page->dirty       ? 'D' : ' ' ) ;

      printf( "LRU:" ) ;
      printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->lru         ? 'L' : ' ' ) ;
      printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->active      ? 'A' : ' ' ) ;
      printf( ANSI_COLOR_MAGENTA  "%c" ANSI_COLOR_RESET, page->unevictable ? 'U' : ' ' ) ;
      printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->referenced  ? 'R' : ' ' ) ;
      printf( ANSI_COLOR_YELLOW   "%c" ANSI_COLOR_RESET, page->reclaim     ? 'R' : ' ' ) ;
      printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->mmap        ? 'M' : ' ' ) ;
      printf( ANSI_COLOR_YELLOW   "%c" ANSI_COLOR_RESET, page->anon        ? 'A' : ' ' ) ;
      printf( ANSI_COLOR_YELLOW   "%c" ANSI_COLOR_RESET, page->swapcache   ? 'C' : ' ' ) ;
      printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->swapbacked  ? 'B' : ' ' ) ;

      printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->nopage      ? 'N' : ' ' ) ;
      printf( ANSI_COLOR_RED      "%c" ANSI_COLOR_RESET, page->hwpoison    ? '!' : ' ' ) ;

      printf( ANSI_COLOR_BRIGHT_CYAN "  %s " ANSI_COLOR_RESET, get_iomem_region_description( (void*) page->pfn ) ) ;

      if( scanForShannon ) {
         printf( "H: %5.3lf ", page->shannon ) ;
         printf( "%-" STRINGIFY_VALUE( MAX_SHANNON_CLASSIFICATION_LENGTH ) "s", getShannonClassification( page->shannon ) ) ;
      }
   }

   printf( "\n" ) ;
}


void closePagemap() {
   /// Pagemap holds some files open (like #pagemap_fd) in static variables.
   /// Close the files properly.
   if( pagemap_fd != -1 ) {
      int closeStatus = close( pagemap_fd ) ;
      pagemap_fd = -1 ;
      if( closeStatus != 0 ) {
         FATAL_ERROR( "Unable to close [%s]", PAGEMAP_FILE );
      }
   }

   closePagecount() ;
   closePageflags() ;
}


void printKey( FILE* outStream ) {
   PRINT_USAGE( outStream, "Key for --phys option\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "Each line starts with the virtual address for each page.\n" ) ;
   PRINT_USAGE( outStream, "If the page is swapped, it will print swapped with the type and offset.\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "pfn: Page Frame Number.  Essentially, the physical page.\n" ) ;
   PRINT_USAGE( outStream, "#:   the number of times each page is mapped\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "Flags:\n" ) ;
   PRINT_USAGE( outStream, "S-D: page is soft-dirty (it's been written to recently)\n" ) ;
   PRINT_USAGE( outStream, "X:   page exclusively mapped\n" ) ;
   PRINT_USAGE( outStream, "F:   page is file mapped and not anonymously mapped\n" ) ;
   PRINT_USAGE( outStream, "KSM: kernel samepage merging: Identical memory pages are dynamically shared\n" ) ;
   PRINT_USAGE( outStream, "     between processes\n" ) ;
   PRINT_USAGE( outStream, "0:   zero page for pfn_zero or huge_zero page\n" ) ;
   PRINT_USAGE( outStream, "L:   locked for exclusive access, e.g. by undergoing read/write IO\n" ) ;
   PRINT_USAGE( outStream, "S:   page is managed by the SLAB/SLOB/SLUB/SLQB kernel memory allocator\n" ) ;
   PRINT_USAGE( outStream, "W:   writeback: page is being synced to disk\n" ) ;
   PRINT_USAGE( outStream, "B:   a free memory block managed by the buddy system allocator\n" ) ;
   PRINT_USAGE( outStream, "I:   idle: The page has not been accessed since it was marked idle\n" ) ;
   PRINT_USAGE( outStream, "P:   the page is in use as a page table\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "\\    the end of regular flags and the start of huge page flags\n" ) ;
   PRINT_USAGE( outStream, "H:   huge TLB page\n" ) ;
   PRINT_USAGE( outStream, "T:   transparent huge page:  Contiguous pages that form a huge allocation\n" ) ;
   PRINT_USAGE( outStream, "<:   the head of a contiguous block of pages\n" ) ;
   PRINT_USAGE( outStream, ">:   the tail of a contiguous block of pages\n" ) ;
   PRINT_USAGE( outStream, "B:   page is tagged for balloon compaction\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "IO:  I/O flags\n" ) ;
   PRINT_USAGE( outStream, "!:   IO error occurred\n" ) ;
   PRINT_USAGE( outStream, "U:   page has up-to-date data for file backed page\n" ) ;
   PRINT_USAGE( outStream, "D:   page page has been written to and contains new data\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "LRU: least recently used flags\n" ) ;
   PRINT_USAGE( outStream, "L:   page is in one of the LRU lists\n" ) ;
   PRINT_USAGE( outStream, "A:   active: page is in the active LRU list\n" ) ;
   PRINT_USAGE( outStream, "U:   unevictable: It is pinned and not a candidate for reclamation\n" ) ;
   PRINT_USAGE( outStream, "R:   referenced: page has been referenced since last LRU list enqueue/requeue\n" ) ;
   PRINT_USAGE( outStream, "R:   reclaim: page will be reclaimed soon after its pageout IO completed\n" ) ;
   PRINT_USAGE( outStream, "M:   memory mapped page\n" ) ;
   PRINT_USAGE( outStream, "A:   anonymous: memory mapped page that is not part of a file\n" ) ;
   PRINT_USAGE( outStream, "C:   swapCache: page is mapped to swap space, i.e. has an associated swap entry\n" ) ;
   PRINT_USAGE( outStream, "B:   swapBacked: page is backed by swap/RAM\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "Rarely seen flags\n" ) ;
   PRINT_USAGE( outStream, "N:   no page frame exists at the requested address\n" ) ;
   PRINT_USAGE( outStream, "!:   hardware detected memory corruption: Don't touch this page\n" ) ;
}

/// NOLINTEND( readability-magic-numbers )
