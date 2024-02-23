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

#include <assert.h>     // For assert()
#include <stdbool.h>    // For bool
#include <stdio.h>      // For FILENAME_MAX FILE stderr fprintf()
#include <stdlib.h>     // For exit()
#include <sys/types.h>  // For pid_t

#ifdef TESTING
   #include "tests/boost_test_util.h"  // For throwException()
#endif


/// Process command line options
///
/// @param argc The number of arguments passed to `memscan` (including the
///             program name).  Always `>= 1`.
/// @param argv A `NULL` terminated array of `char[]` arguments containing the
///             command line options
extern void processOptions( int argc, char* argv[] ) ;


/// Ensure memscan is running with the `CAP_SYS_ADMIN` capability
///
/// Return silently if it has the `CAP_SYS_ADMIN` capability.
/// Call FATAL_ERROR() if it does not have the `CAP_SYS_ADMIN` capability.
void checkCapabilities() ;


/// Print memscan's usage
///
/// @param outStream The output stream (usually `stderr` or `stdout`) to send
///                  the usage statement
void printUsage( FILE* outStream ) ;


#define MAX_PROGRAM_NAME  32  ///< The maximum size of the program name

#define ONE_MEGABYTE 1048576  ///< One megabyte of something

#define BASE_16           16  ///< Base 16

#define BASE_10           10  ///< Base 10

#define BASE_2             2  ///< Base 2

/// Set the program name
///
/// @param newProgramName The new program name
extern void setProgramName( const char* newProgramName ) ;


/// Get the program name
///
/// @return The program's name
extern char* getProgramName() ;

/// Enumeration for how computers hold multi-byte values
enum Endian { ENDIAN_UNKNOWN=-1  ///< If the endianness is unknown
             ,LITTLE             ///< For little endian machines
             ,BIG                ///< For big endian machines
} ;


/// Return the endianness of the current computer
///
/// @see https://en.wikipedia.org/wiki/Endianness
///
/// @return #LITTLE for Little Endian machines.  #BIG for Big Endian machines.
extern enum Endian getEndianness() ;

extern bool openFileWithBlockIO        ;  ///< `true` if `--block` is set
extern bool openFileWithStreamIO       ;  ///< `true` if `--stream` is set
extern bool openFileWithMapIO          ;  ///< `true` if `--map_file` is set
extern bool readFileContents           ;  ///< `true` if `--read` is set
extern bool forkProcess                ;  ///< `true` if `--fork` is set
extern bool allocateLocalMemory        ;  ///< `true` if `--local` is set
extern bool allocateHeapMemory         ;  ///< `true` if `--malloc` is set
extern bool allocateMappedMemory       ;  ///< `true` if `--map_mem` is set
extern bool fillAllocatedMemory        ;  ///< `true` if `--fill` is set
extern bool scanForByte                ;  ///< `true` if `--scan_byte` is set
extern bool scanForShannon             ;  ///< `true` if `--shannon` is set
extern bool iomemSummary               ;  ///< `true` if `--iomem` is set
extern bool printPath                  ;  ///< `true` if `--path` is set
extern bool includePhysicalPageSummary ;  ///< `true` if `--phys` is set
extern bool includePhysicalPageNumber  ;  ///< `true` if `--pfn` is set

extern char blockPath[ FILENAME_MAX ]   ; ///< The path to the file specified by `--block`
extern char streamPath[ FILENAME_MAX ]  ; ///< The path to the file specified by `--stream`
extern char mapFilePath[ FILENAME_MAX ] ; ///< The path to the file specified by `--map_file`

extern size_t localSize ;   ///< The number of bytes specified by `--local`
extern size_t numLocals ;   ///< The number of recursive local allocations of `--localSize`
extern size_t mallocSize ;  ///< The number of bytes specified by `--malloc`
extern size_t numMallocs ;  ///< The number of `malloc ( mallocSize )` allocations specified by `--numMalloc`
extern size_t mappedSize ;  ///< The number of bytes specified by `--map_mem`
extern void*  mappedStart ; ///< The starting address of `--map_mem` specified by `--map_addr`
extern size_t numThreads ;  ///< The number of threads specified by `--threads`
extern unsigned int sleepSeconds ;  ///< The number of seconds specified by `--sleep`

                                      /// The byte to scan for.
extern unsigned char byteToScanFor ;  ///< The default value is an x86 `RET`
                                      ///< (near) instruction

extern char mapsFilePath[ FILENAME_MAX ]    ; ///< The path to the process-specific `maps` file, defaults to `/proc/self/maps`
extern char pagemapFilePath[ FILENAME_MAX ] ; ///< The path to the process-specific `pagemap` file, defaults to `/proc/self/pagemap`
extern char iomemFilePath[ FILENAME_MAX ]   ; ///< The path to the system-specific `iomem` file, defaults to `/proc/iomem`.
                                              /// No config changes this.  It's parameterized for unit testing.

extern bool  scanSelf ;  ///< `true` if we are scanning ourself.  `false` if we are scanning another process.
extern pid_t scanPid ;   ///< The process ID to monitor.  The default is `-1` to monitor yourself.


/// Reset the entire configuration and free any resources that may have been
/// allocated.
extern void reset_config() ;


/// Validate the configuration.
///
/// Validate enforces major "business rules" of memscan.  It's not looking
/// duplicate `--help` or `--key` configs.
///
/// @param printReason if `true`, then print the reason the validation failed
/// @return `true` if the configuration is healthy.  `false` if it is not.
extern bool validateConfig( const bool printReason ) ;


/// Print `...` to `outStream`.
///
/// @NOLINTBEGIN(cert-err33-c): No need to check the return value of `fprintf`
///
/// @param outStream The output stream is usually `stderr` or `stdout`
#define PRINT( outStream, ... )         \
   fprintf( outStream, __VA_ARGS__ ) ;  \
   /* NOLINTEND(cert-err33-c) */        \
   (void)0


/// Print a warning message to `stderr` (along with the program name).
///
/// @NOLINTBEGIN(cert-err33-c): No need to check the return value of `fprintf`
///
/// @param msg The message to print out.  When printed, it will begin
///            with `progName: `.
#define WARNING( msg, ... ) {     \
   fprintf(                       \
      stderr                      \
     ,"%s: " msg ".\n"            \
     ,getProgramName()            \
     ,##__VA_ARGS__ ) ;  }        \
   /* NOLINTEND(cert-err33-c) */  \
   (void)0


/// Abort the program if `condition` is `false`.
///
/// @param condition An expression of scalar type
#ifdef TESTING
   #define ASSERT( condition )                \
      if( !(condition) ) { throwException() ; }
#else
   #define ASSERT( condition )                \
      assert( condition ) ;
#endif


/// Print an error message to `stderr` (along with the program name) and then
/// exit with a failure status.
///
/// @NOLINTBEGIN(cert-err33-c): No need to check the return value of `fprintf`
///
/// @param msg The message to print out.  When printed, it will begin
///            with `progName: ` and end with `.  Exiting.`.
#ifdef TESTING
   #define FATAL_ERROR( msg, ... )   \
   throwException()
#else
   #define FATAL_ERROR( msg, ... )   \
      fprintf(                       \
         stderr                      \
        ,"%s: " msg ".  Exiting.\n"  \
        ,getProgramName()            \
        ,##__VA_ARGS__ ) ;           \
      exit( EXIT_FAILURE ) ;         \
      /* NOLINTEND(cert-err33-c) */  \
      (void)0
#endif


/// Get the `bitPosition` bit from `value`
///
/// @param value       The source value
/// @param bitPosition The bit number to get.  `0` for the least significant bit.
/// @return Return `1` if the bit is set and `0` if it's not.
///         The return datatype is the same datatype as `value`.
#define GET_BIT( value, bitPosition ) (((value) >> (bitPosition)) & 1)


/// The type of Filter
enum FilterType {
   PATTERN     ///< Search MapEntry.sPath for pattern and include the region if there's a hit
  ,INDEX       ///< Search MapEntry.index and include the region if there's a match
  ,ADDRESS     ///< Search MapEntry and include the region if the virtual address is in the entry
  ,PERMISSION  ///< Search MapEntry and include the region if the permission matches the filter
} ;


/// Linked list of filters to match against when processing map regions
struct Filter {
   enum FilterType type ;     ///< The type of filter
                              ///  Search for this pattern.
   char*           pattern ;  ///< Substring search MapEntry.sPath for `pattern` and include the region if there's a hit.
                              ///< Search patterns `r` `w` and `x` will match on MapEntry.sPermissions.
   size_t          index ;    ///< Search for this MapEntry.index
   void*           address ;  ///< Include if this is between MapEntry.pAddressStart and MapEntry.pAddressEnd
   bool            read ;     ///< Include if the region has the read permission
   bool            write ;    ///< Include if the region has the write permission
   bool            execute ;  ///< Include if the region has the execute permission
   struct Filter*  next ;     ///< The next #Filter in the linked list or `NULL` for the end of the list.
} ;

/// The head pointer to a list of filters
extern struct Filter* filterHead ;


/// Perform a safe, truncated string copy.  Copies at most `count-1` characters
/// to `dest`.
///
/// @param dest  Pointer to the character array to copy to.  Must not be `NULL`.
/// @param src   Pointer to the character array to copy from.  Must not be `NULL`.
/// @param count Size of the `dest` buffer, so at most `count-1` characters
///              will be copied to `dest`.
extern void stringCopy( char* dest, const char* src, size_t count ) ;
