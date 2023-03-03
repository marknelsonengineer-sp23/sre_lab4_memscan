///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Process command line parameters and hold configuration for memscan
///
/// @file   config.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <getopt.h>  // For getopt_long() struct option
#include <limits.h>  // For PATH_MAX
#include <stddef.h>  // For NULL
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For exit() EXIT_SUCCESS EXIT_FAILURE
#include <string.h>  // For strlen() & strncpy()

#include "config.h"
#include "version.h"


/// Print a line to outStream.  Ensure the print command was successful.
///
/// @todo Print an appropriate message for the user
///
/// @param outStream The output stream (usually `stderr` or `stdout`) to print to
#define PRINT_USAGE( outStream, ... )              \
   if( fprintf( outStream, __VA_ARGS__ ) <= 0 ) {  \
      exit( EXIT_FAILURE );                        \
   }                                               \
   (void)0

/// Buffer to hold the program name
char programName[MAX_PROGRAM_NAME] = {};


/// Define the command line options for memscan
static struct option long_options[] = {
   { "block",   required_argument, 0, 'b' },
   { "stream",  required_argument, 0, '0' },
   { "mmap",    required_argument, 0, '1' },
   { "fork",    no_argument,       0, 'f' },
   { "malloc",  required_argument, 0, 'm' },
   { "phys",    no_argument,       0, 'p' },
   { "shared",  required_argument, 0, 's' },
   { "threads", required_argument, 0, 't' },
   { "help",    no_argument,       0, 'h' },
   { "version", no_argument,       0, 'v' },
   { 0, 0, 0, 0 }
};


void processOptions( int argc, char* argv[] ) {
   if( argc < 1 ) {
      printf( "Unexpected argument count [%d].  Exiting.\n", argc );
      exit( EXIT_FAILURE );
   }

   setProgramName( argv[0] ) ;

   while( true ) {
      int option_index = 0;
      int optionChar;

      optionChar = getopt_long(argc, argv, "b:fm:ps:t:hv", long_options, &option_index);

      if( optionChar == -1 ) {
         break ;  // Done processing getopt_long
      }

      switch ( optionChar ) {
         case 'v':
            printf( "memscan version %s\n", FULL_VERSION ) ;
            printf( "Copyright (C) 2023 Mark Nelson\n") ;
            printf( "Written by Mark Nelson\n" );
            exit( EXIT_SUCCESS ) ;
            break ;

         case 'h':
            printUsage( stdout ) ;
            exit( EXIT_SUCCESS ) ;
            break ;

         default:
            printUsage( stderr ) ;
            exit( EXIT_FAILURE ) ;
            break ;
      }
   }

   if (optind < argc) {
      PRINT_USAGE( stderr, "%s: Unknown additional arguments.  Exiting.\n", programName ) ;
      printUsage( stderr ) ;
      exit( EXIT_FAILURE ) ;
   }
}

bool openFileWithBlockIO       = 0 ;
bool openFileWithStreamIO      = 0 ;
bool openFileWithMapIO         = 0 ;
bool forkProcess               = 0 ;
bool mallocMemory              = 0 ;
bool includePhysicalMemoryInfo = 0 ;
bool createSharedMemory        = 0 ;
bool createThreads             = 0 ;

char blockPath[ FILENAME_MAX ]   = {} ;
char streamPath [ FILENAME_MAX ] = {} ;
char mmapPath [ FILENAME_MAX ]   = {} ;

size_t mallocSize = 0 ;
size_t sharedSize = 0 ;
size_t numThreads = 0 ;

void printUsage( FILE* outStream ) {
   PRINT_USAGE( outStream, "Usage: memscan [OPTION]...\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "The options below may be used to select memscan's operation\n" ) ;
   PRINT_USAGE( outStream, "  -b, --block=FILE         open FILE using block I/O before the memscan\n" ) ;
   PRINT_USAGE( outStream, "      --stream=FILE        open FILE using stream I/O before the memscan\n" ) ;
   PRINT_USAGE( outStream, "      --mmap=FILE          open FILE using memory mapped I/O before the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -f, --fork               fork a process and display the combined parent and\n" ) ;
   PRINT_USAGE( outStream, "                           child memscan\n" ) ;
   PRINT_USAGE( outStream, "  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -p, --phys               include physical addresses (w/ flags) in the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -s, --shared=NUM[K|M|G]  create a shared memory region of NUM bytes before\n" ) ;
   PRINT_USAGE( outStream, "                           the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -t, --threads=NUM        create NUM threads before the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -h, --help               display this help and exit\n" ) ;
   PRINT_USAGE( outStream, "  -v, --version            output version information and exit\n" ) ;
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
