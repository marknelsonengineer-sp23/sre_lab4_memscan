///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Allocate memory for `--local`, `--malloc` and `--shared` options
///
/// @file   allocate.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>    // For malloc() free()
#include "allocate.h"  // Just cuz
#include "config.h"    // For FATAL_ERROR


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
}


void fillPreScanMemory() {

}


void releasePreScanMemory() {
   if( allocateHeapMemory ) {
      ASSERT( heapAllocation != NULL ) ;

      free( heapAllocation ) ;
      heapAllocation = NULL ;
   }
}
