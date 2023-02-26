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
#include <string.h>  // For strlen() & strncpy()

#include "config.h"
#include "version.h"

/// Buffer to hold the program name
char programName[MAX_PROGRAM_NAME] = {};


bool setProgramName( char* newProgramName ) {
   if( newProgramName == NULL ) {
      /// @todo Print an appropriate error message
      return false ;
   }

   if( strlen( newProgramName ) == 0 ) {
      /// @todo Print an appropriate error message
      return false ;
   }

   strncpy( programName, newProgramName, MAX_PROGRAM_NAME );

   return true;
}


char* getProgramName() {
   return programName;
}
