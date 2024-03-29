///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// A memory scanner:  memscan's `main()`
///
/// @file   memscan.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>    // For exit() EXIT_SUCCESS and EXIT_FAILURE
#include <sys/wait.h>  // For waitpid()
#include <unistd.h>    // For sleep()

#include "allocate.h"  // For allocatePreScanMemory() fillPreScanMemory() releasePreScanMemory()
#include "config.h"    // For processOptions() checkCapabilities()
#include "files.h"     // For openPreScanFiles() readPreScanFiles() closePreScanFiles()
#include "iomem.h"     // For read_iomem() summarize_iomem()
#include "maps.h"      // For getMaps() scanMaps() readPagemapInfo() printMaps()
#include "memscan.h"   // Just cuz
#include "pagemap.h"   // For closePagemap()
#include "threads.h"   // For createThreads() closeThreads()


/// A memory scanner:  memscan's `main()`
///
/// @param argc The number of arguments passed to `memscan` (including the
///             program name).  Always `>= 1`.
/// @param argv A `NULL` terminated array of `char[]` arguments containing the
///             command line options
/// @return     The program's return code:  `EXIT_SUCCESS` or `EXIT_FAILURE`
int main( int argc, char* argv[] ) {
   // Initialize the program
   reset_config() ;
   processOptions( argc, argv ) ; // Process --help, --key, --version

   if( ! validateConfig( true ) ) { // Ensure the configuration is healthy
      printUsage( stderr ) ;
      reset_config() ;
      exit( EXIT_FAILURE ) ;
   }

   checkCapabilities() ;          // Ensure memscan is running with `CAP_SYS_ADMIN`

   read_iomem() ;                 // Bring in the physical memory allocation from `iomem`

   if( iomemSummary ) {           // Process --iomem
      summarize_iomem() ;
   } else if( forkProcess ) {     // Process --fork
      pid_t forkPid = fork() ;

      if( forkPid < 0 ) {
         FATAL_ERROR( "fork failed" ) ;
      }

      /// @todo we should be able to implement the pre-scan options with --fork

      // Both the parent & child run concurrently to get maps...
      struct MapEntry* myMaps = getMaps() ;

      scanMaps( myMaps ) ;        // Process --scan_byte, --shannon

      readPagemapInfo( myMaps ) ; // Process --phys and --pfn

      // The parent waits for the child process to finish...
      if( forkPid == 0 ) {
         printf( "Child memscan\n" ) ;
         printMaps( myMaps ) ;

         printf( "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n" ) ;
      } else {
         int returnStatus ;
         waitpid( forkPid, &returnStatus, 0 ) ;  // Parent process waits here for child to terminate.

         printf( "Parent memscan\n " ) ;
         printMaps( myMaps ) ;
      }

      releaseMaps( myMaps ) ;
      myMaps = NULL ;

      closePagemap() ;

   } else if( scanPid != -1 ) {   // Process --pid
      struct MapEntry* myMaps = getMaps() ;

      scanMaps( myMaps ) ;        // Process --scan_byte, --shannon

      readPagemapInfo( myMaps ) ; // Process --phys and --pfn

      printMaps( myMaps ) ;

      releaseMaps( myMaps ) ;
      myMaps = NULL ;

      closePagemap() ;

   } else if( scanSelf ) {
      // Do pre-scan operations
      openPreScanFiles() ;        // Process --block, --stream, --map_file and --read
      allocatePreScanMemory() ;   // Process --local, --numLocal, --malloc, --numMalloc,
                                  // --mem_map, --mapAddr and --fill

      /// Anything that changes #mapsFilePath should be done before calling getMaps()

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

      struct MapEntry* myMaps = getMaps() ;

      scanMaps( myMaps ) ;        // Process --scan_byte, --shannon

      readPagemapInfo( myMaps ) ; // Process --phys and --pfn

      printMaps( myMaps ) ;

      releaseMaps( myMaps ) ;
      myMaps = NULL ;

      closeThreads() ;
      releasePreScanMemory() ;
      closePreScanFiles() ;
      closePagemap() ;

   } else {
      ASSERT( false ) ;           // We should never get here
   }

   release_iomem() ;
   reset_config() ;

   return EXIT_SUCCESS ;
} // main()
