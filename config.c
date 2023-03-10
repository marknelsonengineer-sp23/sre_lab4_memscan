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
#include <locale.h>  // For set_locale() LC_NUMERIC
#include <stddef.h>  // For NULL
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For exit() EXIT_SUCCESS EXIT_FAILURE
#include <string.h>  // For strlen() & strncpy()

#include "config.h"  // Just cuz
#include "iomem.h"   // For summarize_iomem()
#include "pagemap.h" // For getPageSizeInBytes()
#include "version.h" // For FULL_VERSION


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
char programName[ MAX_PROGRAM_NAME ] = {};

enum Endian getEndianness() {
   int i = 1;

   if( *(char *)&i == 1 ) {  // little endian if true
      return LITTLE ;
   }
   return BIG ;
}


void printUsage( FILE* outStream ) {
   PRINT_USAGE( outStream, "Usage: memscan [OPTION]\n" ) ;
   PRINT_USAGE( outStream, "       memscan -i|--iomem\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "The options below may be used to select memscan's operation\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "PRE-SCAN OPTIONS\n" ) ;
   PRINT_USAGE( outStream, "  -b, --block=FILE         open FILE using block I/O before the memscan\n" ) ;
   PRINT_USAGE( outStream, "      --stream=FILE        open FILE using stream I/O before the memscan\n" ) ;
   PRINT_USAGE( outStream, "      --mmap=FILE          open FILE using memory mapped I/O before the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -f, --fork               fork a process and display the combined parent and\n" ) ;
   PRINT_USAGE( outStream, "                           child memscan\n" ) ;
   PRINT_USAGE( outStream, "  -l, --local=NUM[K|M|G]   allocate NUM bytes in local variables before the\n" ) ;
   PRINT_USAGE( outStream, "                           memscan\n" ) ;
   PRINT_USAGE( outStream, "  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -s, --shared=NUM[K|M|G]  allocate NUM bytes of shared memory before the\n" ) ;
   PRINT_USAGE( outStream, "                           memscan\n" ) ;
   PRINT_USAGE( outStream, "      --fill               fill the local, malloc'd and/or shared memory\n" ) ;
   PRINT_USAGE( outStream, "                           with data before the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -t, --threads=NUM        create NUM threads before the memscan\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "SCAN OPTIONS\n" ) ;
   PRINT_USAGE( outStream, "      --scan_byte[=HEX]    scan for HEX (a byte from 00 to ff)\n" ) ;
   PRINT_USAGE( outStream, "                           or c3 (the x86 RET instruction) by default\n" ) ;
   PRINT_USAGE( outStream, "      --histogram          scan memory and generate a histogram of byte values\n" ) ;
   PRINT_USAGE( outStream, "      --shannon            compute Shannon Entropy for each mmap region\n" ) ;
   PRINT_USAGE( outStream, "                           and physical page\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "OUTPUT OPTIONS\n" ) ;
   PRINT_USAGE( outStream, "  -i, --iomem              print a summary of /proc/iomem\n" ) ;
   PRINT_USAGE( outStream, "      --path               print the path (if available in the memscan\n" ) ;
   PRINT_USAGE( outStream, "  -p, --phys               include physical addresses (w/ flags) in the memscan\n" ) ;
   PRINT_USAGE( outStream, "\n" ) ;
   PRINT_USAGE( outStream, "PROGRAM OPTIONS\n" ) ;
   PRINT_USAGE( outStream, "  -h, --help               display this help and exit\n" ) ;
   PRINT_USAGE( outStream, "  -v, --version            output version information and exit\n" ) ;
}


/// Define the long command line options for memscan and map them to
/// #SINGLE_OPTION_STRING
static struct option long_options[] = {
   // PRE-SCAN OPTIONS
   { "block",     required_argument, 0, 'b' },
   { "stream",    required_argument, 0, '0' },
   { "mmap",      required_argument, 0, '1' },
   { "fork",      no_argument,       0, 'f' },
   { "local",     required_argument, 0, 'l' },
   { "malloc",    required_argument, 0, 'm' },
   { "shared",    required_argument, 0, 's' },
   { "fill",      no_argument      , 0, '2' },
   { "threads",   required_argument, 0, 't' },
   // SCAN OPTIONS
   { "scan_byte", optional_argument, 0, '3' },
   { "histogram", no_argument,       0, '4' },
   { "shannon",   no_argument,       0, '5' },
   // OUTPUT OPTIONS
   { "iomem",     no_argument,       0, 'i' },
   { "path",      no_argument,       0, '6' },
   { "phys",      no_argument,       0, 'p' },
   // PROGRAM OPTIONS
   { "help",      no_argument,       0, 'h' },
   { "version",   no_argument,       0, 'v' },
   { 0, 0, 0, 0 }
};


/// Define the single character option string
const char SINGLE_OPTION_STRING[] = "b:fl:m:s:t:iphv" ;


bool openFileWithBlockIO       = 0 ;
bool openFileWithStreamIO      = 0 ;
bool openFileWithMapIO         = 0 ;
bool forkProcess               = 0 ;
bool allocateLocalMemory       = 0 ;
bool allocateMallocMemory      = 0 ;
bool allocateSharedMemory      = 0 ;
bool fillAllocatedMemory       = 0 ;
bool createThreads             = 0 ;
bool scanForByte               = 0 ;
bool scanForHistogram          = 0 ;
bool scanForShannon            = 0 ;
bool iomemSummary              = 0 ;
bool printPath                 = 0 ;
bool includePhysicalMemoryInfo = 0 ;

char blockPath[ FILENAME_MAX ]   = {} ;
char streamPath [ FILENAME_MAX ] = {} ;
char mmapPath [ FILENAME_MAX ]   = {} ;

size_t localSize  = 0 ;
size_t mallocSize = 0 ;
size_t sharedSize = 0 ;
size_t numThreads = 0 ;

unsigned char byteToScanFor = 0xC3 ;  ///< x86 `RET` instruction (near)


void processOptions( int argc, char* argv[] ) {
   if( argc < 1 ) {
      printf( "Unexpected argument count [%d].  Exiting.\n", argc );
      exit( EXIT_FAILURE );
   }

   setProgramName( argv[0] ) ;

   /// Set locale so numbers we can print localized numbers i.e. `1,024`.
   char* sRetVal;
   sRetVal = setlocale( LC_NUMERIC, "" ) ;
   if( sRetVal == NULL ) {
      FATAL_ERROR( "Unable to set locale" ) ;
   }

   while( true ) {
      int option_index = 0;
      int optionChar;

      optionChar = getopt_long(argc, argv, SINGLE_OPTION_STRING, long_options, &option_index);

      if( optionChar == -1 ) {
         break ;  // Done processing getopt_long
      }

      switch ( optionChar ) {
         case 'i':
            iomemSummary = true ;
            break ;

         case 'v':
            printf( "memscan version %s ", FULL_VERSION ) ;
            printf( "running on a %zu-bit ", sizeof( void* ) << 3 ) ;
            printf( "%s Endian system ", getEndianness() == BIG ? "Big" : "Little" ) ;
            printf( "with %'zu-byte page size\n", getPageSizeInBytes() ) ;
            printf( "Copyright (C) 2023 Mark Nelson\n") ;
            printf( "Written by Mark Nelson\n" );
            exit( EXIT_SUCCESS ) ;
            break ;

         case 'h':
            printUsage( stdout ) ;
            exit( EXIT_SUCCESS ) ;
            break ;

         case '6':
            printPath = true ;
            break;

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
