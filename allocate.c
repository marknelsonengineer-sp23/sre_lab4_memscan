///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Allocate memory for `--local`, `--malloc` and `--mem_map` options
///
/// @file   allocate.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>       // For uint64_t
#include <stdlib.h>       // For malloc() calloc() free()

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

/// Pointer to the `--map_mem` allocation
static void* mappedAllocation = NULL ;

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

   if( allocateMappedMemory ) {
      ASSERT( mappedAllocation == NULL ) ;
      ASSERT( mappedSize > 0 ) ;

      mappedAllocation = mmap( (void*) 0x550000000000          // addr
//      mappedAllocation = mmap( NULL         // addr
                              ,mappedSize     // length
                              ,PROT_READ      // protection
                              |PROT_WRITE
                              ,MAP_PRIVATE    // flags
//                            ,MAP_SHARED     // flags
                              |MAP_FIXED_NOREPLACE
                              |MAP_ANONYMOUS
                              ,-1             // file descriptor
                              ,0              // offset
                              ) ;
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

   if( allocateMappedMemory ) {
      ASSERT( mappedAllocation != NULL ) ;
      free( mappedAllocation ) ;
      mappedAllocation = NULL ;
   } // allocateMappedMemory
} // releasePreScanMemory
