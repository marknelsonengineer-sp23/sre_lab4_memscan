///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Centralize inline Assembly Language functionality
///
/// @file   assembly.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stddef.h>  // For NULL size_t


#ifdef __x86_64__
/// Get the end of the stack frame for this thread -- which is where new
/// local variables would be allocated.
///
/// This must be declared as a macro because `inline` is not always inlined
/// and this **must** run in the context of the calling function's stack frame.
///
/// @param baseOfStack This **must** be a pointer (ideally, a `void*`) and
///                    the macro will put the base of the stack into this
///                    pointer.
#define GET_BASE_OF_STACK( baseOfStack )                   \
   baseOfStack = NULL ;                                    \
                                                           \
__asm__ inline ( "MOVQ %%RSP, %0 ;"                        \
            : "=g"(baseOfStack)  /* output operands */     \
            :                    /* input operands  */     \
            : )                  /* clobbered registers */
#endif

#ifdef __i386__
#define GET_BASE_OF_STACK( baseOfStack )                   \
   baseOfStack = NULL ;                                    \
                                                           \
__asm__ inline ( "MOVL %%ESP, %0 ;"                        \
            : "=g"(baseOfStack)  /* output operands */     \
            :                    /* input operands  */     \
            : )                  /* clobbered registers */
#endif

#ifdef __arm__
#define GET_BASE_OF_STACK( baseOfStack )                   \
   baseOfStack = NULL ;                                    \
                                                           \
__asm__ inline ( "STR SP, [%0]"                            \
            : "=g"(baseOfStack)  /* output operands */     \
            :                    /* input operands  */     \
            : )                  /* clobbered registers */
#endif

#ifndef GET_BASE_OF_STACK
   #pragma GCC error "Need inline assembly instructions for GET_BASE_OF_STACK"
#endif

/// Get the end of the stack frame for this thread -- which is where new
/// local variables would be allocated.
///
/// This is mostly for testing and should not be used because it
/// will return the base of the stack in its own stack frame rather
/// than the calling function's stack frame (which is what we want).
/// Use #GET_BASE_OF_STACK instead.
///
/// @return A pointer to the next local variable
extern void* getBaseOfStack() ;




#ifdef __x86_64__
/// Subtract `newSize` from the stack, thereby allocating that amount
/// for local variables.
///
/// This must be declared as a macro because `inline` is not always inlined
/// and this **must** run in the context of the calling function's stack frame.
///
/// @param newSize The number of bytes to allocate for local data
#define ALLOCATE_LOCAL_STORAGE( newSize )                     \
                                                              \
   __asm__ inline ( "SUBQ %0, %%RSP ;"                        \
               :                    /* output operands     */ \
               : "g" (newSize)      /* input operands      */ \
               : )                  /* clobbered registers */
#endif

#ifdef __i386__
#define ALLOCATE_LOCAL_STORAGE( newSize )                     \
                                                              \
   __asm__ inline ( "SUBL %0, %%ESP ;"                        \
               :                    /* output operands     */ \
               : "g" (newSize)      /* input operands      */ \
               : )                  /* clobbered registers */
#endif

#ifdef __arm__
#define ALLOCATE_LOCAL_STORAGE( newSize )                     \
                                                              \
   __asm__ inline ( "LDR r0, %0\n\t"                          \
                    "SUB SP, SP, r0"                          \
               :                    /* output operands     */ \
               : "g" (newSize)      /* input operands      */ \
               : "r0" )             /* clobbered registers */
#endif

#ifndef ALLOCATE_LOCAL_STORAGE
   #pragma GCC error "Need inline assembly instructions for ALLOCATE_LOCAL_STORAGE"
#endif

/// Subtract `newSize` from the stack, thereby allocating that amount
/// for local variables.
///
/// This is mostly for testing and should not be used because it
/// will update the stack in its own stack frame rather
/// than the calling function's stack frame (which is what we want).
/// Use #ALLOCATE_LOCAL_STORAGE instead.
///
/// @param newSize The number of bytes to allocate for local data
extern void allocateLocalStorage( const size_t newSize ) ;
