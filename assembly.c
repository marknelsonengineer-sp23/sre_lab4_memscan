///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Centralize inline Assembly Language functionality
///
/// @file   assembly.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "assembly.h"

void* getBaseOfStack() {
   void* baseOfStack = NULL ;

   #ifdef __x86_64__
   __asm__ inline ( "MOVQ %%RSP, %0 ;"
            : "=g"(baseOfStack)  // output operands
            :                    // input operands
            : ) ;                // clobbered registers
   #endif

   #ifdef __i386__
   __asm__ inline ( "MOVL %%ESP, %0 ;"
            : "=g"(baseOfStack)  // output operands
            :                    // input operands
            : ) ;                // clobbered registers
   #endif

   return baseOfStack ;
}


void allocateLocalStorage( const size_t newSize ) {
   #ifdef __x86_64__
   __asm__ inline ( "SUBQ %0, %%RSP ;"
            :                    /* output operands     */
            : "g" (newSize)      /* input operands      */
            : ) ;                /* clobbered registers */
   #endif
}
