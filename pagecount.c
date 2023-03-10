///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get the number of times each physical page is mapped from `kpagecount`
///
/// @file   pagecount.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

/// Enables declaration of `pread()`
///
/// @see https://man7.org/linux/man-pages/man3/read.3p.html
/// @NOLINTNEXTLINE(bugprone-reserved-identifier, cert-dcl37-c, cert-dcl51-cpp): This is a legitimate use of a reserved identifier
#define _XOPEN_SOURCE 700

#include <fcntl.h>      // For open() O_RDONLY
#include <unistd.h>     // For close()

#include "config.h"     // For FATAL_ERROR()
#include "pagecount.h"  // Just cuz


/// The `kpagecount` file we intend to read from `/proc`
#define PAGECOUNT_FILE "/proc/kpagecount"

/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt),
/// each pagecount entry is `8` bytes long
#define PAGECOUNT_ENTRY 8


/// A static file descriptor to #PAGECOUNT_FILE (or `-1` if it hasn't been set yet)
///
/// The static file descriptor allows us to efficiently keep #PAGECOUNT_FILE open.
/// It's closed in closePagecount()
static int pagecount_fd = -1 ;


uint64_t getPagecount( void* pfn ) {
   off_t pagecount_offset = (long) ((size_t) pfn * sizeof( uint64_t ) ) ;

   if( pagecount_fd < 0 ) {
      pagecount_fd = open( PAGECOUNT_FILE, O_RDONLY ) ;
      if( pagecount_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", PAGECOUNT_FILE );
      }
   }

   uint64_t pagecount_data;

   // There's some risk here... some pread functions may return something between
   // 1 and 7 bytes, and we'd continue the read.  There's examples of how to
   // do this in our GitHub history, but I'm simplifying the code for now and
   // just requesting a single 8 byte read -- take it or leave it.
   ssize_t ret = pread( pagecount_fd                  // File descriptor
                       ,((uint8_t*) &pagecount_data)  // Destination buffer
                       ,PAGECOUNT_ENTRY               // Bytes to read
                       ,pagecount_offset );           // Read data from this offset  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
   if( ret != PAGECOUNT_ENTRY ) {
      printf( "Unable to read[%s] for PFN [%p]\n", PAGECOUNT_FILE, pfn ) ;
      pagecount_data = 0 ;
   }

   return pagecount_data ;
} // getPagecount


void closePagecount() {
   /// Pagecount holds some files open (like #pagecount_fd) in static variables.
   /// Close the files properly.
   if( pagecount_fd != -1 ) {
      int closeStatus = close( pagecount_fd ) ;
      pagecount_fd = -1 ;
      if( closeStatus != 0 ) {
         FATAL_ERROR( "Unable to close [%s]", PAGECOUNT_FILE );
      }
   }
} // closePagecount
