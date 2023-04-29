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
   int i = 1 ;

   if( *(char *)&i == 1 ) {  // little endian if true
      return LITTLE ;
   }
   return BIG ;
}


void printUsage( FILE* outStream ) {
   ASSERT( outStream != NULL ) ;

   PRINT( outStream, "Usage: memscan [PRE-SCAN OPTIONS]... [SCAN OPTIONS]... [OUTPUT OPTIONS]... [FILTER]... \n" ) ;
   PRINT( outStream, "       memscan --pid=NUM [OUTPUT OPTIONS]... [FILTER]...\n" ) ;
   PRINT( outStream, "       memscan --iomem\n" ) ;
   PRINT( outStream, "       memscan-static\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "  When FILTER is present, only process memory regions that match:\n" ) ;
   PRINT( outStream, "    - If FILTER is a decimal number, match on the region's index\n" ) ;
   PRINT( outStream, "    - If FILTER is a hex number (address), match the region that has the address\n" ) ;
   PRINT( outStream, "    - If FILTER prefix is +, followed by r w or x, include regions with that\n" ) ;
   PRINT( outStream, "      permission set\n" ) ;
   PRINT( outStream, "  When no FILTERs are present, process all regions\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "memscan-static has the same usage as memscan.  The difference is that it's\n" ) ;
   PRINT( outStream, "built statically.\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "TARGETING OPTIONS\n" ) ;
   PRINT( outStream, "  --pid=NUM          Scan process ID (by default, it scans itself)\n" ) ;
   PRINT( outStream, "  --iomem            Print a summary of /proc/iomem and exit\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "PRE-SCAN OPTIONS\n" ) ;
   PRINT( outStream, "  --block=FILE       Open FILE using block I/O before the scan\n" ) ;
   PRINT( outStream, "  --stream=FILE      Open FILE using stream I/O before the scan\n" ) ;
   PRINT( outStream, "  --map_file=FILE    Open FILE using memory mapped I/O before the scan\n" ) ;
   PRINT( outStream, "  --read             Read the contents of the files\n" ) ;
// PRINT( outStream, "  --fork             Fork a process and display the combined parent & child scan\n" ) ;
   PRINT( outStream, "  --local=SIZE       Allocate SIZE bytes in local variables before the scan\n" ) ;
   PRINT( outStream, "  --numLocal=NUM     Number of local allocations\n" ) ;
   PRINT( outStream, "  --malloc=SIZE      Malloc SIZE bytes before the scan\n" ) ;
   PRINT( outStream, "  --numMalloc=NUM    Number of malloc'd allocations\n" ) ;
   PRINT( outStream, "  --map_mem=SIZE     Allocate SIZE bytes of memory via mmap before the scan\n" ) ;
   PRINT( outStream, "  --map_addr=ADDR    The starting address of the memory map; by default, the OS\n" ) ;
   PRINT( outStream, "                     will select an address\n" ) ;
   PRINT( outStream, "  --fill             Fill the local, malloc'd and/or mapped memory with data\n" ) ;
   PRINT( outStream, "                     before the scan\n" ) ;
   PRINT( outStream, "  --threads=NUM      Create NUM threads before the scan\n" ) ;
   PRINT( outStream, "  --sleep=NUM        Pause the primary thread for NUM seconds before scanning\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "SCAN OPTIONS\n" ) ;
   PRINT( outStream, "  --scan_byte[=NUM]  Scan for NUM (a byte from 0x00 to 0xff)\n" ) ;
   PRINT( outStream, "                     or c3 (the x86 RET instruction) by default\n" ) ;
   PRINT( outStream, "  --shannon          Compute Shannon Entropy for each mapped region\n" ) ;
   PRINT( outStream, "                     and physical page\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "OUTPUT OPTIONS\n" ) ;
   PRINT( outStream, "  --path             Print the path (if available)\n" ) ;
   PRINT( outStream, "  --pfn              Print each physical page number w/ flags\n" ) ;
   PRINT( outStream, "  --phys             Print a summary of the physical pages w/ flags\n" ) ;
   PRINT( outStream, "                     --pfn and --phys are exclusive\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "PROGRAM OPTIONS\n" ) ;
   PRINT( outStream, "  -h, --help         Display this help and exit\n" ) ;
   PRINT( outStream, "  -k, --key          Display key to the --pfn & --phys flags and exit\n" ) ;
   PRINT( outStream, "  -v, --version      Display memscan's version and exit\n" ) ;
   PRINT( outStream, "\n" ) ;
   PRINT( outStream, "SIZE may end in K|M|G to scale by 1,024 or k|m|g to scale by 1,000\n" ) ;
   PRINT( outStream, "NUM and ADDR may be prefixed by 0x (for hex) or 0b (for binary)\n" ) ;
   PRINT( outStream, "\n" ) ;
}


/// Define the long command line options for memscan and map them to
/// #SINGLE_OPTION_STRING
///
/// @see https://linux.die.net/man/3/getopt_long
static struct option long_options[] = {
   // TARGETING OPTIONS
   { "pid",       required_argument, 0, 'D' },
   { "iomem",     no_argument,       0, 'i' },

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
   { "path",      no_argument,       0, '5' },
   { "phys",      no_argument,       0, 'p' },
   { "pfn",       no_argument,       0, 'P' },
   // PROGRAM OPTIONS
   { "help",      no_argument,       0, 'h' },
   { "key",       no_argument,       0, 'k' },
   { "version",   no_argument,       0, 'v' },
   { 0, 0, 0, 0 }
} ;

/// Define the single character option string
const char SINGLE_OPTION_STRING[] = "hkv" ;


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

bool  scanSelf ;
pid_t scanPid ;


void processOptions( int argc, char* argv[] ) {
   ASSERT( argc >= 1 ) ;  // There's always at least 1 argument

   if( strlen( getProgramName() ) == 0 ) {
      setProgramName( argv[0] ) ;
   }

   /// Set locale so numbers we can print localized numbers i.e. `1,024`.
   char* sRetVal ;
   sRetVal = setlocale( LC_NUMERIC, "" ) ;
   if( sRetVal == NULL ) {
      FATAL_ERROR( "Unable to set locale" ) ;
   }

   optind = 1 ;  // Reset the option index
   while( true ) {
      int option_index = 0 ;
      int optionChar ;

      optionChar = getopt_long(argc, argv, SINGLE_OPTION_STRING, long_options, &option_index) ;

      if( optionChar == -1 ) {
         break ;  // Done processing getopt_long
      }

      switch ( optionChar ) {
         case 'D': {
            unsigned long long trialValue = stringToUnsignedLongLongWithScale( optarg ) ;
            ASSERT( trialValue <= SIZE_MAX ) ;
            scanPid = (int) trialValue ;
            scanSelf = false ;
            sprintf( mapsFilePath, "/proc/%d/maps", scanPid ) ;        /// NOLINT(cert-err33-c): It's OK to ignore the result of `sprintf`
            sprintf( pagemapFilePath, "/proc/%d/pagemap", scanPid ) ;  //  NOLINT(cert-err33-c): It's OK to ignore the result of `sprintf`
            }
            break ;

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
               unsigned long long trialValue = stringToUnsignedLongLongWithBasePrefix( optarg ) ;
               ASSERT( trialValue <= UCHAR_MAX ) ;
               byteToScanFor = (unsigned char) trialValue ;
            }
            scanForByte = true ;
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
            printf( "Written by Mark Nelson\n" ) ;
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
            break ;

         default:
            printUsage( stderr ) ;
            exit( EXIT_FAILURE ) ;
            break ;
      }
   }

   // Search the remaining arguments, which should be filters
   /// @API{ calloc, https://man.archlinux.org/man/calloc.3 }
   while (optind < argc) {
      // Allocate and zero out a new Filter
      struct Filter* newFilter = calloc( 1, sizeof ( struct Filter ) ) ;
      if( newFilter == NULL ) {
         FATAL_ERROR( "unable to allocate memory for newFilter") ;
      }

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

   /// @API{ strncpy, https://man.archlinux.org/man/strncpy.3 }
   char trialProgramName[ MAX_PROGRAM_NAME ] ;
   strncpy( trialProgramName, newProgramName, MAX_PROGRAM_NAME ) ;
   trialProgramName[ MAX_PROGRAM_NAME - 1 ] = '\0' ;
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

   scanSelf = true ;
   scanPid  = -1 ;

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


/// Check the PRE-SCAN OPTIONS and SCAN OPTIONS
///
/// This is an internal helper routine.
///
/// @return `true` if any of the PRE-SCAN OPTIONS or SCAN OPTIONS are set
bool hasScanOptions() {
   return   blockPath[0]   != '\0'
         || streamPath[0]  != '\0'
         || mapFilePath[0] != '\0'
         || readFileContents
         || localSize != 0
         || numLocals != 1
         || mallocSize != 0
         || numMallocs != 1
         || mappedSize != 0
         || mappedStart != NULL
         || fillAllocatedMemory
         || numThreads != 0
         || sleepSeconds != 0
         || scanForByte
         || scanForShannon
         ;
}


/// Check the OUTPUT OPTIONS
///
/// This is an internal helper routine.
///
/// @return `true` if any of the OUTPUT OPTIONS are set
bool hasOutputOptions() {
   return   printPath
         || includePhysicalPageNumber
         || includePhysicalPageSummary
         ;
}


/// Call when a validation has failed in validateConfig().
///
/// If `printReason` is set, then print a message.
/// In all cases, return `false`
#define FAILED_VALIDATION( msg )  \
   if( printReason ) {            \
      WARNING( msg ) ;            \
   }                              \
   return false


bool validateConfig( const bool printReason ) {
   /// - If `--iomem` is set, then none of the hasScanOptions() nor
   ///   hasOutputOptions() can be set.  Also, `--iomem` is exclusive
   ///   with `--fork` and `--pid`.
   if( iomemSummary ) {
      if( scanPid >= 0 || forkProcess || hasScanOptions() || hasOutputOptions() ) {
         FAILED_VALIDATION( "illegal options with --iomem" ) ;
      }
   }

   /// - If `--pid` is set, then none of the hasScanOptions()
   /// can be set.  Also, `--pid` is exclusive with `--fork` and `--iomem`.
   if( scanPid >= 0 ) {
      if( iomemSummary || forkProcess || hasScanOptions() ) {
         FAILED_VALIDATION( "illegal options with --pid" ) ;
      }
   }

   /// - If `--fork` is set, then none of the hasScanOptions()
   /// can be set.  Also, `--fork` is exclusive with `--pid` and `--iomem`.
   if( forkProcess ) {
      if( scanPid >= 0 || iomemSummary || hasScanOptions() ) {
         FAILED_VALIDATION( "illegal options with --fork" ) ;
      }
   }

   /// - If `--read` is set, then at least `--block`, `--stream` or
   ///   `--map_file` must be set
   if( readFileContents ) {
      if(    blockPath[0]   == '\0'
          && streamPath[0]  == '\0'
          && mapFilePath[0] == '\0' ) {
         FAILED_VALIDATION( "no file IO options with --read" ) ;
      }
   }

   /// - If `--numLocal` is set, then `--local` must be set
   if( numLocals > 1 ) {
      if( localSize == 0 ) {
         FAILED_VALIDATION( "no --local option with --numLocal" ) ;
      }
   }

   /// - If `--numMalloc` is set, then `--malloc` must be set
   if( numMallocs > 1 ) {
      if( mallocSize == 0 ) {
         FAILED_VALIDATION( "no --malloc option with --numMalloc" ) ;
      }
   }

   /// - If `--map_addr` is set, then `--map_mem` must be set
   if( mappedStart != NULL ) {
      if( mappedSize == 0 ) {
         FAILED_VALIDATION( "no --map_mem option with --map_addr" ) ;
      }
   }

   /// - If `--fill` is set, then at least `--local`, `--malloc` or
   ///   `--map_mem` must be set
   if( fillAllocatedMemory ) {
      if(       localSize  == 0
             && mallocSize == 0
             && mappedSize == 0 ) {
         FAILED_VALIDATION( "no memory allocation options with --fill" ) ;
      }
   }

   /// - `--pfn` and `--phys` are exclusive
   if( includePhysicalPageSummary && includePhysicalPageNumber ) {
      FAILED_VALIDATION( "--pfn and --phys are exclusive" ) ;
   }

   return true ;
}


void stringCopy( char *restrict dest, const char *restrict src, size_t count ) {
   ASSERT( dest != NULL ) ;
   ASSERT( src != NULL ) ;
   ASSERT( count > 0 ) ;
   ASSERT( count < SIZE_MAX ) ;

   // Wraparound testing doesn't work the way I'd like it to...
   // ASSERT( dest < dest + count ) ;  // No wraparound!
   // ASSERT( src  < src  + count ) ;  // No wraparound!

   /// Ensure the two ranges are not overlapping
   /// @see https://stackoverflow.com/questions/36035074/how-can-i-find-an-overlap-between-two-given-ranges
   ASSERT( dest > src + count || dest + count < src ) ;

   strncpy( dest, src, count ) ;  // This is a bit more efficient than zeroing
   dest[ count - 1 ] = '\0' ;     // out the entire destination buffer
}
