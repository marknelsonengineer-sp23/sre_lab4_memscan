///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get more information about a virtual address
///
/// @see http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
///
/// @file   pagemap.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

/// Enables declaration of `lseek64()`
///
/// @see https://man7.org/linux/man-pages/man3/lseek64.3.html
/// @NOLINTNEXTLINE(bugprone-reserved-identifier, cert-dcl37-c, cert-dcl51-cpp): This is a legitimate use of a reserved identifier
#define _LARGEFILE64_SOURCE

/// Enables declaration of `pread()`
///
/// @see https://man7.org/linux/man-pages/man3/read.3p.html
/// @NOLINTNEXTLINE(bugprone-reserved-identifier, cert-dcl37-c, cert-dcl51-cpp): This is a legitimate use of a reserved identifier
#define _XOPEN_SOURCE 700


#include <fcntl.h>        // For open() O_RDONLY
#include <inttypes.h>     // For PRIu64
#include <stdint.h>       // For uint64_t
#include <stdio.h>        // For fopen() fprintf()
#include <stdlib.h>       // For exit() EXIT_FAILURE
#include <sys/syscall.h>  // Definition of SYS_* constants
#include <unistd.h>       // For sysconf()

#include "config.h"  // For getProgramName()
#include "iomem.h"   // For get_iomem_region_description()
#include "pagemap.h" // Just cuz


/// The `pagemap` file we intend to read from `/proc`
#define PAGEMAP_FILE "/proc/self/pagemap"

/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt),
/// each pagemap entry is 8 bytes long
#define PAGEMAP_ENTRY 8


/// Get the `bitPosition` bit from `value`
///
/// Return `1` if the bit is set and `0` if it's not.
/// The return datatype is the same datatype as `value`
#define GET_BIT( value, bitPosition ) (((value) >> (bitPosition)) & 1)


/// A static file descriptor to PAGEMAP_FILE (or -1 if it hasn't been set yet)
///
/// The static file descriptor allows us to efficiently keep `pagemap` open.
/// It's closed in closePagemap()
static int pagemap_fd = -1 ;


inline size_t getPageSizeInBytes() {
   return sysconf( _SC_PAGESIZE ) ;
}


inline unsigned char getPageSizeInBits() {
   /// The pagesize must be a power of 2 (this is what allows us to access the
   /// interior of a page).  This function finds out which bit (which power of 2)
   /// it is.  FFS is meant to be used with signed numbers (not what we want), so
   /// we will use CLZ which counts leading zeros.
   ///
   /// @see https://en.wikipedia.org/wiki/Find_first_set
   /// @see https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
   ///
   /// - Start with the size of an address in bits
   /// - Subtract the number of leading 0s
   /// - Subtract one more to get the index of the first bit

   return ( sizeof( size_t ) << 3 ) - __builtin_clzl( getPageSizeInBytes() - 1 ) ;
}


/// Get the PFN and associated flags from pAddr
///
/// @param pAddr The address to analyze (usually the starting address of a
///              page frame, but it doesn't have to be.
void doPagemap( void* pAddr ) {  /// @todo consider renaming to `vaddr`
// printf( "%p\n", pAddr ) ;

   struct PhysicalPage page = {} ;

   /// @todo Rename to pagemap_offset
   /// @todo Reconsider the datatype
   long pagefile_offset = (long) ((size_t) pAddr / getPageSizeInBytes() * PAGEMAP_ENTRY ) ;

   if( pagemap_fd < 0 ) {
      pagemap_fd = open( PAGEMAP_FILE, O_RDONLY ) ;
      if( pagemap_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", PAGEMAP_FILE );
      }
   }

   size_t nread = 0 ;
   uint64_t data;

   page.valid = true ;
   while( nread < sizeof(data) ) {
      ssize_t ret = pread(pagemap_fd, ((uint8_t*) &data) + nread, PAGEMAP_ENTRY - nread,
                          pagefile_offset + nread );  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
      nread += ret;
      if (ret <= 0) {
         page.valid = false ;
         printf( "Unable to read[%s] for [%p]\n", PAGEMAP_FILE, pAddr ) ;
      }
   }

   if( page.valid ) {
      page.pfn = (void*) ( data & 0x7FFFFFFFFFFFFF );  /// @NOLINT( readability-magic-numbers, performance-no-int-to-ptr )
      page.swapped = GET_BIT( data, 62 );
      page.present = GET_BIT( data, 63 );
   }

   printf( "pAddr: %p  ", pAddr ) ;
   printf( "pfn: 0x%p  ", page.pfn ) ;
   printf( "Swapped: %d  ", page.swapped ) ;
   printf( "Present: %d  ", page.present ) ;
   printf( "Region: %s  ", get_iomem_region_description( (void*) page.pfn ) ) ;
   printf( "\n" ) ;
}


void closePagemap() {
   if( pagemap_fd != -1 ) {
      int closeStatus = close( pagemap_fd ) ;
      pagemap_fd = -1 ;
      if( closeStatus != 0 ) {
         FATAL_ERROR( "Unable to close [%s]", PAGEMAP_FILE );
      }
   }
}
