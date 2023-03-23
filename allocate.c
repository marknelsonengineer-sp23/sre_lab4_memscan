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

#include <stdio.h>        // @todo REMOVE REMOVE REMOVE

#include <pthread.h>      // For pthread_create()
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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/// Recursively allocate `--local` bytes of memory for `--numLocal` iterations
/// and put them in the #localAllocations array.
///
/// @param remainingLocalAllocations The number of remaining `--numLocal`
///                                  allocations to make
void allocateLocalMemory_recurse( const size_t remainingLocalAllocations ) {
   if( remainingLocalAllocations == 0 ) {
      printf( "...waiting...\n" ) ;

      pthread_mutex_lock( &mutex );

      printf( "Released!!!\n" ) ;
      return ;
   }

   // Store the base of the stack in localAllocations array
   GET_BASE_OF_STACK( localAllocations[remainingLocalAllocations - 1] ) ;

   /// Local memory allocations needed to be rounded up to `sizeof( uint64_t )`
   ///   - `localSize` 0 through 8 should round to 8
   ///   - `localSize` 9 through 16 should round to 16
   ///   - `localSize`17 through 24 should round to 24

   int roundingCorrection = 7 - (localSize-1) % sizeof( uint64_t ) ;

   size_t roundedLocalSize = localSize + roundingCorrection ;

   ALLOCATE_LOCAL_STORAGE( roundedLocalSize ) ;

   printf( "Allocate %zu bytes of local memory at %p\n", roundedLocalSize, localAllocations[remainingLocalAllocations - 1] ) ;

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

      pthread_mutex_lock( &mutex ) ;

      int rVal = pthread_create( &localAllocationThread, NULL, &localAllocationThreadStart, NULL ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to create local allocator thread" ) ;
      }
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


/// This constant has a Shannon entropy of `3.000`.
/// When `--malloc`, `--fill`, and `--shannon` are used together, this will fill
/// memory regions and is detectable when we analyze for Shannon entropy.
//
/// The constant must contain 8 unique bytes.
#define SHANNON_CONSTANT_FOR_ALLOCATIONS 0x1122334455667788


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

//    for( size_t i = 0 ; i < numLocals ; i++ ) {
      for( size_t i = 0 ; i < 1 ; i++ ) {
//       for( size_t j = 0 ; j < localSize ; j += sizeof( uint64_t ) ) {
         for( size_t j = 0 ; j < 1 ; j += sizeof( uint64_t ) ) {
            printf( "x\n" ) ;
//            *(uint64_t*)(localAllocations[i] + j) = SHANNON_CONSTANT_FOR_ALLOCATIONS ;
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
