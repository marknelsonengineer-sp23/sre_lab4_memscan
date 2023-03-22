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

#include <stddef.h>  // For NULL


#ifdef __x86_64__
/// Get the end of the stack frame for this thread -- which is where any new
/// local variables would be allocated.
///
/// This must be declared as a macro because `inline` is not always inlined
/// and this **must** run in the context of the calling function.  It can't
/// run in the context of its own function, which has a different stack
/// frame.
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

#ifndef GET_BASE_OF_STACK
   #pragma GCC error "Need inline assembly instructions for this architecture"
#endif

/// Getting the base of the stack should be the start of a dynamic
/// local variable region.
///
/// This is mostly for testing and should not be used because it
/// will return the base of the stack in its own stack frame rather
/// than the calling function's stack frame (which is what we want).
/// Use #GET_BASE_OF_STACK instead.
///
/// @return A pointer to the next local variable
extern void* getBaseOfStack() ;
