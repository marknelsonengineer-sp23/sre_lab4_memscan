///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Allocate memory for `--local`, `--malloc` and `--shared` options
///
/// @file   allocate.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>     // For uint64_t
#include <stdlib.h>     // For malloc() calloc() free()

#include "allocate.h"   // Just cuz
#include "config.h"     // For FATAL_ERROR


/// Maintain an array[`--numMalloc`] of pointers to heap allocations`--malloc`
static void** heapAllocations = NULL ;

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
   }
} // allocatePreScanMemory


/// This constant has a Shannon entropy of 3.000.
/// When `--malloc`, `--fill`, and `--shannon` are used together, this will fill
/// memory regions and is detectable when we analyze for Shannon entropy.
//
/// The constant needs to consist of 8 unique bytes.
#define SHANNON_CONSTANT_FOR_HEAP 0x1122334455667788


void fillPreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocations != NULL ) ;
      ASSERT( mallocSize > 0 ) ;
      ASSERT( numMallocs >= 1 ) ;

      for( size_t i = 0 ; i < numMallocs ; i++ ) {
         for( size_t j = 0 ; j < mallocSize ; j += sizeof( uint64_t ) ) {
            *(uint64_t*)(heapAllocations[i] + j) = SHANNON_CONSTANT_FOR_HEAP ;
         }
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
   }
} // releasePreScanMemory
