///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// A memory scanner:  memscan's `main()`
///
/// @file   memscan.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>   // For printf() fprintf()
#include <stdlib.h>  // For EXIT_SUCCESS and EXIT_FAILURE

#include "allocate.h" // For allocatePreScanMemory(), et. al.
#include "config.h"   // For processOptions()
#include "files.h"    // For openPreScanFiles() readPreScanFiles() closePreScanFiles()
#include "iomem.h"    // For read_iomem()
#include "maps.h"     // For readEntries() scanEntries()
#include "memscan.h"  // Just cuz
#include "pagemap.h"  // For closePagemap()


/// A memory scanner:  memscan's `main()`
///
/// @param argc The number of arguments passed to `memscan`
/// @param argv An array of arguments passed to `memscan`
/// @return The program's return code
int main( int argc, char* argv[] ) {
   // Initialize the program
   processOptions( argc, argv ) ;
   read_iomem() ;  // Bring in the physical memory allocation from `iomem`

   if( iomemSummary ) {      // Process --iomem
      summarize_iomem() ;
      exit( EXIT_SUCCESS ) ;
   }

   // Do pre-scan operations
   openPreScanFiles() ;
   allocatePreScanMemory() ;

   /// Anything that changes #MEMORY_MAP_FILE should be done before calling readMaps()
   /// @todo Process --block
   /// @todo Process --stream
   /// @todo Process --map_file
   /// @todo Process --local, --malloc, --mem_map and --fill
   /// @todo Process --threads

   readMaps() ;

   /// Anything that changes the physical pagemap information such as scanning
   /// or waiting, should be done before calling readPagemapInfo()

   if( readFileContents ) {
      readPreScanFiles() ;
   }

   if( fillAllocatedMemory ) {
      fillPreScanMemory() ;
   }

   scanMaps() ;  /// @todo Process --scan_byte, --histogram and --shannon

   readPagemapInfo() ;  // Process --phys

   printMaps() ;

   releasePreScanMemory() ;
   closePreScanFiles() ;
   closePagemap() ;

   return EXIT_SUCCESS ;
} // main()
