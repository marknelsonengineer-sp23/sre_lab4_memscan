///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get flags for each physical page from `/proc/kpageflags`
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     This file contains a 64-bit set of flags for each page, indexed by PFN
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
///
/// @file   pageflags.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

/// Enables declaration of `pread()`
///
/// @see https://man.archlinux.org/man/pread.2
/// @NOLINTNEXTLINE(bugprone-reserved-identifier, cert-dcl37-c, cert-dcl51-cpp): This is a legitimate use of a reserved identifier
#define _XOPEN_SOURCE 700

#include <fcntl.h>      // For open() O_RDONLY
#include <unistd.h>     // For pread() close()

#include "config.h"     // For FATAL_ERROR()
#include "pageflags.h"  // Just cuz
#include "typedefs.h"   // For pfn_t PFN_FORMAT


/// The `kpageflags` file we intend to read from `/proc`
static const char PAGEFLAG_FILE[] = "/proc/kpageflags" ;


/// A static file descriptor to #PAGEFLAG_FILE (or `-1` if it hasn't been set yet)
///
/// The static file descriptor allows us to efficiently keep #PAGEFLAG_FILE open.
/// It's closed in closePageflags()
static int pageflag_fd = -1 ;


void getPageflags( struct PageInfo* page ) {
   off_t pageflag_offset = (off_t) page->pfn * (off_t) sizeof( pfn_t ) ;

   if( pageflag_fd < 0 ) {
      /// @API{ open, https://man.archlinux.org/man/open.2 }
      pageflag_fd = open( PAGEFLAG_FILE, O_RDONLY ) ;
      if( pageflag_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", PAGEFLAG_FILE ) ;
      }
   }

   pageflags_t pageflag_data ;

   // There's a small risk here... some pread functions may return something
   // between 1 and 7 bytes, and we'd continue the read.  There's examples of
   // how to do this in our GitHub history, but I'm simplifying the code for
   // now and just requesting a single 8 byte read -- take it or leave it.
   /// @API{ pread, https://man.archlinux.org/man/pread.2 }
   ssize_t ret = pread( pageflag_fd                  // File descriptor
                       ,((uint8_t*) &pageflag_data)  // Destination buffer
                       ,PAGEFLAG_ENTRY               // Bytes to read
                       ,pageflag_offset ) ;          // Read data from this offset  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
   if( ret != PAGEFLAG_ENTRY ) {
      printf( "Unable to read[%s] for PFN [%" PFN_FORMAT "]\n", PAGEFLAG_FILE, page->pfn ) ;
      pageflag_data = 0 ;
   }

   page->locked      = GET_BIT( pageflag_data,  0 ) ;
   page->error       = GET_BIT( pageflag_data,  1 ) ;
   page->referenced  = GET_BIT( pageflag_data,  2 ) ;
   page->uptodate    = GET_BIT( pageflag_data,  3 ) ;
   page->dirty       = GET_BIT( pageflag_data,  4 ) ;
   page->lru         = GET_BIT( pageflag_data,  5 ) ;
   page->active      = GET_BIT( pageflag_data,  6 ) ;
   page->slab        = GET_BIT( pageflag_data,  7 ) ;
   page->writeback   = GET_BIT( pageflag_data,  8 ) ;
   page->reclaim     = GET_BIT( pageflag_data,  9 ) ;
   page->buddy       = GET_BIT( pageflag_data, 10 ) ;
   page->mmap        = GET_BIT( pageflag_data, 11 ) ;
   page->anon        = GET_BIT( pageflag_data, 12 ) ;
   page->swapcache   = GET_BIT( pageflag_data, 13 ) ;
   page->swapbacked  = GET_BIT( pageflag_data, 14 ) ;
   page->comp_head   = GET_BIT( pageflag_data, 15 ) ;
   page->comp_tail   = GET_BIT( pageflag_data, 16 ) ;
   page->huge        = GET_BIT( pageflag_data, 17 ) ;
   page->unevictable = GET_BIT( pageflag_data, 18 ) ;
   page->hwpoison    = GET_BIT( pageflag_data, 19 ) ;
   page->nopage      = GET_BIT( pageflag_data, 20 ) ;
   page->ksm         = GET_BIT( pageflag_data, 21 ) ;
   page->thp         = GET_BIT( pageflag_data, 22 ) ;
   page->balloon     = GET_BIT( pageflag_data, 23 ) ;
   page->zero_page   = GET_BIT( pageflag_data, 24 ) ;
   page->idle        = GET_BIT( pageflag_data, 25 ) ;
   page->pgtable     = GET_BIT( pageflag_data, 26 ) ;
} // getPageflags


void closePageflags() {
   /// Pageflags holds some files open (like #pageflag_fd) in static variables.
   /// Close the files properly.
   if( pageflag_fd != -1 ) {
      int closeStatus = close( pageflag_fd ) ;  /// @API{ close, https://man.archlinux.org/man/close.2 }
      pageflag_fd = -1 ;
      if( closeStatus != 0 ) {
         FATAL_ERROR( "Unable to close [%s]", PAGEFLAG_FILE ) ;
      }
   }
} // closePageflags
