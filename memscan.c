///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// A memory scanner:  memscan's `main()`
///
/// @file   memscan.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>   // For exit() EXIT_SUCCESS and EXIT_FAILURE
#include <unistd.h>   // For sleep()

#include "allocate.h" // For allocatePreScanMemory() fillPreScanMemory() releasePreScanMemory()
#include "config.h"   // For processOptions() checkCapabilities()
#include "files.h"    // For openPreScanFiles() readPreScanFiles() closePreScanFiles()
#include "iomem.h"    // For read_iomem() summarize_iomem()
#include "maps.h"     // For readMaps() scanMaps() readPagemapInfo() printMaps()
#include "memscan.h"  // Just cuz
#include "pagemap.h"  // For closePagemap()
#include "threads.h"  // For createThreads() closeThreads()


/// A memory scanner:  memscan's `main()`
///
/// @param argc The number of arguments passed to `memscan` (including the
///             program name).  Always `>= 1`.
/// @param argv A `NULL` terminated array of `char[]` arguments containing the
///             command line options
/// @return     The program's return code:  `EXIT_SUCCESS` or `EXIT_FAILURE`
int main( int argc, char* argv[] ) {
   // Initialize the program
   processOptions( argc, argv ) ;  // Process --help, --key, --version

   checkCapabilities() ;       // Ensure memscan is running with `CAP_SYS_ADMIN`

   read_iomem() ;              // Bring in the physical memory allocation from `iomem`

   if( iomemSummary ) {        // Process --iomem
      summarize_iomem() ;
      exit( EXIT_SUCCESS ) ;
   }

   // Do pre-scan operations
   openPreScanFiles() ;        // Process --block, --stream, --map_file and --read
   allocatePreScanMemory() ;   // Process --local, --numLocal, --malloc, --numMalloc,
                               // --mem_map, --mapAddr and --fill

   /// Anything that changes #MEMORY_MAP_FILE should be done before calling readMaps()

   /// Anything that changes the physical pagemap information such as scanning
   /// or waiting, should be done before calling readPagemapInfo()

   if( numThreads == 0 ) {
      if( readFileContents ) {
         readPreScanFiles() ;  // Process --read
      }

      if( fillAllocatedMemory ) {
         fillPreScanMemory() ; // Process --fill
      }
   } else {
      createThreads() ;        // Process --threads (which will do --read and --fill)
   }

   if( sleepSeconds > 0 ) {
      sleep( sleepSeconds ) ;  // Process --sleep
   }

   readMaps() ;

   scanMaps() ;                // Process --scan_byte, --shannon

   readPagemapInfo() ;         // Process --phys and --pfx

   printMaps() ;

   closeThreads() ;
   releasePreScanMemory() ;
   closePreScanFiles() ;
   closePagemap() ;

   return EXIT_SUCCESS ;
} // main()
