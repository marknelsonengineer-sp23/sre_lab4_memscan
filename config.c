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

#include <ctype.h>   // For isdigit()
#include <errno.h>   // For errno
#include <getopt.h>  // For getopt_long() struct option
#include <limits.h>  // For PATH_MAX UINT_MAX
#include <locale.h>  // For set_locale() LC_NUMERIC
#include <stddef.h>  // For NULL
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For exit() EXIT_SUCCESS EXIT_FAILURE
#include <string.h>  // For strlen() strncpy()
#include <sys/capability.h>  // For cap_get_proc() cap_set_flag() cap_set_proc() cap_free()

#include "config.h"  // Just cuz
#include "convert.h" // For stringToUnsignedLongLongWithScale() stringToUnsignedLongLongWithBasePrefix()
#include "iomem.h"   // For summarize_iomem()
#include "pagemap.h" // For getPageSizeInBytes()
#include "trim.h"    // For trim()
#include "version.h" // For FULL_VERSION

/// The x86 `RET` machine instruction (for both 32- and 64-bit machines) which
/// is the default value for the `--scan_byte` option
#define X86_RET_INSTRUCTION 0xC3

struct Filter* filterHead ;


enum Endian getEndianness() {
   int i = 1;

   if( *(char *)&i == 1 ) {  // little endian if true
      return LITTLE ;
   }
   return BIG ;
}


void printUsage( FILE* outStream ) {
   ASSERT( outStream != NULL ) ;

   PRINT( outStream, "Usage: memscan [PRE-SCAN OPTIONS]... [SCAN OPTIONS]... [OUTPUT OPTIONS]... [FILTER]... \n" ) ;
   PRINT( outStream, "       memscan --pid=NUM [OUTPUT OPTIONS]... [FILTER]...\n" ) ;
   PRINT( outStream, "       memscan -i|--iomem\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "  When FILTER is present, only process sections that match a filter\n" ) ;
   PRINT( outStream, "  If FILTER is a decimal number, match it to the region's index\n" ) ;
   PRINT( outStream, "  If FILTER is a hex number, include regions with that virtual address\n" ) ;
   PRINT( outStream, "  If FILTER starts with + and r w or x then include sections with that permission\n" ) ;
   PRINT( outStream, "  When FILTER is not present, process all sections\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "The options below may be used to select memscan's operation\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "PRE-SCAN OPTIONS\n" ) ;
   PRINT( outStream, "  -b, --block=FILE         open FILE using block I/O before the memscan\n" ) ;
   PRINT( outStream, "      --stream=FILE        open FILE using stream I/O before the memscan\n" ) ;
   PRINT( outStream, "      --map_file=FILE      open FILE using memory mapped I/O before the memscan\n" ) ;
   PRINT( outStream, "  -r, --read               read the contents of the files\n" ) ;
// PRINT( outStream, "  -f, --fork               fork a process and display the combined parent and\n" ) ;
// PRINT( outStream, "                           child memscan\n" ) ;
   PRINT( outStream, "  -l, --local=NUM[K|M|G]   allocate NUM bytes in local variables before the\n" ) ;
   PRINT( outStream, "                           memscan\n" ) ;
   PRINT( outStream, "      --numLocal=NUM       number of local allocations\n" ) ;
   PRINT( outStream, "  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan\n" ) ;
   PRINT( outStream, "      --numMalloc=NUM      number of malloc'd allocations\n" ) ;
   PRINT( outStream, "      --map_mem=NUM[K|M|G] allocate NUM bytes of memory via mmap before the\n" ) ;
   PRINT( outStream, "                           memscan\n" ) ;
   PRINT( outStream, "      --map_addr=ADDR      the starting address of the memory map\n" ) ;
   PRINT( outStream, "                           by default, the OS will select an address\n" ) ;
   PRINT( outStream, "      --fill               fill the local, malloc'd and/or mapped memory\n" ) ;
   PRINT( outStream, "                           with data before the memscan\n" ) ;
   PRINT( outStream, "  -t, --threads=NUM        create NUM threads before the memscan\n" ) ;
   PRINT( outStream, "      --sleep=SECONDS      pause the primary thread before scanning\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "SCAN OPTIONS\n" ) ;
   PRINT( outStream, "      --scan_byte[=NUM]    scan for NUM (a byte from 0x00 to 0xff)\n" ) ;
   PRINT( outStream, "                           or c3 (the x86 RET instruction) by default\n" ) ;
   PRINT( outStream, "      --shannon            compute Shannon Entropy for each mapped region\n" ) ;
   PRINT( outStream, "                           and physical page\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "OUTPUT OPTIONS\n" ) ;
   PRINT( outStream, "      --path               print the path (if available) in the memscan\n" ) ;
   PRINT( outStream, "  -p, --phys               print a summary of the physical pages w/ flags\n" ) ;
   PRINT( outStream, "  -P  --pfn                print each physical page number w/ flags\n" ) ;
   PRINT( outStream, "                           --pnf and --phys are exclusive\n" ) ;
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
   { "map_file",  required_argument, 0, '1' },
   { "read",      no_argument,       0, 'r' },
   { "fork",      no_argument,       0, 'f' },
   { "local",     required_argument, 0, 'l' },
   { "numLocal",  required_argument, 0, 'L' },
   { "malloc",    required_argument, 0, 'm' },
   { "numMalloc", required_argument, 0, '6' },
   { "map_mem",   required_argument, 0, '7' },
   { "map_addr",  required_argument, 0, '8' },
   { "fill",      no_argument      , 0, '2' },
   { "threads",   required_argument, 0, 't' },
   { "sleep",     required_argument, 0, '9' },
   // SCAN OPTIONS
   { "scan_byte", optional_argument, 0, '3' },
   { "shannon",   no_argument,       0, '4' },
   // OUTPUT OPTIONS
   { "iomem",     no_argument,       0, 'i' },
   { "path",      no_argument,       0, '5' },
   { "phys",      no_argument,       0, 'p' },
   { "pfn",       no_argument,       0, 'P' },
   // PROGRAM OPTIONS
   { "help",      no_argument,       0, 'h' },
   { "key",       no_argument,       0, 'k' },
   { "version",   no_argument,       0, 'v' },
   { 0, 0, 0, 0 }
};

/// Define the single character option string
const char SINGLE_OPTION_STRING[] = "b:rfl:m:t:pPhkv" ;


// All of these globals are set in reset_config()
bool openFileWithBlockIO ;
bool openFileWithStreamIO ;
bool openFileWithMapIO ;
bool readFileContents ;
bool forkProcess ;
bool allocateLocalMemory ;
bool allocateHeapMemory ;
bool allocateMappedMemory ;
bool fillAllocatedMemory ;
bool scanForByte ;
bool scanForShannon ;
bool iomemSummary ;
bool printPath ;
bool includePhysicalPageSummary ;
bool includePhysicalPageNumber ;

char blockPath[ FILENAME_MAX ] ;
char streamPath [ FILENAME_MAX ] ;
char mapFilePath [ FILENAME_MAX ] ;

size_t localSize ;
size_t numLocals ;
size_t mallocSize ;
size_t numMallocs ;
size_t mappedSize ;
void*  mappedStart ;
size_t numThreads ;
unsigned int sleepSeconds ;

unsigned char byteToScanFor ;

char mapsFilePath[ FILENAME_MAX ] ;
char pagemapFilePath[ FILENAME_MAX ] ;
char iomemFilePath[ FILENAME_MAX ] ;


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
            ASSERT( optarg != NULL ) ;
            openFileWithBlockIO = true ;
            strncpy( blockPath, optarg, sizeof( blockPath ) ) ;
            trim( blockPath ) ;
            if( strlen( blockPath ) == 0 ) {
               FATAL_ERROR( "--block must have a filename" ) ;
            }
            break ;

         case '0':
            ASSERT( optarg != NULL ) ;
            openFileWithStreamIO = true ;
            strncpy( streamPath, optarg, sizeof( streamPath ) ) ;
            trim( streamPath ) ;
            if( strlen( streamPath ) == 0 ) {
               FATAL_ERROR( "--stream must have a filename" ) ;
            }
            break ;

         case '1':
            ASSERT( optarg != NULL ) ;
            openFileWithMapIO = true ;
            strncpy( mapFilePath, optarg, sizeof( mapFilePath ) ) ;
            trim( mapFilePath ) ;
            if( strlen( mapFilePath ) == 0 ) {
               FATAL_ERROR( "--map_file must have a filename" ) ;
            }
            break ;

         case 'r':
            readFileContents = true ;
            break ;

         case 'f':
            forkProcess = true ;
            break ;

         case 't': {
            size_t trialValue = stringToUnsignedLongLongWithBasePrefix( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            numThreads = (size_t) trialValue ;
            }
            break ;

         case 'l': {
            unsigned long long trialValue = stringToUnsignedLongLongWithScale( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            localSize = (size_t) trialValue ;
            allocateLocalMemory = true ;
            }
            break ;

         case 'L': {
            unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            numLocals = (size_t) trialValue ;
            }
            break ;

         case 'm': {
            unsigned long long trialValue = stringToUnsignedLongLongWithScale( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            mallocSize = (size_t) trialValue ;
            allocateHeapMemory = true ;
            }
            break ;

         case '6': {
            unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            numMallocs = (size_t) trialValue ;
            }
            break ;

         case '7': {
            unsigned long long trialValue = stringToUnsignedLongLongWithScale( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            mappedSize = (size_t) trialValue ;
            allocateMappedMemory = true ;
            }
            break ;

         case '8': {
            unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            mappedStart = (void*) trialValue ;  /// @NOLINT(performance-no-int-to-ptr): integer to void* is intended
            }
            break ;

         case '2':
            fillAllocatedMemory = true ;
            break ;

         case 'p':
            includePhysicalPageSummary = true ;
            break ;

         case 'P':
            includePhysicalPageNumber = true ;
            break ;

         case '9': {
            unsigned long long trialValue = stringToUnsignedLongLongWithScale( optarg ) ;
            ASSERT( trialValue <= UINT_MAX ) ;
            sleepSeconds = (unsigned int) trialValue ;
            }
            break ;

         case '3':
            if( optarg != NULL ) {
               unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( optarg );
               ASSERT( trialValue <= UCHAR_MAX );
               byteToScanFor = (unsigned char) trialValue;
            }
            scanForByte = true;
            break ;

         case '4':
            scanForShannon = true ;
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
            /// @todo Colorize the `--key` output
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

   // Search the remaining arguments, which should be filters
   while (optind < argc) {
      // Allocate and zero out a new Filter
      struct Filter* newFilter = malloc( sizeof ( struct Filter ) ) ;
      if( newFilter == NULL ) {
         FATAL_ERROR( "unable to allocate memory for newFilter") ;
      }
      memset( newFilter, 0, sizeof( struct Filter ) ) ;

      // Search for addresses:  Filters that start with 0x...
      if( argv[optind][0] == '0' && argv[optind][1] == 'x' ) {
         unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( argv[optind] ) ;
         ASSERT( trialValue <= SIZE_MAX ) ;
         newFilter->address = (void*) trialValue ;  /// @NOLINT(performance-no-int-to-ptr): integer to void* is intended
         newFilter->type = ADDRESS ;
         goto NextOption ;
      }

      // Search for indexes:  Filters that start with a number...
      if( isdigit( argv[optind][0] ) ) {
         unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( argv[optind] ) ;
         ASSERT( trialValue <= SIZE_MAX ) ;
         newFilter->index = (size_t) trialValue ;
         newFilter->type = INDEX ;
         goto NextOption ;
      }

      // Search for permissions:  Filters that start with a +...
      if( argv[optind][0] == '+' ) {
         newFilter->read    = ( strchr( argv[optind], 'r' ) != NULL ) ;
         newFilter->write   = ( strchr( argv[optind], 'w' ) != NULL ) ;
         newFilter->execute = ( strchr( argv[optind], 'x' ) != NULL ) ;
         newFilter->type = PERMISSION ;
         goto NextOption ;
      }

      newFilter->pattern = malloc( strlen( argv[optind] ) ) ;
      if( newFilter->pattern == NULL ) {
         FATAL_ERROR( "unable to allocate memory for a filter's pattern") ;
      }
      strncpy( newFilter->pattern, argv[optind], strlen( argv[optind] ) ) ;
      newFilter->type = PATTERN ;

      NextOption:
      // Insert newFilter to the linked list
      newFilter->next = filterHead ;
      filterHead = newFilter ;

      #ifdef DEBUG
         printf( "newFilter->type=%d\n",      newFilter->type ) ;
         printf( "newFilter->pattern=[%s]\n", newFilter->pattern ) ;
         printf( "newFilter->index=%zu\n",    newFilter->index ) ;
         printf( "newFilter->address=%p\n",   newFilter->address ) ;
         printf( "newFilter->read=%d\n",      newFilter->read ) ;
         printf( "newFilter->write=%d\n",     newFilter->write ) ;
         printf( "newFilter->execute=%d\n",   newFilter->execute ) ;
         printf( "newFilter->next=%p\n",      newFilter->next ) ;
      #endif

      optind += 1 ;
   }
} // processOptions


/// Buffer to hold the program name
#ifdef TARGET
   char programName[ MAX_PROGRAM_NAME ] = TARGET ;
#else
   char programName[ MAX_PROGRAM_NAME ] = {} ;
#endif


void checkCapabilities() {
   cap_t myCapabilities ;
   const cap_value_t requiredCapabilitiesList[1] = { CAP_SYS_ADMIN } ;

   if( !CAP_IS_SUPPORTED( CAP_SYS_ADMIN )) {
      FATAL_ERROR( "system does not support CAP_SYS_ADMIN" ) ;
   }

   myCapabilities = cap_get_proc() ;
   if( myCapabilities == NULL ) {
      FATAL_ERROR( "unable to get the process' capabilities" ) ;
   }

   int rVal = cap_set_flag( myCapabilities, CAP_EFFECTIVE, 1, requiredCapabilitiesList, CAP_SET) ;
   if( rVal != 0 ) {
      FATAL_ERROR( "cap_set_flag did not set a capability" ) ;
   }

   rVal = cap_set_proc( myCapabilities ) ;
   if( rVal != 0 ) {
      FATAL_ERROR( "memscan requires CAP_SYS_ADMIN to run" ) ;
   }

   rVal = cap_free( myCapabilities ) ;
   if( rVal != 0 ) {
      FATAL_ERROR( "cap_free failed" ) ;
   }
}


void setProgramName( const char* newProgramName ) {
   ASSERT( newProgramName != NULL ) ;

   char trialProgramName[ MAX_PROGRAM_NAME ] = {} ;
   strncpy( trialProgramName, newProgramName, MAX_PROGRAM_NAME ) ;

   trim( trialProgramName ) ;

   ASSERT( strlen( trialProgramName ) != 0 ) ;

   strncpy( programName, trialProgramName, MAX_PROGRAM_NAME - 1 ) ;
} // setProgramName


char* getProgramName() {
   return programName ;
}


void reset_config() {
   openFileWithBlockIO        = 0 ;
   openFileWithStreamIO       = 0 ;
   openFileWithMapIO          = 0 ;
   readFileContents           = 0 ;
   forkProcess                = 0 ;
   allocateLocalMemory        = 0 ;
   allocateHeapMemory         = 0 ;
   allocateMappedMemory       = 0 ;
   fillAllocatedMemory        = 0 ;
   scanForByte                = 0 ;
   scanForShannon             = 0 ;
   iomemSummary               = 0 ;
   printPath                  = 0 ;
   includePhysicalPageSummary = 0 ;
   includePhysicalPageNumber  = 0 ;

   memset( blockPath,   0, FILENAME_MAX ) ;
   memset( streamPath,  0, FILENAME_MAX ) ;
   memset( mapFilePath, 0, FILENAME_MAX ) ;

   localSize  = 0 ;
   numLocals  = 1 ;      /// By default, `--local` will allocate 1 region
   mallocSize = 0 ;
   numMallocs = 1 ;      /// By default, `--malloc` will allocate 1 region
   mappedSize = 0 ;
   mappedStart = NULL ;  /// By default, the OS will select a start address
   numThreads = 0 ;
   sleepSeconds = 0 ;    /// By default, don't sleep

   byteToScanFor = X86_RET_INSTRUCTION ;

   memset( mapsFilePath,    0, FILENAME_MAX ) ;
   memset( pagemapFilePath, 0, FILENAME_MAX ) ;
   memset( iomemFilePath,   0, FILENAME_MAX ) ;

   strncpy( mapsFilePath,    "/proc/self/maps",    FILENAME_MAX ) ;
   strncpy( pagemapFilePath, "/proc/self/pagemap", FILENAME_MAX ) ;
   strncpy( iomemFilePath,   "/proc/iomem",        FILENAME_MAX ) ;

   /// Free the #Filter linked list from #filterHead
   struct Filter* currentFilter = filterHead ;
   while( currentFilter != NULL ) {
      if( currentFilter->pattern != NULL ) {
         free( currentFilter->pattern ) ;
         currentFilter->pattern = NULL ;
      }
      struct Filter* oldFilter = currentFilter ;
      currentFilter = currentFilter->next ;
      free( oldFilter ) ;
   }

   filterHead = NULL ;
}
