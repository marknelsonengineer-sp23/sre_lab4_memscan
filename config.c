///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Process command line parameters and hold configuration for memscan
///
/// @see https://linux.die.net/man/3/getopt_long
///
/// @file   config.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <assert.h>  // For assert()
#include <errno.h>   // For errno
#include <getopt.h>  // For getopt_long() struct option
#include <limits.h>  // For PATH_MAX
#include <locale.h>  // For set_locale() LC_NUMERIC
#include <stddef.h>  // For NULL
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For exit() EXIT_SUCCESS EXIT_FAILURE
#include <string.h>  // For strlen() strncpy()

#include "config.h"  // Just cuz
#include "iomem.h"   // For summarize_iomem()
#include "pagemap.h" // For getPageSizeInBytes()
#include "trim.h"    // For trim()
#include "version.h" // For FULL_VERSION

/// The x86 `RET` machine instruction (for both 32- and 64-bit machines) which
/// is the default value for the `--scan_byte` option
#define X86_RET_INSTRUCTION 0xC3


struct IncludePattern* patternHead = NULL ;


enum Endian getEndianness() {
   int i = 1;

   if( *(char *)&i == 1 ) {  // little endian if true
      return LITTLE ;
   }
   return BIG ;
}


void printUsage( FILE* outStream ) {
   PRINT( outStream, "Usage: memscan [OPTION]... [PATTERN]... \n" ) ;
   PRINT( outStream, "       memscan -i|--iomem\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "  When PATTERN is present, only process sections with a path that includes PATTERN\n" ) ;
   PRINT( outStream, "  If PATTERN is 'r' 'w' or 'x' then include sections with that permission\n" ) ;
   PRINT( outStream, "  When PATTERN is not present, process all sections\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "The options below may be used to select memscan's operation\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "PRE-SCAN OPTIONS\n" ) ;
   PRINT( outStream, "  -b, --block=FILE         open FILE using block I/O before the memscan\n" ) ;
   PRINT( outStream, "      --stream=FILE        open FILE using stream I/O before the memscan\n" ) ;
   PRINT( outStream, "      --mmap=FILE          open FILE using memory mapped I/O before the memscan\n" ) ;
   PRINT( outStream, "  -r, --read               read the contents of the files\n" ) ;
   PRINT( outStream, "  -f, --fork               fork a process and display the combined parent and\n" ) ;
   PRINT( outStream, "                           child memscan\n" ) ;
   PRINT( outStream, "  -l, --local=NUM[K|M|G]   allocate NUM bytes in local variables before the\n" ) ;
   PRINT( outStream, "                           memscan\n" ) ;
   PRINT( outStream, "  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan\n" ) ;
   PRINT( outStream, "  -s, --shared=NUM[K|M|G]  allocate NUM bytes of shared memory before the\n" ) ;
   PRINT( outStream, "                           memscan\n" ) ;
   PRINT( outStream, "      --fill               fill the local, malloc'd and/or shared memory\n" ) ;
   PRINT( outStream, "                           with data before the memscan\n" ) ;
   PRINT( outStream, "  -t, --threads=NUM        create NUM threads before the memscan\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "SCAN OPTIONS\n" ) ;
   PRINT( outStream, "      --scan_byte[=HEX]    scan for HEX (a byte from 00 to ff)\n" ) ;
   PRINT( outStream, "                           or c3 (the x86 RET instruction) by default\n" ) ;
   PRINT( outStream, "      --shannon            compute Shannon Entropy for each mmap region\n" ) ;
   PRINT( outStream, "                           and physical page\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "OUTPUT OPTIONS\n" ) ;
   PRINT( outStream, "  -i, --iomem              print a summary of /proc/iomem\n" ) ;
   PRINT( outStream, "      --path               print the path (if available) in the memscan\n" ) ;
   PRINT( outStream, "  -p, --phys               print physical page numbers w/ flags in the memscan\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "PROGRAM OPTIONS\n" ) ;
   PRINT( outStream, "  -h, --help               display this help and exit\n" ) ;
   PRINT( outStream, "  -k, --key                display key to the --phys flags\n" ) ;
   PRINT( outStream, "  -v, --version            output version information and exit\n" ) ;
}


/// Define the long command line options for memscan and map them to
/// #SINGLE_OPTION_STRING
///
/// @see https://linux.die.net/man/3/getopt_long
static struct option long_options[] = {
   // PRE-SCAN OPTIONS
   { "block",     required_argument, 0, 'b' },
   { "stream",    required_argument, 0, '0' },
   { "mmap",      required_argument, 0, '1' },
   { "read",      no_argument,       0, 'r' },
   { "fork",      no_argument,       0, 'f' },
   { "local",     required_argument, 0, 'l' },
   { "malloc",    required_argument, 0, 'm' },
   { "shared",    required_argument, 0, 's' },
   { "fill",      no_argument      , 0, '2' },
   { "threads",   required_argument, 0, 't' },
   // SCAN OPTIONS
   { "scan_byte", optional_argument, 0, '3' },
   { "shannon",   no_argument,       0, '4' },
   // OUTPUT OPTIONS
   { "iomem",     no_argument,       0, 'i' },
   { "path",      no_argument,       0, '5' },
   { "phys",      no_argument,       0, 'p' },
   // PROGRAM OPTIONS
   { "help",      no_argument,       0, 'h' },
   { "key",       no_argument,       0, 'k' },
   { "version",   no_argument,       0, 'v' },
   { 0, 0, 0, 0 }
};

/// Define the single character option string
const char SINGLE_OPTION_STRING[] = "b:rfl:m:s:t:iphkv" ;


bool openFileWithBlockIO       = 0 ;
bool openFileWithStreamIO      = 0 ;
bool openFileWithMapIO         = 0 ;
bool readFileContents          = 0 ;
bool forkProcess               = 0 ;
bool allocateLocalMemory       = 0 ;
bool allocateHeapMemory        = 0 ;
bool allocateSharedMemory      = 0 ;
bool fillAllocatedMemory       = 0 ;
bool createThreads             = 0 ;
bool scanForByte               = 0 ;
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

unsigned char byteToScanFor = X86_RET_INSTRUCTION ;


/// Get a number from a string (with units)
///
/// Memscan has options like:  `--local=NUM[K|M|G]`.  This function parses
/// NUM with the units.  The following units are allowed:
///
/// | Unit |              Scale |
/// |:----:|-------------------:|
/// |   k  |              1,000 |
/// |   K  |              1,024 |
/// |   m  |          1,000,000 |
/// |   M  |        1024 * 1024 |
/// |   g  |      1,000,000,000 |
/// |   G  | 1024 * 1024 * 1024 |
///
/// @see https://en.wikipedia.org/wiki/Binary_prefix
///
/// @NOLINTBEGIN(readability-magic-numbers): Magic numbers are allowed in this function
///
/// @param optarg The input string (this must be modifiable)
/// @return The number or a fatal error
size_t getOptargNumericValue( char* optarg ) {
   assert( optarg != NULL ) ;
   trim( optarg ) ;
   size_t result = 0 ;
   char* strtolRemainder = NULL ;

   if( optarg[0] == '-' ) {
      FATAL_ERROR( "negative number not allowed here" ) ;
   }

   result = strtol( optarg, &strtolRemainder, 10 ) ;
   // printf( "result=%ld  strtolRemainder=%p [%s]  errno=%d\n", result, strtolRemainder, strtolRemainder, errno ) ;

   // If there's an error or excess characters...
   if( errno != 0 ) {
      FATAL_ERROR( "illegal numeric option" ) ;
   }

   // If there's an error or excess characters...
   if( strlen( strtolRemainder ) > 0 ) {
      if( strcmp( strtolRemainder, "k" ) == 0 ) {
         result = result * 1000 ;
      } else if( strcmp( strtolRemainder, "K" ) == 0 ) {
         result = result * 1024 ;
      } else if( strcmp( strtolRemainder, "m" ) == 0 ) {
         result = result * 1000 * 1000 ;
      } else if( strcmp( strtolRemainder, "M" ) == 0 ) {
         result = result * 1024 * 1024 ;
      } else if( strcmp( strtolRemainder, "g" ) == 0 ) {
         result = result * 1000 * 1000 * 1000 ;
      } else if( strcmp( strtolRemainder, "G" ) == 0 ) {
         result = result * 1024 * 1024 * 1024 ;
      } else {
         FATAL_ERROR( "illegal numeric option" ) ;
      }
   }

   /// @todo Build unit tests for all of this
   // printf( "result=%ld\n", result ) ;
   return result ;
} // getOptargNumericValue
// @NOLINTEND(readability-magic-numbers)

void processOptions( int argc, char* argv[] ) {
   if( argc < 1 ) {
      printf( "Unexpected argument count [%d].  Exiting.\n", argc );
      exit( EXIT_FAILURE );
   }

   if( strlen( getProgramName() ) == 0 ) {
      setProgramName( argv[0] ) ;
   }

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
         case 'b':
            assert( optarg != NULL ) ;
            openFileWithBlockIO = true ;
            strncpy( blockPath, optarg, sizeof( blockPath ) ) ;
            trim( blockPath ) ;
            if( strlen( blockPath ) == 0 ) {
               FATAL_ERROR( "--block must have a filename" ) ;
            }
            break ;

         case '0':
            assert( optarg != NULL ) ;
            openFileWithStreamIO = true ;
            strncpy( streamPath, optarg, sizeof( streamPath ) ) ;
            trim( streamPath ) ;
            if( strlen( streamPath ) == 0 ) {
               FATAL_ERROR( "--stream must have a filename" ) ;
            }
            break ;

         case '1':
            assert( optarg != NULL ) ;
            openFileWithMapIO = true ;
            strncpy( mmapPath, optarg, sizeof( mmapPath ) ) ;
            trim( mmapPath ) ;
            if( strlen( mmapPath ) == 0 ) {
               FATAL_ERROR( "--mmap must have a filename" ) ;
            }
            break ;

         case 'r':
            readFileContents = true ;
            break ;

         case 'l':
            localSize = getOptargNumericValue( optarg ) ;
            if( localSize > 0 ) {
               allocateLocalMemory = true ;
            } else {
               FATAL_ERROR( "--local has illegal number" ) ;
            }
            break ;

         case 'm':
            mallocSize = getOptargNumericValue( optarg ) ;
            if( mallocSize > 0 ) {
               allocateHeapMemory = true ;
            } else {
               FATAL_ERROR( "--malloc has illegal number" ) ;
            }
            break ;

         case 's':
            sharedSize = getOptargNumericValue( optarg ) ;
            if( sharedSize > 0 ) {
               allocateSharedMemory = true;
            } else {
               FATAL_ERROR( "--shared has illegal number" ) ;
            }
            break ;

         case 'p':
            includePhysicalMemoryInfo = true ;
            break ;

         case '3':
            scanForByte = true ;
            if( scanForShannon ) {
               FATAL_ERROR( "can not simultaneously scan for Shannon Entropy and for byte") ;
            }

            if( optarg != NULL ) {
               int base = 10 ;   /// @NOLINT(readability-magic-numbers):  Base 10 is a legit magic number
               trim( optarg ) ;
               if( optarg[0] == '0' && optarg[1] == 'x' ) {
                  base = 16 ;    // NOLINT(readability-magic-numbers)
                  optarg += 2 ;  // Skip the '0x'
               }
               if( optarg[0] == '0' && optarg[1] == 'b' ) {
                  base = 2 ;
                  optarg += 2 ;  // Skip the '0b'
               }
               errno = 0 ;
               char* strtolRemainder = NULL ;
               long trialValue = strtol( optarg, &strtolRemainder, base ) ;
               // printf( "trialValue=%ld  strtolRemainder=%p [%s]  errno=%d\n", trialValue, strtolRemainder, strtolRemainder, errno ) ;

               // If there's an error or excess characters...
               if( errno != 0 || strlen( strtolRemainder ) > 0 ) {
                  FATAL_ERROR( "Illegal format for --scan_byte=HEX" ) ;
                  break ;
               }

               if( trialValue < 0 || trialValue > 255 ) {  // NOLINT(readability-magic-numbers)
                  FATAL_ERROR( "--scan_byte=HEX is out of range" ) ;
                  break ;
               }

               byteToScanFor = trialValue ;
            } // if( optarg != NULL )
            break ;

         case '4':
            scanForShannon = true ;
            if( scanForByte ) {
               FATAL_ERROR( "can not simultaneously scan for Shannon Entropy and for byte") ;
            }
            break ;

         case 'i':
            iomemSummary = true ;
            break ;

         case 'v':
            printf( "%s version %s ", getProgramName(), FULL_VERSION ) ;
            printf( "running on a %zu-bit ", sizeof( void* ) << 3 ) ;
            printf( "%s Endian system ", getEndianness() == BIG ? "Big" : "Little" ) ;
            printf( "with %'zu-byte page size\n", getPageSizeInBytes() ) ;
            printf( "Copyright (C) 2023 Mark Nelson, All rights reserved.\n") ;
            printf( "Written by Mark Nelson\n" );
            exit( EXIT_SUCCESS ) ;
            break ;

         case 'h':
            printUsage( stdout ) ;
            exit( EXIT_SUCCESS ) ;
            break ;

         case 'k':
            printKey( stdout ) ;
            exit( EXIT_SUCCESS ) ;
            break ;

         case '5':
            printPath = true ;
            break;

         default:
            printUsage( stderr ) ;
            exit( EXIT_FAILURE ) ;
            break ;
      }
   }

   while (optind < argc) {
      /// @todo I am not `free`ing this memory and I should be
      struct IncludePattern* newPattern = malloc( sizeof ( struct IncludePattern ) ) ;
      if( newPattern == NULL ) {
         FATAL_ERROR( "unable to allocate memory for newPattern") ;
      }
      newPattern->pattern = malloc( strlen( argv[optind] ) ) ;
      if( newPattern->pattern == NULL ) {
         FATAL_ERROR( "unable to allocate memory for pattern") ;
      }
      strncpy( newPattern->pattern, argv[optind], strlen( argv[optind] ) ) ;

      // Insert newPattern to the linked list
      newPattern->next = patternHead ;
      patternHead = newPattern ;

      optind += 1 ;
   }
} // processOptions


/// Buffer to hold the program name
#ifdef TARGET
   char programName[ MAX_PROGRAM_NAME ] = TARGET ;
#else
   char programName[ MAX_PROGRAM_NAME ] = {} ;
#endif


bool setProgramName( const char* newProgramName ) {
   if( newProgramName == NULL ) {
      /// @todo Print an appropriate error message
      return false ;
   }

   char trialProgramName[ MAX_PROGRAM_NAME ] = {} ;
   strncpy( trialProgramName, newProgramName, MAX_PROGRAM_NAME ) ;

   trim( trialProgramName ) ;

   if( strlen( trialProgramName ) == 0 ) {
      /// @todo Print an appropriate error message
      return false ;
   }

   strncpy( programName, trialProgramName, MAX_PROGRAM_NAME );

   return true;
} // setProgramName


char* getProgramName() {
   return programName;
}
