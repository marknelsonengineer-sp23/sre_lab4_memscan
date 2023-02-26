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

/// The name of the program (copied from `argv[0]`)
extern char* programName ;

extern bool setProgramName( char* newProgramName ) ;

extern char* getProgramName() ;
