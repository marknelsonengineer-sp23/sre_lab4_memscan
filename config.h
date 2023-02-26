///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Process command line parameters and hold configuration for memscan
///
/// @file   config.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdbool.h>  // For bool


#define MAX_PROGRAM_NAME 32  ///< The maximum size of the program name


/// Set the program name
///
/// @todo Copy into a buffer and do some parameter chacking
///
/// @param newProgramName The new program name
/// @return `true` if successful.  `false` if unsuccessful.
extern bool setProgramName( char* newProgramName ) ;


/// Get the program name
///
/// @return The program's name
extern char* getProgramName() ;
