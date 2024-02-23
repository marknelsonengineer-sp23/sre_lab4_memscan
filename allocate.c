///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Allocate and optionally populate memory for `--local`, `--numLocal`,
/// `--malloc`, `--numMalloc`, `--mem_map`, `--mapAddr` and `--fill`
///
/// @file   allocate.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>      // For pthread_mutex_t PTHREAD_MUTEX_INITIALIZER
                          // pthread_mutex_lock() pthread_mutex_unlock()
                          // pthread_t pthread_create()  pthread_join()
#include <semaphore.h>    // For sem_t sem_init() sem_post() sem_wait()
#include <stdint.h>       // For uint64_t
#include <stdlib.h>       // For malloc() calloc() free()

#include "assembly.h"     // For GET_BASE_OF_STACK() ALLOCATE_LOCAL_STORAGE()

/// Enable Linux-specific features in `mmap()`
///
/// @NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp): Need this to use `MAP_FIXED_NOREPLACE` and `MAP_ANONYMOUS`
#define __USE_MISC 1
   #include <sys/mman.h>  // For mmap()
#undef __USE_MISC

#include "allocate.h"     // Just cuz
#include "config.h"       // For FATAL_ERROR


/// This constant has a Shannon entropy of `3.000`.
/// When `--malloc`, `--fill`, and `--shannon` are used together, this will fill
/// memory regions and is detectable when we analyze for Shannon entropy.
//
/// The constant must contain 8 unique bytes.
#define SHANNON_CONSTANT_FOR_ALLOCATIONS 0x1122334455667788


/// Maintain an array[`--numMalloc`] of pointers to heap allocations for `--malloc`.
static void** heapAllocations = NULL ;

/// Maintain an array[`--numLocal`] of pointers to local allocations for `--local`.
static void** localAllocations = NULL ;

/// The thread, starting in localAllocationThreadStart(), that holds the local
/// allocations for `--local`.
static pthread_t localAllocationThread = 0 ;

/// Pointer to the `--map_mem` allocation
static void* mappedAllocation = NULL ;

/// Mutex to hold the local memory allocator thread until memscan is shutdown
///   - The mutex is locked in allocatePreScanMemory()
///   - It is held (synchronized) by the local allocation thread in
///     allocateLocalMemory_recurse()
///   - The mutex is unlocked in releasePreScanMemory(), which allows the
///     thread to continue, unwind its recursive calls and end.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;


/// Semaphore to track when the local variables have been created ;
sem_t localAllocationsReady ;


/// Recursively allocate `--local` bytes of memory for `--numLocal` iterations
/// and put them in the #localAllocations array.
///
/// @param remainingLocalAllocations The number of remaining `--numLocal`
///                                  allocations to make
///
/// @NOLINTNEXTLINE(misc-no-recursion):  Recursion is allowed here
void allocateLocalMemory_recurse( const size_t remainingLocalAllocations ) {
   if( remainingLocalAllocations == 0 ) {
      // printf( "...waiting...\n" ) ;

      sem_post( &localAllocationsReady ) ;

      pthread_mutex_lock( &mutex ) ;

      // printf( "Released!!!\n" ) ;
      return ;
   }

   // Remember the base of the stack...
   void* stackBase = NULL ;
   GET_BASE_OF_STACK( stackBase ) ;

   /// Local memory allocations needed to be rounded up to `sizeof( uint64_t )`
   /// for both stack alignment and to insert #SHANNON_CONSTANT_FOR_ALLOCATIONS
   ///   - `localSize` 0 through 8 should round to 8
   ///   - `localSize` 9 through 16 should round to 16
   ///   - `localSize`17 through 24 should round to 24
   size_t roundingCorrection = (sizeof( uint64_t ) - 1) - (localSize-1) % sizeof( uint64_t ) ;

   size_t roundedLocalSize = localSize + roundingCorrection ;

   ALLOCATE_LOCAL_STORAGE( roundedLocalSize ) ;

   /// Save the start address of the local memory allocation in #localAllocations.
   localAllocations[remainingLocalAllocations - 1] = stackBase - roundedLocalSize ;

   // printf( "Allocate %zu bytes of local memory: %p - %p\n", roundedLocalSize, localAllocations[remainingLocalAllocations - 1], localAllocations[remainingLocalAllocations - 1] + localSize - 1 ) ;

   allocateLocalMemory_recurse( remainingLocalAllocations - 1 ) ;
}


/// The thread that creates and holds local memory allocations via
/// allocateLocalMemory_recurse().  This thread is kept in
/// #localAllocationThread.
///
/// @param arg Required by `pthread_create()` but not used by memscan
/// @return Always `NULL`
static void* localAllocationThreadStart( void *arg ) {
   (void) arg ;  // Suppress warning about unused variable

   allocateLocalMemory_recurse( numLocals ) ;

   return NULL ;
}


void allocatePreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocations == NULL ) ;
      ASSERT( mallocSize > 0 ) ;
      ASSERT( numMallocs >= 1 ) ;

      heapAllocations = calloc( numMallocs, sizeof( void* ) ) ;
      if( heapAllocations == NULL ) {
         FATAL_ERROR( "unable to allocate malloc array during --malloc" ) ;
      }

      for( size_t i = 0 ; i < numMallocs ; i++ ) {
         heapAllocations[i] = malloc( mallocSize ) ;
         if( heapAllocations[i] == NULL ) {
            FATAL_ERROR( "unable to allocate memory during --malloc" ) ;
         }
      }
   } // allocateHeapMemory

   if( allocateLocalMemory ) {
      ASSERT( localAllocations == NULL ) ;
      ASSERT( localSize > 0 ) ;
      ASSERT( numLocals >= 1 ) ;

      localAllocations = calloc( numLocals, sizeof( void* ) ) ;
      if( localAllocations == NULL ) {
         FATAL_ERROR( "unable to allocate local array during --local" ) ;
      }

      // Initialize localAllocations to 0
      for( size_t i = 0 ; i < numLocals ; i++ ) {
         localAllocations[i] = NULL ;
      }

      // Lock the mutex and only unlock it when memscan is ready to quit
      pthread_mutex_lock( &mutex ) ;

      // Use a semaphore to continue main() only after the local memory allocation
      // is done
      sem_init( &localAllocationsReady, 0, 0 ) ;

      int rVal = pthread_create( &localAllocationThread, NULL, &localAllocationThreadStart, NULL ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to create local allocator thread" ) ;
      }

      // Wait until the local memory allocation is done
      sem_wait( &localAllocationsReady ) ;

   } // allocateLocalMemory

   if( allocateMappedMemory ) {
      ASSERT( mappedAllocation == NULL ) ;
      ASSERT( mappedSize > 0 ) ;

      if( mappedStart == NULL ) {
         mappedAllocation = mmap( NULL           // addr
                                 ,mappedSize     // length
                                 ,PROT_READ      // protection
                                 |PROT_WRITE
                                 ,MAP_PRIVATE    // flags
//                               ,MAP_SHARED     // flags
                                 |MAP_ANONYMOUS
                                 ,-1             // file descriptor
                                 ,0              // offset
         ) ;
      } else {
         mappedAllocation = mmap( mappedStart    // addr
                                 ,mappedSize     // length
                                 ,PROT_READ      // protection
                                 |PROT_WRITE
                                 ,MAP_PRIVATE    // flags
//                               ,MAP_SHARED     // flags
                                 |MAP_FIXED_NOREPLACE
                                 |MAP_ANONYMOUS
                                 ,-1             // file descriptor
                                 ,0              // offset
         ) ;
      }

      // printf( "mappedStart=%p  mappedAllocation=%p\n", mappedStart, mappedAllocation ) ;

      if( mappedAllocation == MAP_FAILED ) {
         FATAL_ERROR( "unable to allocate memory during --map_mem" ) ;
      }
   } // allocateMappedMemory
} // allocatePreScanMemory


void fillPreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocations != NULL ) ;
      ASSERT( mallocSize > 0 ) ;
      ASSERT( numMallocs >= 1 ) ;

      for( size_t i = 0 ; i < numMallocs ; i++ ) {
         for( size_t j = 0 ; j < mallocSize ; j += sizeof( uint64_t ) ) {
            *(uint64_t*)(heapAllocations[i] + j) = SHANNON_CONSTANT_FOR_ALLOCATIONS ;
         }
      }
   } // allocateHeapMemory

   if( allocateLocalMemory ) {
      ASSERT( localAllocations != NULL ) ;
      ASSERT( localSize > 0 ) ;
      ASSERT( numLocals >= 1 ) ;

      for( size_t i = 0 ; i < numLocals ; i++ ) {
         // printf( "i=%zu  localAllocations[%zu]=%p - %p  localSize=%zu\n", i, i, localAllocations[i], localAllocations[i] + localSize - 1, localSize ) ;
         if( localAllocations[i] == NULL ) {
            // printf( "continue\n" ) ;  // This really should never happen
            continue ;
         }
         for( size_t j = 0 ; j < localSize ; j += sizeof( uint64_t ) ) {
            // printf( "i=%zu  j=%zu  localAllocations[i] + j=%p\n", i, j, (localAllocations[i] + j) ) ;
            *(uint64_t*)(localAllocations[i] + j) = SHANNON_CONSTANT_FOR_ALLOCATIONS ;
         }
      }
   } // allocateLocalMemory

   if( allocateMappedMemory ) {
      ASSERT( mappedAllocation != NULL ) ;
      ASSERT( mappedSize > 0 ) ;

      for( size_t i = 0 ; i < mappedSize ; i += sizeof( uint64_t ) ) {
         *(uint64_t*)(mappedAllocation + i) = SHANNON_CONSTANT_FOR_ALLOCATIONS ;
      }
   }
} // fillPreScanMemory


void releasePreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocations != NULL ) ;
      ASSERT( numMallocs >= 1 ) ;

      for( size_t i = 0 ; i < numMallocs ; i++ ) {
         free( heapAllocations[i] ) ;
         heapAllocations[i] = NULL ;
      }

      free( heapAllocations ) ;
      heapAllocations = NULL ;
   } // allocateHeapMemory

   if( allocateLocalMemory ) {
      ASSERT( localAllocations != NULL ) ;
      ASSERT( numLocals >= 1 ) ;
      ASSERT( localAllocationThread != 0 ) ;

      // Release local allocator thread
      pthread_mutex_unlock (&mutex) ;
      int result = pthread_join( localAllocationThread, NULL ) ;
      if( result != 0 ) {
         WARNING( "pthread_join returned %d", result ) ;
      }

      // Set localAllocations to NULL
      for( size_t i = 0 ; i < numLocals ; i++ ) {
         localAllocations[i] = NULL ;
      }

      free( localAllocations ) ;
      localAllocations = NULL ;
   } // allocateLocalMemory

   if( allocateMappedMemory ) {
      ASSERT( mappedAllocation != NULL ) ;

      int i = munmap( mappedAllocation, mappedSize ) ;
      if( i != 0 ) {
         WARNING( "munmap failed" ) ;
      }

      mappedAllocation = NULL ;

   } // allocateMappedMemory
} // releasePreScanMemory
