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
#include <stdlib.h>     // For malloc() free()
#include <x86intrin.h>  // For _rdtsc()

#include "allocate.h"   // Just cuz
#include "config.h"     // For FATAL_ERROR


/// Pointer to the heap's memory allocation:  `--malloc`
static void* heapAllocation = NULL ;

void allocatePreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocation == NULL ) ;
      ASSERT( mallocSize > 0 ) ;

      heapAllocation = malloc( mallocSize ) ;

      if( heapAllocation == NULL ) {
         FATAL_ERROR( "unable to allocate memory during --malloc" ) ;
      }
   }
} // allocatePreScanMemory


/// This constant has a Shannon entropy of 3.000.
/// When `--malloc`, `--fill`, and `--shannon` are used together, this will fill
/// memory regions and is detectable when we analyze for Shannon entropy.
#define SHANNON_CONSTANT_FOR_HEAP 0x1122334455667788


void fillPreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( mallocSize > 0 ) ;
      ASSERT( heapAllocation != NULL ) ;

      for( size_t i = 0 ; i < mallocSize ; i += sizeof( uint64_t ) ) {
         if( scanForShannon ) {
            *(uint64_t*)(heapAllocation + i) = SHANNON_CONSTANT_FOR_HEAP ;
         } else {
            *(uint64_t*)(heapAllocation + i) = _rdtsc() ;
         }
      }
   }
} // fillPreScanMemory


void releasePreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocation != NULL ) ;

      free( heapAllocation ) ;
      heapAllocation = NULL ;
   }
} // releasePreScanMemory
