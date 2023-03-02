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
#include <stdio.h>    // For FILE stderr fprintf()


/// Process command line options
///
/// @param argc The number of command line options (including the program name)
/// @param argv A `NULL` terminated array of `char[]` containing the command
///             line options
extern void processOptions( int argc, char* argv[] );


/// Print memscan's usage
///
/// @param outStream The output stream (usually `stderr` or `stdout`) to send
///                  the usage statement
void printUsage( FILE* outStream ) ;


#define MAX_PROGRAM_NAME 32  ///< The maximum size of the program name

/// Set the program name
///
/// @todo Copy into a buffer and do some parameter checking
///
/// @param newProgramName The new program name
/// @return `true` if successful.  `false` if unsuccessful.
extern bool setProgramName( char* newProgramName ) ;


/// Get the program name
///
/// @return The program's name
extern char* getProgramName() ;


/// Print an error message to `stderr` (along with the program name) and then
/// exit with a failure status.
///
/// @NOLINTBEGIN(cert-err33-c): No need to check the return value of `fprintf`
///
/// @param msg The message to print out.  When printed, it will begin
///            with `progName: ` and end with `.  Exiting.`.
#define FATAL_ERROR( msg, ... ) { \
   fprintf(                       \
      stderr                      \
     ,"%s: " msg ".  Exiting.\n"  \
     ,getProgramName()            \
     ,__VA_ARGS__ ) ;             \
   exit( EXIT_FAILURE ); }        \
   /* NOLINTEND(cert-err33-c) */  \
   (void)0
