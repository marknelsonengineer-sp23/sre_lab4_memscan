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

#include <assert.h>   // For assert()
#include <limits.h>   // For PATH_MAX
#include <stdbool.h>  // For bool
#include <stdio.h>    // For FILE stderr fprintf()
#include <stdlib.h>   // For exit()


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


#define MAX_PROGRAM_NAME  32  ///< The maximum size of the program name

#define ONE_MEGABYTE 1048576  ///< One megabyte of something

/// Set the program name
///
/// @param newProgramName The new program name
/// @return `true` if successful.  `false` if unsuccessful.
extern bool setProgramName( const char* newProgramName ) ;


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

extern bool openFileWithBlockIO       ;  ///< `true` if `--block` is set
extern bool openFileWithStreamIO      ;  ///< `true` if `--stream` is set
extern bool openFileWithMapIO         ;  ///< `true` if `--mmap` is set
extern bool readFileContents          ;  ///< `true` if `--read` is set
extern bool forkProcess               ;  ///< `true` if `--fork` is set
extern bool allocateLocalMemory       ;  ///< `true` if `--local` is set
extern bool allocateHeapMemory        ;  ///< `true` if `--malloc` is set
extern bool allocateSharedMemory      ;  ///< `true` if `--shared` is set
extern bool fillAllocatedMemory       ;  ///< `true` if `--fill` is set
extern bool createThreads             ;  ///< `true` if `--threads` is set
extern bool scanForByte               ;  ///< `true` if `--scan_byte` is set
extern bool scanForShannon            ;  ///< `true` if `--shannon` is set
extern bool iomemSummary              ;  ///< `true` if `--iomem` is set
extern bool printPath                 ;  ///< `true` if `--path` is set
extern bool includePhysicalMemoryInfo ;  ///< `true` if `--phys` is set

extern char blockPath[ FILENAME_MAX ] ;    ///< The path to the file specified by `--block`
extern char streamPath [ FILENAME_MAX ] ;  ///< The path to the file specified by `--stream`
extern char mmapPath [ FILENAME_MAX ] ;    ///< the path to the file specified by `--mmap`

extern size_t localSize ;   ///< The number of bytes specified by `--local`
extern size_t mallocSize ;  ///< The number of bytes specified by `--malloc`
extern size_t sharedSize ;  ///< The number of bytes specified by `--shared`
extern size_t numThreads ;  ///< The number of threads specified by `--threads`

                                      /// The byte to scan for.
extern unsigned char byteToScanFor ;  ///< The default value is an x86 `RET`
                                      ///< (near) instruction


/// Print `...` to `outStream`.
///
/// @NOLINTBEGIN(cert-err33-c): No need to check the return value of `fprintf`
///
/// @param outStream The output stream is usually `stderr` or `stdout`
#define PRINT( outStream, ... )         \
   fprintf( outStream, __VA_ARGS__ ) ;  \
   /* NOLINTEND(cert-err33-c) */        \
   (void)0


#define ASSERT( condition ) \
   assert( condition ) ;


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
     ,##__VA_ARGS__ ) ;           \
   exit( EXIT_FAILURE ); }        \
   /* NOLINTEND(cert-err33-c) */  \
   (void)0


/// Get the `bitPosition` bit from `value`
///
/// @param value       The source value
/// @param bitPosition The bit number to get.  `0` for the least significant bit.
/// @return Return `1` if the bit is set and `0` if it's not.
///         The return datatype is the same datatype as `value`.
#define GET_BIT( value, bitPosition ) (((value) >> (bitPosition)) & 1)


/// Linked list of patterns to match against when processing map regions
struct IncludePattern {
                                     ///  Search for this pattern.
   char*                  pattern ;  ///< Search MapEntry.sPath for pattern and include the region if there's a hit.
                                     ///< Search patterns `r` `w` and `x` will match on MapEntry.sPermissions.
   struct IncludePattern* next ;     ///< The next #IncludePattern in the linked list or `NULL` for the end of the list.
} ;

/// The head pointer to a list of region patterns
extern struct IncludePattern* patternHead ;
