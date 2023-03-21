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

#include <fcntl.h>        // For open() O_RDONLY
#include <inttypes.h>     // For PRIu64
#include <stdint.h>       // For uint64_t
#include <stdio.h>        // For printf()
#include <stdlib.h>       // For exit() EXIT_FAILURE
#include <string.h>       // For memset() strncmp()
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


struct PageInfo getPageInfo( void* vAddr, bool okToRead ) {
   /// @NOLINTBEGIN( performance-no-int-to-ptr ):  This function mixes `void*` and `size_t`.  C normally warns about this, but in this case, it's OK.
// printf( "%p\n", vAddr ) ;

   struct PageInfo page = {} ;
   memset( &page, 0, sizeof( page ) ) ;

   page.virtualAddress = vAddr ;

   /// Scan for Shannon entropy before reading `pagemap`, `pageflags` and `pagecount`
   if( okToRead && scanForShannon ) {
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
      ASSERT( GET_BIT( pagemap_data, 57 ) == 0 );
      ASSERT( GET_BIT( pagemap_data, 58 ) == 0 );
      ASSERT( GET_BIT( pagemap_data, 59 ) == 0 );
      ASSERT( GET_BIT( pagemap_data, 60 ) == 0 );
      page.file_mapped = GET_BIT( pagemap_data, 61 );
      page.present = GET_BIT( pagemap_data, 63 );
      page.page_count = getPagecount( page.pfn ) ;
      setPageflags( &page ) ;
   }

   return( page ) ;
}  // getPageInfo    /// @NOLINTEND( performance-no-int-to-ptr )


/// Print the virtual address starting address for both `--pfn` and `--phys`
///
/// @param page The page to print
void printVirtualAddressStart( const struct PageInfo* page ) {
   ASSERT( page != NULL ) ;

   printf( ANSI_COLOR_GREEN "    %p - " ANSI_COLOR_RESET, page->virtualAddress ) ;
}


/// Print the generic information about a page
///
/// @param page The page to print
void printPageFlags( const struct PageInfo* page ) {
   printf( "Flags: " ) ;
   printf( ANSI_COLOR_YELLOW   "%c" ANSI_COLOR_RESET, page->soft_dirty  ? '*' : ' ' ) ;
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
      printf( ANSI_COLOR_BRIGHT_YELLOW ) ;
      printf( "H: %5.3lf ", page->shannon ) ;
      printf( "%-" STRINGIFY_VALUE( MAX_SHANNON_CLASSIFICATION_LENGTH ) "s", getShannonClassification( page->shannon ) ) ;
      printf( ANSI_COLOR_RESET ) ;
   }
} // printPageFlags


/// Print the latter half of the virtual address or PFN and any
/// message that's not a flag
///
/// @param page The page to print
/// @return `true` if flags should be printed.  `false` if flags should not
///         be printed.
bool printPageContext( const struct PageInfo* page ) {
   ASSERT( page != NULL );

   if( includePhysicalPageSummary ) {
      /// @todo Optimize this a bit, so we don't have to call getPageSizeInBytes() every time... Maybe make it a global/inline
      printf( ANSI_COLOR_GREEN "%p " ANSI_COLOR_RESET, page->virtualAddress + getPageSizeInBytes() - 1 ) ;
   }

   if( !page->valid ) {
      printf( ANSI_COLOR_RED "virtual address was not read by pagemap" ANSI_COLOR_RESET ) ;
      return false ;
   }

   if( !page->present && !page->swapped ) {
      printf( ANSI_COLOR_RED "page not present" ANSI_COLOR_RESET ) ;
      return false ;
   }

   if( page->swapped ) {
      printf( ANSI_COLOR_RED "swapped: " ANSI_COLOR_RESET );
      printf( "type: %u  ", page->swap_type );
      printf( "offset: 0x%p  ", page->swap_offset );
      return false ;
   }

   if( includePhysicalPageNumber && page->present && !page->swapped ) {
      printf( "pfn: " ANSI_COLOR_GREEN "0x%07zu " ANSI_COLOR_RESET, (size_t) page->pfn ) ;
      printf( "#:%3" PRIu64 " ", page->page_count ) ;
   }

   return true ;
} // printPageContext


void printFullPhysicalPage( const struct PageInfo* page ) {
   ASSERT( page != NULL );

   printVirtualAddressStart( page ) ;
   if( printPageContext( page ) ) {
      printPageFlags( page ) ;
   }

   printf( "\n" ) ;
} // printFullPhysicalPage


/// The potential result of comparing two physical pages
enum PageCompare {
    SAME_PAGE_SAME_FLAGS            ///< The same page and the same flags
   ,SAME_PAGE_DIFFERENT_FLAGS       ///< The same page and different flags
   ,DIFFERENT_PAGE_SAME_FLAGS       ///< A different page (probably the next page) and the same flags
   ,DIFFERENT_PAGE_DIFFERENT_FLAGS  ///< A different page (probably the next page) and different flags
} ;


/// Compare two pages
///
/// Right now, this is just used for `--phys` to summarize / combine identical
/// into one row.  In the future, this can be used to compare a parent's
/// physical page map with a child processes' physical page map.
///
/// @param left The first page to compare
/// @param right The second page to compare
/// @return Look at both the flags (metadata) and the virtual page address to
///         report how the pages are similar or different
enum PageCompare comparePages( const struct PageInfo* left, const struct PageInfo* right ) {
   ASSERT( left != NULL ) ;
   ASSERT( right != NULL ) ;

   bool same = true ;

   // @todo this code is not working right
   // If !present & !swapped, then ignore the other flags
   if(    !left->present
       && !right->present
       && !left->swapped
       && !right->swapped ) {
      same = true ;
      goto Done ;
   }

   if( left->valid       != right->valid )       { same = false ; goto Done ; }
   if( left->swap_type   != right->swap_type )   { same = false ; goto Done ; }
   if( left->swap_offset != right->swap_offset ) { same = false ; goto Done ; }
   if( left->soft_dirty  != right->soft_dirty )  { same = false ; goto Done ; }
   if( left->exclusive   != right->exclusive )   { same = false ; goto Done ; }
   if( left->file_mapped != right->file_mapped ) { same = false ; goto Done ; }
   if( left->swapped     != right->swapped )     { same = false ; goto Done ; }
   if( left->present     != right->present )     { same = false ; goto Done ; }
   if( left->locked      != right->locked )      { same = false ; goto Done ; }
   if( left->error       != right->error )       { same = false ; goto Done ; }
   if( left->referenced  != right->referenced )  { same = false ; goto Done ; }
   if( left->uptodate    != right->uptodate )    { same = false ; goto Done ; }
   if( left->lru         != right->lru )         { same = false ; goto Done ; }
   if( left->active      != right->active )      { same = false ; goto Done ; }
   if( left->slab        != right->slab )        { same = false ; goto Done ; }
   if( left->writeback   != right->writeback )   { same = false ; goto Done ; }
   if( left->reclaim     != right->reclaim )     { same = false ; goto Done ; }
   if( left->buddy       != right->buddy )       { same = false ; goto Done ; }
   if( left->mmap        != right->mmap )        { same = false ; goto Done ; }
   if( left->anon        != right->anon )        { same = false ; goto Done ; }
   if( left->swapcache   != right->swapcache )   { same = false ; goto Done ; }
   if( left->swapbacked  != right->swapbacked )  { same = false ; goto Done ; }
   if( left->comp_head   != right->comp_head )   { same = false ; goto Done ; }
   if( left->comp_tail   != right->comp_tail )   { same = false ; goto Done ; }
   if( left->huge        != right->huge )        { same = false ; goto Done ; }
   if( left->unevictable != right->unevictable ) { same = false ; goto Done ; }
   if( left->hwpoison    != right->hwpoison )    { same = false ; goto Done ; }
   if( left->nopage      != right->nopage )      { same = false ; goto Done ; }
   if( left->ksm         != right->ksm )         { same = false ; goto Done ; }
   if( left->thp         != right->thp )         { same = false ; goto Done ; }
   if( left->balloon     != right->balloon )     { same = false ; goto Done ; }
   if( left->zero_page   != right->zero_page )   { same = false ; goto Done ; }
   if( left->idle        != right->idle )        { same = false ; goto Done ; }
   if( left->pgtable     != right->pgtable )     { same = false ; goto Done ; }

   // Make sure PFN Regions are the same
   if( strncmp( get_iomem_region_description( left->pfn )
               ,get_iomem_region_description( right->pfn )
               ,MAX_IOMEM_DESCRIPTION ) != 0 ) { same = false ; goto Done ; }

   Done:
   if( same ) {
      if( left->virtualAddress == right->virtualAddress ) {
         return SAME_PAGE_SAME_FLAGS ;
      }
      return DIFFERENT_PAGE_SAME_FLAGS ;
   } else {
      if( left->virtualAddress == right->virtualAddress ) {
         return SAME_PAGE_DIFFERENT_FLAGS ;
      }
      return DIFFERENT_PAGE_DIFFERENT_FLAGS ;
   }
} // comparePages


void printPageSummary( const struct PageInfo page[], const size_t numPages ) {
   ASSERT( numPages > 0 ) ;

   printVirtualAddressStart( &page[0] ) ;

   for( size_t i = 0 ; i < numPages-1 ; i++ ) {
      if( comparePages( &page[i], &page[i+1] ) == DIFFERENT_PAGE_SAME_FLAGS ) {
         continue ;
      }

      if( printPageContext( &page[i] ) ) {
         printPageFlags( &page[i] ) ;
      }

      printf( "\n" ) ;

      printVirtualAddressStart( &page[i+1] ) ;
   }

   // Print the last page
   if( printPageContext( &page[numPages-1] ) ) {
      printPageFlags( &page[numPages-1] ) ;
   }
   printf( "\n" ) ;
} // printPageSummary


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
   PRINT( outStream, "memscan --phys will output physical pages like this:\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "0x55a5be863000 - 0x55a5be864fff page not present\n" ) ;
   PRINT( outStream, "0x55a5be865000 - 0x55a5be8abfff Flags: *X           \\      IO: U LRU:L    MA B    System RAM\n" ) ;
   PRINT( outStream, "0x55a5be8ac000 - 0x55a5be8adfff Flags: *X           \\      IO: U LRU: A   MA B    System RAM\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "memscan --pfn will output physical pages like this:\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "0x7ffca078e000 - pfn: 0x0279672 #:  1 Flags: *X           \\      IO: U LRU:L    MA B    System RAM H: 0.564 Very low entropy\n" ) ;
   PRINT( outStream, "0x7ffca078f000 - pfn: 0x0214113 #:  1 Flags: *X           \\      IO: U LRU:L    MA B    System RAM H: 3.082 English text in UNICODE\n" ) ;
   PRINT( outStream, "0x7ffca0790000 - pfn: 0x0118352 #:  1 Flags: *X           \\      IO: U LRU:L    MA B    System RAM H: 1.190 Unknown\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "Each line starts with the virtual address region of the physical page.\n" ) ;
   PRINT( outStream, "It will report high-level messages like \"page not present\" or \"page swapped\".\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "pfn: Page Frame Number.  Essentially, the physical page.\n" ) ;
   PRINT( outStream, "#:   the number of times each page is mapped\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "Flags:\n" ) ;
   PRINT( outStream, "*:   page is soft-dirty (it's been written to recently)\n" ) ;
   PRINT( outStream, "X:   page exclusively mapped\n" ) ;
   PRINT( outStream, "F:   page is file mapped and not anonymously mapped\n" ) ;
   PRINT( outStream, "KSM: kernel samepage merging: Identical memory pages are dynamically shared\n" ) ;
   PRINT( outStream, "     between processes\n" ) ;
   PRINT( outStream, "0:   zero page for pfn_zero or huge_zero page\n" ) ;
   PRINT( outStream, "L:   locked for exclusive access, e.g. by undergoing read/write IO\n" ) ;
   PRINT( outStream, "S:   page is managed by the SLAB/SLOB/SLUB/SLQB kernel memory allocator\n" ) ;
   PRINT( outStream, "W:   writeback: page is being synced to disk\n" ) ;
   PRINT( outStream, "B:   a free memory block managed by the buddy system allocator\n" ) ;
   PRINT( outStream, "I:   idle: The page has not been accessed since it was marked idle\n" ) ;
   PRINT( outStream, "P:   the page is in use as a page table\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "\\    the end of regular flags and the start of huge page flags\n" ) ;
   PRINT( outStream, "H:   huge TLB page\n" ) ;
   PRINT( outStream, "T:   transparent huge page:  Contiguous pages that form a huge allocation\n" ) ;
   PRINT( outStream, "<:   the head of a contiguous block of pages\n" ) ;
   PRINT( outStream, ">:   the tail of a contiguous block of pages\n" ) ;
   PRINT( outStream, "B:   page is tagged for balloon compaction\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "IO:  I/O flags\n" ) ;
   PRINT( outStream, "!:   IO error occurred\n" ) ;
   PRINT( outStream, "U:   page has up-to-date data for file backed page\n" ) ;
   PRINT( outStream, "D:   page page has been written to and contains new data\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "LRU: least recently used flags\n" ) ;
   PRINT( outStream, "L:   page is in one of the LRU lists\n" ) ;
   PRINT( outStream, "A:   active: page is in the active LRU list\n" ) ;
   PRINT( outStream, "U:   unevictable: It is pinned and not a candidate for reclamation\n" ) ;
   PRINT( outStream, "R:   referenced: page has been referenced since last LRU list enqueue/requeue\n" ) ;
   PRINT( outStream, "R:   reclaim: page will be reclaimed soon after its pageout IO completed\n" ) ;
   PRINT( outStream, "M:   memory mapped page\n" ) ;
   PRINT( outStream, "A:   anonymous: memory mapped page that is not part of a file\n" ) ;
   PRINT( outStream, "C:   swapCache: page is mapped to swap space, i.e. has an associated swap entry\n" ) ;
   PRINT( outStream, "B:   swapBacked: page is backed by swap/RAM\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "Rarely seen flags\n" ) ;
   PRINT( outStream, "N:   no page frame exists at the requested address\n" ) ;
   PRINT( outStream, "!:   hardware detected memory corruption: Don't touch this page\n" ) ;
} // printKey

/// NOLINTEND( readability-magic-numbers )
