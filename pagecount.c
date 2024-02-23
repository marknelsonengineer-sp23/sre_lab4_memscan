///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get the number of times each physical page is mapped from `/proc/kpagecount`
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     This file contains a 64-bit count of the number of times each page is
///     mapped, indexed by PFN.
///
///
/// @file   pagecount.c
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
#include "pagecount.h"  // Just cuz
#include "typedefs.h"   // For pfn_t const_pfn_t PFN_FORMAT


/// The `kpagecount` file we intend to read from `/proc`
static const char PAGECOUNT_FILE[] = "/proc/kpagecount" ;

/// A static file descriptor to #PAGECOUNT_FILE (or `-1` if it hasn't been set yet)
///
/// The static file descriptor allows us to efficiently keep #PAGECOUNT_FILE open.
/// It's closed in closePagecount()
static int pagecount_fd = -1 ;


pagecount_t getPagecount( const_pfn_t pfn ) {
   off_t pagecount_offset = (off_t) pfn * (off_t) sizeof( pfn_t ) ;

   if( pagecount_fd < 0 ) {
      /// @API{ open, https://man.archlinux.org/man/open.2 }
      pagecount_fd = open( PAGECOUNT_FILE, O_RDONLY ) ;
      if( pagecount_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", PAGECOUNT_FILE ) ;
      }
   }

   pagecount_t pagecount_data ;

   // There's a small risk here... some pread functions may return something
   // between 1 and 7 bytes, and we'd continue the read.  There's examples of
   // how to do this in our GitHub history, but I'm simplifying the code for
   // now and just requesting a single 8 byte read -- take it or leave it.
   /// @API{ pread, https://man.archlinux.org/man/pread.2 }
   ssize_t ret = pread( pagecount_fd                  // File descriptor
                       ,((uint8_t*) &pagecount_data)  // Destination buffer
                       ,PAGECOUNT_ENTRY               // Bytes to read
                       ,pagecount_offset ) ;          // Read data from this offset  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
   if( ret != PAGECOUNT_ENTRY ) {
      printf( "Unable to read[%s] for PFN [%" PFN_FORMAT "]\n", PAGECOUNT_FILE, pfn ) ;
      pagecount_data = 0 ;
   }

   return pagecount_data ;
} // getPagecount


void closePagecount() {
   /// Pagecount holds some files open (like #pagecount_fd) in static variables.
   /// Close the files properly.
   if( pagecount_fd != -1 ) {
      int closeStatus = close( pagecount_fd ) ;  /// @API{ close, https://man.archlinux.org/man/close.2 }
      pagecount_fd = -1 ;
      if( closeStatus != 0 ) {
         FATAL_ERROR( "Unable to close [%s]", PAGECOUNT_FILE ) ;
      }
   }
} // closePagecount
