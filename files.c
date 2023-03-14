///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process File I/O pre-scan options: `--block`, `--stream` and `--mmap`
///
/// @file   files.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>     // For open() and O_RDONLY
#include <stdio.h>     // For FILE* fopen() fclose()
#include <sys/mman.h>  // For mmap(), munmap(), PROT_READ, PROT_PRIVATE
#include <sys/stat.h>  // For stat() and struct stat
#include <unistd.h>    // For close()

#include "config.h"    // For FATAL_ERROR
#include "files.h"     // Just cuz

/// File descriptor for block files: `--block`
static int block_fd = -1 ;

/// File descriptor for stream files: `--stream`
static FILE* stream_fd = NULL ;

/// File stat structure for the memory mapped file: `--mmap`
struct stat mmapFileStat ;

/// File descriptor for the memory mapped file: `--mmap`
static int mmap_fd = -1 ;

/// Buffer to the memory mapped file:  `--mmap`
void* mmapBuffer = NULL ;

void openPreScanFiles() {
   if( openFileWithBlockIO ) {
      struct stat fileStat ;
      int rVal = stat( blockPath, &fileStat ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to find --block=[%s]", blockPath ) ;
      }

      block_fd = open( blockPath,  O_RDONLY ) ;
      if( block_fd <= -1 ) {
         FATAL_ERROR( "unable to open --block=[%s]", blockPath ) ;
      }
   }

   if( openFileWithStreamIO ) {
      struct stat fileStat ;
      int rVal = stat( streamPath, &fileStat ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to find --stream=[%s]", streamPath ) ;
      }

      stream_fd = fopen( streamPath, "r" ) ;
      if( stream_fd == NULL ) {
         FATAL_ERROR( "unable to open --stream=[%s]", streamPath ) ;
      }
   }

   if( openFileWithMapIO ) {
      int rVal = stat( mmapPath, &mmapFileStat ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to find --mmap=[%s]", mmapPath ) ;
      }

      mmap_fd = open( mmapPath,  O_RDONLY ) ;
      if( mmap_fd <= -1 ) {
         FATAL_ERROR( "unable to open --mmap=[%s]", mmapPath ) ;
      }

      mmapBuffer = mmap( NULL, mmapFileStat.st_size, PROT_READ, MAP_PRIVATE, mmap_fd, 0 );
      if( mmapBuffer == MAP_FAILED ) {
         FATAL_ERROR( "unable to map file --mmap=[%s]", mmapPath ) ;
      }
   }
} // openPreScanFiles


void readPreScanFiles() {

}


void closePreScanFiles() {
   int iRet ;

   if( openFileWithBlockIO && block_fd >= 0 ) {
      iRet = close( block_fd ) ;
      if( iRet != 0 ) {
         WARNING( "close unexpectedly returned %d for --block=[%s]", iRet, blockPath ) ;
      }
      block_fd = -1 ;
   }

   if( openFileWithStreamIO && stream_fd != NULL ) {
      iRet = fclose( stream_fd ) ;
      if( iRet != 0 ) {
         WARNING( "fclose unexpectedly returned %d for --stream=[%s]", iRet, streamPath ) ;
      }
      stream_fd = NULL ;
   }

   if( openFileWithMapIO && mmap_fd >= 0 ) {
      iRet = munmap( mmapBuffer, mmapFileStat.st_size ) ;
      if( iRet != 0 ) {
         WARNING( "munmap unexpectedly returned %d for --mmap=[%s]\n", iRet, mmapPath ) ;
      }
      iRet = close( mmap_fd ) ;
      if( iRet != 0 ) {
         WARNING( "close unexpectedly returned %d for --mmap=[%s]\n", iRet, mmapPath ) ;
      }
      mmap_fd = -1 ;
      mmapBuffer = NULL ;
   }
} // closePreScanFiles
