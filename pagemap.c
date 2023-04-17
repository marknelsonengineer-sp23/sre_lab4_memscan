///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get more information about a virtual address
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     This file lets a userspace process find out which physical frame each
///     virtual page is mapped to.  It contains one 64-bit value for each
///     virtual page.
///
///     * Bits 0-54  page frame number (PFN) if present
///     * Bits 0-4   swap type if swapped
///     * Bits 5-54  swap offset if swapped
///     * Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
///     * Bit  56    page exclusively mapped (since 4.2)
///     * Bits 57-60 zero
///     * Bit  61    page is file-page or shared-anon (since 3.5)
///     * Bit  62    page swapped
///     * Bit  63    page present
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
/// @see http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html
/// @see https://stackoverflow.com/questions/5748492/is-there-any-api-for-determining-the-physical-address-from-virtual-address-in-li/45128487#45128487
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
#include <stdio.h>        // For printf()
#include <string.h>       // For memset() strncmp()
#include <unistd.h>       // For sysconf() close() pread() _SC_PAGESIZE

#include "colors.h"       // For ANSI_COLOR_...
#include "config.h"       // For getProgramName()
#include "iomem.h"        // For get_iomem_region_description() MAX_IOMEM_DESCRIPTION
#include "pagecount.h"    // For getPagecount() closePagecount()
#include "pageflags.h"    // For getPageflags() closePageflags()
#include "pagemap.h"      // Just cuz
#include "shannon.h"      // For computeShannonEntropy() getShannonClassification()
#include "typedefs.h"     // for pfn_t PFN_FORMAT
#include "version.h"      // For STRINGIFY_VALUE()


/// A static file descriptor to #pagemapFilePath (or `-1` if it hasn't been set)
///
/// The static file descriptor allows us to efficiently keep #pagemapFilePath open.
/// It's closed in closePagemap()
static int pagemap_fd = -1 ;


inline size_t getPageSizeInBytes() {
   return sysconf( _SC_PAGESIZE ) ;  /// @API{ sysconf, https://man.archlinux.org/man/sysconf.3 }
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
   ///
   /// @API{ __builtin_clzl, https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-_005f_005fbuiltin_005fclzl }
   return ( sizeof( size_t ) << 3 ) - __builtin_clzl( getPageSizeInBytes() - 1 ) ;
}


struct PageInfo getPageInfo( void* vAddr, const bool okToRead ) {
   // printf( "%p\n", vAddr ) ;

   struct PageInfo page = {} ;
   memset( &page, 0, sizeof( page ) ) ;  /// @API{ memset, https://man.archlinux.org/man/memset.3 }

   uintptr_t pageMask = ~( ( 1 << getPageSizeInBits() ) - 1 ) ;
   // printf( "pageMask = %lx\n", pageMask ) ;
   page.virtualAddress = (void*) ( (uintptr_t) vAddr & pageMask ) ;  /// @NOLINT( performance-no-int-to-ptr ):  We need to cast the `uintptr_t` to a `void*`

   /// Scan for Shannon entropy before reading `pagemap`, `pageflags` and `pagecount`
   if( okToRead && scanForShannon ) {
      page.shannon = computeShannonEntropy( page.virtualAddress, getPageSizeInBytes() ) ;
   }

   off_t pagemap_offset = (off_t) (((long long) vAddr) / getPageSizeInBytes() * PAGEMAP_ENTRY) ;

   /// @API{ open, https://man.archlinux.org/man/open.2 }
   if( pagemap_fd < 0 ) {
      pagemap_fd = open( pagemapFilePath, O_RDONLY ) ;
      if( pagemap_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", pagemapFilePath ) ;
      }
   }

   pagemap_t pagemap_data ;

   page.valid = true ;
   // There's a small risk here... some pread functions may return something
   // between 1 and 7 bytes, and we'd continue the read.  There's examples of
   // how to do this in our GitHub history, but I'm simplifying the code for
   // now and just requesting a single 8 byte read -- take it or leave it.
   /// @API{ pread, https://man.archlinux.org/man/pread.2 }
   ssize_t ret = pread(pagemap_fd                  // File descriptor
                      ,((uint8_t*) &pagemap_data)  // Destination buffer
                      ,PAGEMAP_ENTRY               // Bytes to read
                      ,pagemap_offset ) ;          // Read data from this offset  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
   if( ret != PAGEMAP_ENTRY ) {
      page.valid = false ;
      printf( "Unable to read[%s] for [%p]\n", pagemapFilePath, vAddr ) ;
   }

   if( page.valid ) {
      page.swapped = GET_BIT( pagemap_data, 62 ) ;

      /// @NOLINTBEGIN( readability-magic-numbers ):  Due to the nature of this module, we will allow magic numbers
      if( page.swapped ) {
         page.swap_type = pagemap_data & 0b000011111 ; // Bits 0-4
         page.swap_offset = (void*) ( ( pagemap_data & 0x007FFFFFFFFFFFC0 ) >> 5 ) ; // Bits 5-54 >> 5 bits  /// @NOLINT( performance-no-int-to-ptr ):  We need to map an int into a pointer
      } else {
         page.pfn = (pfn_t) ( pagemap_data & PFN_MASK ) ;  // Bits 0-54
      }
      /// NOLINTEND( readability-magic-numbers )

      page.soft_dirty = GET_BIT( pagemap_data, 55 ) ;
      page.exclusive = GET_BIT( pagemap_data, 56 ) ;
      ASSERT( GET_BIT( pagemap_data, 57 ) == 0 ) ;
      ASSERT( GET_BIT( pagemap_data, 58 ) == 0 ) ;
      ASSERT( GET_BIT( pagemap_data, 59 ) == 0 ) ;
      ASSERT( GET_BIT( pagemap_data, 60 ) == 0 ) ;
      page.file_mapped = GET_BIT( pagemap_data, 61 ) ;
      page.present = GET_BIT( pagemap_data, 63 ) ;

      page.page_count = getPagecount( page.pfn ) ;

      getPageflags( &page ) ;
   }

   return( page ) ;
}


/// Print the virtual address starting address for both `--pfn` and `--phys`
///
/// @param page The page to print
void printVirtualAddressStart( const struct PageInfo* page ) {
   ASSERT( page != NULL ) ;

   printf( ANSI_COLOR_GREEN "    %p - " ANSI_COLOR_RESET, page->virtualAddress ) ;
}


/// Print generic information about a page
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
   printf( ANSI_COLOR_GREEN    "%c" ANSI_COLOR_RESET, page->swapbacked  ? 'S' : ' ' ) ;

   printf( ANSI_COLOR_CYAN     "%c" ANSI_COLOR_RESET, page->nopage      ? 'N' : ' ' ) ;
   printf( ANSI_COLOR_RED      "%c" ANSI_COLOR_RESET, page->hwpoison    ? '!' : ' ' ) ;

   printf( ANSI_COLOR_BRIGHT_CYAN "  %s " ANSI_COLOR_RESET, get_iomem_region_description( page->pfn ) ) ;

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
/// @param startPage Is NULL when #includePhysicalPageNumber is set.  When
///                  #includePhysicalPageSummary, it's the start page for
///                  the region and is used to compute the size of the region.
/// @return `true` if flags should be printed.  `false` if flags should not
///         be printed.
bool printPageContext( const struct PageInfo* page, const struct PageInfo* startPage ) {
   ASSERT( page != NULL ) ;

   if( includePhysicalPageNumber ) {
      ASSERT( startPage == NULL ) ;
   }

   if( includePhysicalPageSummary ) {
      ASSERT( startPage != NULL ) ;
      /// @todo Optimize this a bit, so we don't have to call getPageSizeInBytes() every time... Maybe make it a global/inline
      printf( ANSI_COLOR_GREEN "%p " ANSI_COLOR_RESET, page->virtualAddress + getPageSizeInBytes() - 1 ) ;
      printf( ANSI_COLOR_CYAN "%'10zu " ANSI_COLOR_RESET, page->virtualAddress - startPage->virtualAddress + getPageSizeInBytes() ) ;
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
      printf( ANSI_COLOR_RED "swapped: " ANSI_COLOR_RESET ) ;
      printf( "type: %u  ", page->swap_type );
      if( includePhysicalPageNumber ) {
         printf( "offset: 0x%p  ", page->swap_offset ) ;
      }
      return false ;
   }

   if( includePhysicalPageNumber && page->present && !page->swapped ) {
      printf( "pfn: " ANSI_COLOR_GREEN "0x%07" PFN_FORMAT " " ANSI_COLOR_RESET, page->pfn ) ;
      printf( "#:%3" PRIu64 " ", page->page_count ) ;
   }

   return true ;
} // printPageContext


void printFullPhysicalPage( const struct PageInfo* page ) {
   ASSERT( page != NULL );

   printVirtualAddressStart( page ) ;
   if( printPageContext( page, NULL ) ) {
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

   // If swapped & swap_type is the same, then ignore the other flags
   if(     left->swapped
        && right->swapped
        && left->swap_type == right->swap_type ) {
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
   /// @API{ strncmp, https://man.archlinux.org/man/strncmp.3 }
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

   size_t startPage = 0 ;  // The start of the current region

   printVirtualAddressStart( &page[0] ) ;

   for( size_t i = 0 ; i < numPages-1 ; i++ ) {
      if( comparePages( &page[i], &page[i+1] ) == DIFFERENT_PAGE_SAME_FLAGS ) {
         continue ;
      }

      if( printPageContext( &page[i], &page[startPage] ) ) {
         printPageFlags( &page[i] ) ;
      }

      printf( "\n" ) ;

      printVirtualAddressStart( &page[i+1] ) ;
      startPage = i+1 ;
   }

   // Print the last page
   if( printPageContext( &page[numPages-1], &page[startPage] ) ) {
      printPageFlags( &page[numPages-1] ) ;
   }
   printf( "\n" ) ;
} // printPageSummary


void closePagemap() {
   /// Pagemap holds some files open (like #pagemap_fd) in static variables.
   /// Close the files properly.
   if( pagemap_fd != -1 ) {
      int closeStatus = close( pagemap_fd ) ;  /// @API{ close, https://man.archlinux.org/man/close.2 }
      pagemap_fd = -1 ;
      if( closeStatus != 0 ) {
         FATAL_ERROR( "Unable to close [%s]", pagemapFilePath );
      }
   }

   closePagecount() ;
   closePageflags() ;
}


void printKey( FILE* outStream ) {
   PRINT( outStream, ANSI_COLOR_RESET ) ;
   PRINT( outStream, "memscan --phys will output physical pages like this:\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN "0x55a5be863000 - 0x55a5be864fff " ANSI_COLOR_CYAN "  8,192 " ANSI_COLOR_RED "page not present\n" ANSI_COLOR_RESET ) ;
   PRINT( outStream, ANSI_COLOR_GREEN "0x55a5be865000 - 0x55a5be8abfff " ANSI_COLOR_CYAN "290,816 " ANSI_COLOR_RESET "Flags: " ANSI_COLOR_YELLOW "*" ANSI_COLOR_CYAN "X" ANSI_COLOR_RESET "           \\      IO: " ANSI_COLOR_GREEN "U" ANSI_COLOR_RESET " LRU:" ANSI_COLOR_GREEN "L    M" ANSI_COLOR_YELLOW "A " ANSI_COLOR_GREEN "S    " ANSI_COLOR_BRIGHT_CYAN "System RAM\n" ANSI_COLOR_RESET ) ;
   PRINT( outStream, ANSI_COLOR_GREEN "0x55a5be8ac000 - 0x55a5be8adfff " ANSI_COLOR_CYAN "  8,192 " ANSI_COLOR_RESET "Flags: " ANSI_COLOR_YELLOW "*" ANSI_COLOR_CYAN "X" ANSI_COLOR_RESET "           \\      IO: " ANSI_COLOR_GREEN "U" ANSI_COLOR_RESET " LRU:" ANSI_COLOR_GREEN " A   M" ANSI_COLOR_YELLOW "A " ANSI_COLOR_GREEN "S    " ANSI_COLOR_BRIGHT_CYAN "System RAM\n" ANSI_COLOR_RESET ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "memscan --pfn will output physical pages like this:\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN "0x7ffca078e000 - " ANSI_COLOR_RESET "pfn: " ANSI_COLOR_GREEN "0x0279672 " ANSI_COLOR_RESET "#:  1 Flags: " ANSI_COLOR_YELLOW "*" ANSI_COLOR_CYAN "X" ANSI_COLOR_RESET "           \\      IO: " ANSI_COLOR_GREEN "U" ANSI_COLOR_RESET " LRU:" ANSI_COLOR_GREEN "L    M" ANSI_COLOR_YELLOW "A " ANSI_COLOR_GREEN "S    " ANSI_COLOR_BRIGHT_CYAN "System RAM " ANSI_COLOR_BRIGHT_YELLOW "H: 0.564 Very low entropy\n" ANSI_COLOR_RESET ) ;
   PRINT( outStream, ANSI_COLOR_GREEN "0x7ffca078f000 - " ANSI_COLOR_RESET "pfn: " ANSI_COLOR_GREEN "0x0214113 " ANSI_COLOR_RESET "#:  1 Flags: " ANSI_COLOR_YELLOW "*" ANSI_COLOR_CYAN "X" ANSI_COLOR_RESET "           \\      IO: " ANSI_COLOR_GREEN "U" ANSI_COLOR_RESET " LRU:" ANSI_COLOR_GREEN "L    M" ANSI_COLOR_YELLOW "A " ANSI_COLOR_GREEN "S    " ANSI_COLOR_BRIGHT_CYAN "System RAM " ANSI_COLOR_BRIGHT_YELLOW "H: 3.082 English text in UNICODE\n" ANSI_COLOR_RESET ) ;
   PRINT( outStream, ANSI_COLOR_GREEN "0x7ffca0790000 - " ANSI_COLOR_RESET "pfn: " ANSI_COLOR_GREEN "0x0118352 " ANSI_COLOR_RESET "#:  1 Flags: " ANSI_COLOR_YELLOW "*" ANSI_COLOR_CYAN "X" ANSI_COLOR_RESET "           \\      IO: " ANSI_COLOR_GREEN "U" ANSI_COLOR_RESET " LRU:" ANSI_COLOR_GREEN "L    M" ANSI_COLOR_YELLOW "A " ANSI_COLOR_GREEN "S    " ANSI_COLOR_BRIGHT_CYAN "System RAM " ANSI_COLOR_BRIGHT_YELLOW "H: 1.190 Unknown\n" ANSI_COLOR_RESET ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "Each line starts with the virtual address region of the physical page.\n" ) ;
   PRINT( outStream, "It will report high-level messages like \"page not present\" or \"page swapped\".\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "pfn: Page Frame Number.  Essentially, the physical page.\n" ) ;
   PRINT( outStream, ANSI_COLOR_RESET  "#:   the number of times each page is mapped\n" ) ;
   PRINT( outStream,                   "\n" ) ;
   PRINT( outStream,                   "Flags:\n" ) ;
   PRINT( outStream, ANSI_COLOR_YELLOW "*:   page is soft-dirty (it's been written to recently)\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "X:   page exclusively mapped\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "F:   page is file mapped and not anonymously mapped\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "KSM: kernel samepage merging: Identical memory pages are dynamically shared\n" ) ;
   PRINT( outStream,                   "     between processes\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "0:   zero page for pfn_zero or huge_zero page\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "L:   locked for exclusive access, e.g. by undergoing read/write IO\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "S:   page is managed by the SLAB/SLOB/SLUB/SLQB kernel memory allocator\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "W:   writeback: page is being synced to disk\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "B:   a free memory block managed by the buddy system allocator\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "I:   idle: The page has not been accessed since it was marked idle\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "P:   the page is in use as a page table\n" ) ;
   PRINT( outStream, ANSI_COLOR_RESET  "\n" ) ;
   PRINT( outStream,                   "\\    the end of regular flags and the start of huge page flags\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "H:   huge TLB page\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "T:   transparent huge page:  Contiguous pages that form a huge allocation\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "<:   the head of a contiguous block of pages\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   ">:   the tail of a contiguous block of pages\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "B:   page is tagged for balloon compaction\n" ) ;
   PRINT( outStream, ANSI_COLOR_RESET  "\n" ) ;
   PRINT( outStream,                   "IO:  I/O flags\n" ) ;
   PRINT( outStream, ANSI_COLOR_RED    "!:   IO error occurred\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "U:   page has up-to-date data for file backed page\n" ) ;
   PRINT( outStream, ANSI_COLOR_YELLOW "D:   page page has been written to and contains new data\n" ) ;
   PRINT( outStream, ANSI_COLOR_RESET  "\n" ) ;
   PRINT( outStream,                   "LRU: least recently used flags\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "L:   page is in one of the LRU lists\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "A:   active: page is in the active LRU list\n" ) ;
   PRINT( outStream, ANSI_COLOR_MAGENTA "U:   unevictable: It is pinned and not a candidate for reclamation\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "R:   referenced: page has been referenced since last LRU list enqueue/requeue\n" ) ;
   PRINT( outStream, ANSI_COLOR_YELLOW "R:   reclaim: page will be reclaimed soon after its pageout IO completed\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "M:   memory mapped page\n" ) ;
   PRINT( outStream, ANSI_COLOR_YELLOW "A:   anonymous: memory mapped page that is not part of a file\n" ) ;
   PRINT( outStream, ANSI_COLOR_YELLOW "C:   swapCache: page is mapped to swap space, i.e. has an associated swap entry\n" ) ;
   PRINT( outStream, ANSI_COLOR_GREEN  "S:   swapBacked: page is backed by swap/RAM\n" ) ;
   PRINT( outStream, ANSI_COLOR_RESET  "\n" ) ;
   PRINT( outStream,                   "Rarely seen flags\n" ) ;
   PRINT( outStream, ANSI_COLOR_CYAN   "N:   no page frame exists at the requested address\n" ) ;
   PRINT( outStream, ANSI_COLOR_RED    "!:   hardware detected memory corruption: Don't touch this page\n" ) ;
   PRINT( outStream, ANSI_COLOR_RESET  "" ) ;
} // printKey
