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


#include <fcntl.h>     // For open() O_RDONLY
#include <inttypes.h>  // For PRIu64
#include <stdint.h>    // For uint64_t
#include <stdio.h>     // For fopen() fprintf()
#include <stdlib.h>    // For exit() EXIT_FAILURE
//#include <sys/types.h>
#include <unistd.h>    // For sysconf()

#include <sys/syscall.h>      /* Definition of SYS_* constants */
//#include <unistd.h>

#include "config.h"  // For getProgramName()
#include "pagemap.h" // Just cuz


/// The `pagemap` file we intend to read from `/proc`
#define PAGEMAP_FILE "/proc/self/pagemap"

/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt),
/// each pagemap entry is 8 bytes long
#define PAGEMAP_ENTRY 8

/// Get the `bitPosition` bit from `value`
///
/// Return `1` if the bit is set and `0` if it's not in the same datatype as `value`
#define GET_BIT( value, bitPosition ) ((value) & ( (uint64_t) 1 << (bitPosition))) >> (bitPosition)


/// Get the Page Frame Number
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
///
/// Per `pagemap.txt`:  `Bits 0-54  page frame number (PFN) if present`
#define GET_PFN( pageMapData ) ((pageMapData) & 0x7FFFFFFFFFFFFF)  // Bits 0-54

/// A static file descriptor to PAGEMAP_FILE (or -1 if it hasn't been set yet)
///
/// The static file descriptor allows us to efficiently keep `pagemap` open.
/// It's closed in closePagemap()
static int pagemap_fd = -1 ;


size_t getPageSizeInBytes() {
   return sysconf(_SC_PAGESIZE) ;
}


/// Get the PFN and associated flags from pAddr
///
/// @param pAddr The address to analyze (usually the starting address of a
///              page frame, but it doesn't have to be.
void doPagemap( void* pAddr ) {  /// @todo consider renaming to `vaddr`
   struct PhysicalPage page = {} ;

   if( pagemap_fd < 0 ) {
      pagemap_fd = open( PAGEMAP_FILE, O_RDONLY ) ;
      if( pagemap_fd == -1 ) {
         FATAL_ERROR( "Unable to open [%s]", PAGEMAP_FILE );
      }
   }

   FILE* pagefile = fopen( PAGEMAP_FILE, "rb" );

   if( !pagefile ){
      FATAL_ERROR( "Unable to open [%s]", PAGEMAP_FILE );
   }

   // Code courtesy of http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html
   // Shift by pagefile_offset number of bytes
   // and multiply by the size of an address (the size of an entry in pagemap file)
   long pagefile_offset = (long) ((size_t) pAddr / getPageSizeInBytes() * PAGEMAP_ENTRY) ;
   // printf( "Vaddr: 0x%p, Page_size: %zd, Entry_size: %d\n", pAddr, getPageSizeInBytes(), PAGEMAP_ENTRY ) ;
   // printf( "Reading %s at 0x%" PRIu64 "\n", PAGEMAP_FILE, pagefile_offset );
   int status = fseek( pagefile, pagefile_offset, SEEK_SET );
   if(status) {
      printf( "Failed to %s\n", "fseek()" );
   }

   unsigned char c_buf[PAGEMAP_ENTRY] ;

   for( int i = 0; i < PAGEMAP_ENTRY ; i++) {
      int j = getc( pagefile );
      if( j == EOF ) {
         FATAL_ERROR( "failed to getc() - End of file" );
      }
//      if(is_bigendian())
//         c_buf[i] = c;
//      else
      c_buf[ PAGEMAP_ENTRY - i - 1 ] = j ;
//      printf("[%d]0x%x ", i, c);
   }

   uint64_t read_val = 0;

   for( int i = 0 ; i < PAGEMAP_ENTRY ; i++ ) {
      //printf("%d ",c_buf[i]);
      read_val = (read_val << 8) + c_buf[i] ;   /// @NOLINT(readability-magic-numbers): 8 bits per byte is a valid magic number
   }


   /*
   size_t pagefile_offset2 = ((size_t) pAddr / getPageSizeInBytes() * PAGEMAP_ENTRY) ;
   size_t result = lseek64( pagemap_fd, pagefile_offset2, SEEK_SET ) ;
   if( result != pagefile_offset2 ) {
      printf( "Failed to lseek64 to position %lx in [%s]\n", pagefile_offset2, PAGEMAP_FILE );
   }

   result = read( pagemap_fd, &page.pfn, sizeof( page.pfn ) );
   if( result != sizeof( page.pfn ) ) {
      printf( "failed to read the PFN [%zd]\n", result );
   }
*/


   size_t nread = 0 ;
   uint64_t data;
   uintptr_t vpn = (size_t) pAddr / sysconf(_SC_PAGE_SIZE) ;

   while( nread < sizeof(data) ) {
      ssize_t ret = pread(pagemap_fd, ((uint8_t*) &data) + nread, sizeof(data) - nread,
                          vpn * sizeof( data  ) + nread );  /// @NOLINT( bugprone-narrowing-conversions ):  `pread`'s `offset` parameter is a `off_t` (`long`), so we have to accept the narrowing conversion
      nread += ret;
      if (ret <= 0) {
         FATAL_ERROR( "Unable to read [%s]", PAGEMAP_FILE );
      }
   }

   page.pfn = data & (((uint64_t)1 << 55) - 1);  /// @NOLINT( readability-magic-numbers )
   page.swapped = (data >> 62) & 1 ;  /// @NOLINT( readability-magic-numbers )
   page.present = (data >> 63) & 1 ;  /// @NOLINT( readability-magic-numbers )





   printf( "Result: 0x%" PRIx64 "   0x%" PRIx64 "\n", read_val, page.pfn ) ;

   if( GET_BIT( read_val, 63 ) ) {
      printf( "PFN: 0x%llx\n", (unsigned long long) GET_PFN( read_val ));
   } else {
      printf( "Page not present\n" );
   }

   if( GET_BIT( read_val, 62 ) ) {
      printf( "Page swapped\n" );
   } else {
      printf( "Page not swapped\n" );
   }

   int iRetVal = fclose( pagefile ) ;
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", PAGEMAP_FILE );
   }
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
