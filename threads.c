///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Implement the `--threads` option
///
/// @file   threads.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>  // For pthread_create() pthread_join()
#include <stdio.h>    // REMOVE BEFORE FLIGHT
#include <stdlib.h>   // For calloc()

#include "allocate.h" // For fillPreScanMemory()
#include "config.h"   // For numThreads
#include "files.h"    // For readPreScanFiles()
#include "threads.h"  // Just cuz


/// While this is true, the threads will run
bool runningThreads = true ;

/// Hold information about the thread
struct ThreadInfo {
   size_t    index  ;  ///< The index of the thread:  0, 1, 2, ...
   pthread_t thread ;  ///< A structure that holds thread information
} ;

/// A dynamic array of #ThreadInfo
struct ThreadInfo* threadArray = NULL ;


/// The thread that runs for `--threads`
///
/// @param arg An argument passed into the worker thread.  In our case, it's a
///            pointer to its #ThreadInfo structure
/// @return A pointer... in our case, `NULL`
void* workerThread(void *arg) {
   struct ThreadInfo thread = *(struct ThreadInfo*)arg ;  // Dereference a ThreadInfo from arg
   (void)thread ;
   // printf( "start thread.index=%zu  &thread=%p\n", thread.index, &thread ) ;

   /// Loop while #runningThreads and readPreScanFiles() then fillPreScanMemory()
   while( runningThreads ) {
      if( readFileContents ) {
         readPreScanFiles() ;  // Process --read
      }

      if( fillAllocatedMemory ) {
         fillPreScanMemory() ; // Process --fill
      }
   }

   // printf( "end threadIndex=%d\n", thread.index ) ;

   return NULL ;
}


void createThreads() {
   ASSERT( threadArray == NULL ) ;

   if( numThreads <= 0 ) {
      return ;
   }

   threadArray = calloc( numThreads, sizeof( struct ThreadInfo ) ) ;
   if( threadArray == NULL ) {
      FATAL_ERROR( "unable to allocate thread array" ) ;
   }

   ASSERT( runningThreads == true ) ;
   for( size_t i = 0 ; i < numThreads ; i++ ) {
      threadArray[i].index = i ;
      int result = pthread_create(
              &threadArray[i].thread  // Pointer to the pthread_t struct
             ,NULL                    // Thread attributes
             ,workerThread            // Pointer to the start function
             ,&threadArray[i]         // Pass everything to the thread
             ) ;
      if( result != 0 ) {
         FATAL_ERROR( "pthread_create[%zu] failed", i ) ;
      }
   }
}


void closeThreads() {
   if( numThreads <= 0 ) {
      return ;
   }

   ASSERT( runningThreads == true ) ;

   /// Setting #runningThreads to `false` will cause the worker threads to fall
   /// out of their `while( runningThreads )` loop
   runningThreads = false ;

   // Wait for all of the threads to quit...
   for( size_t i = 0 ; i < numThreads ; i++ ) {
      int result = pthread_join( threadArray[i].thread, NULL ) ;
      if( result != 0 ) {
         WARNING( "pthread_join failed" ) ;
      }
   }

   free( threadArray ) ;
   threadArray = NULL ;
}
