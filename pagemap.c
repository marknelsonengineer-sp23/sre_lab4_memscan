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

#include <inttypes.h>  // For PRIu64
#include <stdint.h>    // For uint64_t
#include <stdio.h>     // For fopen() fprintf()
#include <stdlib.h>    // For exit() EXIT_FAILURE
#include <unistd.h>    // For sysconf()

#include "config.h"  // For getProgramName()
#include "pagemap.h" // Just cuz


/// The `pagemap` file we intend to read from `/proc`
#define PAGEMAP_FILE "/proc/self/pagemap"

/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt),
/// each pagemap entry is 8 bytes long
#define PAGEMAP_ENTRY 8

#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y

#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF


size_t getPageSizeInBytes() {
   return sysconf(_SC_PAGESIZE) ;
}

/// Lorem Ipsum
void doP( void* pAddr ) {
   FILE* pagefile = fopen( PAGEMAP_FILE, "rb" );

   if( !pagefile ){
      FATAL_ERROR( "Unable to open [%s]", PAGEMAP_FILE );
   }

   //Shifting by virt-addr-offset number of bytes
   //and multiplying by the size of an address (the size of an entry in pagemap file)
   uint64_t pagefile_offset = (size_t) pAddr / getPageSizeInBytes() * PAGEMAP_ENTRY ;
   printf( "Vaddr: 0x%p, Page_size: %zd, Entry_size: %d\n", pAddr, getPageSizeInBytes(), PAGEMAP_ENTRY ) ;
   printf( "Reading %s at 0x%" PRIu64 "\n", PAGEMAP_FILE, pagefile_offset );
   int status = fseek( pagefile, pagefile_offset, SEEK_SET );
   if(status) {
      FATAL_ERROR( "Failed to %s", "fseek()" );
   }

   unsigned char c_buf[PAGEMAP_ENTRY] ;

   for( int i = 0; i < PAGEMAP_ENTRY ; i++) {
      int c = getc( pagefile );
      if( c == EOF ) {
         FATAL_ERROR( "Failed to %s - End of file", "getc()" );
      }
//      if(is_bigendian())
//         c_buf[i] = c;
//      else
         c_buf[ PAGEMAP_ENTRY - i - 1 ] = c ;
//      printf("[%d]0x%x ", i, c);
   }

   uint64_t read_val = 0;

   for( int i = 0 ; i < PAGEMAP_ENTRY ; i++ ) {
      //printf("%d ",c_buf[i]);
      read_val = (read_val << 8) + c_buf[i] ;
   }

   printf( "Result: 0x%" PRIu64 "\n", read_val ) ;

   if( GET_BIT( read_val, 63 ) )
      printf("PFN: 0x%llx\n",(unsigned long long) GET_PFN(read_val) );
   else
      printf("Page not present\n");

   if( GET_BIT( read_val, 62 ) )
      printf("Page swapped\n");
   else
      printf( "Page not swapped\n" );


   int iRetVal = fclose( pagefile ) ;
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", PAGEMAP_FILE );
   }
}
