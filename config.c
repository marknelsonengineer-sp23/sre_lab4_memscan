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
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For EXIT_FAILURE
#include <string.h>  // For strlen() & strncpy()

#include "config.h"
#include "version.h"

/// Buffer to hold the program name
char programName[MAX_PROGRAM_NAME] = {};


/// Print a line to outStream.  Ensure the print command was successful.
///
/// @todo Print an appropriate message for the user
///
/// @param outStream The output stream (usually `stderr` or `stdout`) to print to
#define PRINT_USAGE( outStream, ... )              \
   if( fprintf( outStream, __VA_ARGS__ ) <= 0 ) {  \
      exit( EXIT_FAILURE );                        \
   }


void printUsage( FILE* outStream ) {
   PRINT_USAGE( outStream, "Usage: memscan [OPTION]...\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "The options below may be used to select memscan's operation\n" ) ;
   PRINT_USAGE( outStream, "  -h, --help    display this help and exit\n" ) ;
   PRINT_USAGE( outStream, "  -v, --version output version information and exit\n" ) ;
}


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
