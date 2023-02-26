///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Process command line parameters and hold configuration for memscan
///
/// @file   config.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stddef.h>  // For NULL

#include "config.h"
#include "version.h"

char* programName = NULL ;

/// Set the program name
///
/// @todo Copy into a buffer and do some parameter chacking
///
/// @param newProgramName The new program name
/// @return `true` if successful.  `false` if unsuccessful.
bool setProgramName( char* newProgramName ) {
   programName = newProgramName;

   return true;
}

/// Get the program name
///
/// @return The program's name
char* getProgramName() {
   return programName;
}
