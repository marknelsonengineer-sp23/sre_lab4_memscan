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
extern bool forkProcess               ;  ///< `true` if `--fork` is set
extern bool allocateLocalMemory       ;  ///< `true` if `--local` is set
extern bool allocateMallocMemory      ;  ///< `true` if `--malloc` is set
extern bool allocateSharedMemory      ;  ///< `true` if `--shared` is set
extern bool fillAllocatedMemory       ;  ///< `true` if `--fill` is set
extern bool createThreads             ;  ///< `true` if `--threads` is set
extern bool scanForByte               ;  ///< `true` if `--scan_byte` is set
extern bool scanForHistogram          ;  ///< `true` if `--histogram` is set
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

extern unsigned char byteToScanFor ;  ///< The byte to scan for



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
