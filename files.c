///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process File I/O pre-scan options: `--block`, `--stream`, `--map_file` and
/// `--read`
///
/// @file   files.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>     // For open() and O_RDONLY
#include <stdio.h>     // For FILE* fopen() fclose() rewind()
#include <string.h>    // For strlen()
#include <sys/mman.h>  // For mmap(), munmap(), PROT_READ, PROT_PRIVATE
#include <sys/stat.h>  // For stat() and struct stat
#include <unistd.h>    // For close() lseek()

#include "config.h"    // For FATAL_ERROR
#include "files.h"     // Just cuz
#include "pagemap.h"   // For getPageSizeInBytes()

/// File stat structure for the block file:  `--block`
struct stat blockFileStat ;

/// File descriptor for the block file: `--block`
static int block_fd = -1 ;

/// File stat structure for the stream file: `--stream`
struct stat streamFileStat ;

/// File descriptor for stream files: `--stream`
static FILE* stream_fd = NULL ;

/// File stat structure for the memory mapped file: `--map_file`
struct stat mmapFileStat ;

/// File descriptor for the memory mapped file: `--map_file`
static int mmap_fd = -1 ;

/// Pointer to the memory mapped file buffer:  `--map_file`
void* mmapBuffer = NULL ;


/// @API{ stat, https://man.archlinux.org/man/stat.2 }
/// @API{ open, https://man.archlinux.org/man/open.2 }
/// @API{ fopen, https://man.archlinux.org/man/fopen.3 }
/// @API{ mmap, https://man.archlinux.org/man/mmap.2 }
void openPreScanFiles() {
   if( openFileWithBlockIO ) {
      int rVal = stat( blockPath, &blockFileStat ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to find --block=[%s]", blockPath ) ;
      }

      block_fd = open( blockPath,  O_RDONLY ) ;
      if( block_fd <= -1 ) {
         FATAL_ERROR( "unable to open --block=[%s]", blockPath ) ;
      }
   } // openFileWithBlockIO

   if( openFileWithStreamIO ) {
      int rVal = stat( streamPath, &streamFileStat ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to find --stream=[%s]", streamPath ) ;
      }

      stream_fd = fopen( streamPath, "r" ) ;
      if( stream_fd == NULL ) {
         FATAL_ERROR( "unable to open --stream=[%s]", streamPath ) ;
      }
   } // openFileWithStreamIO

   if( openFileWithMapIO ) {
      int rVal = stat( mapFilePath, &mmapFileStat ) ;
      if( rVal != 0 ) {
         FATAL_ERROR( "unable to find --map_file=[%s]", mapFilePath ) ;
      }

      mmap_fd = open( mapFilePath,  O_RDONLY ) ;
      if( mmap_fd <= -1 ) {
         FATAL_ERROR( "unable to open --map_file=[%s]", mapFilePath ) ;
      }

      mmapBuffer = mmap( NULL, mmapFileStat.st_size, PROT_READ, MAP_PRIVATE, mmap_fd, 0 ) ;
      if( mmapBuffer == MAP_FAILED ) {
         FATAL_ERROR( "unable to map file --map_file=[%s]", mapFilePath ) ;
      }
   } // openFileWithMapIO
} // openPreScanFiles


/// @API{ malloc, https://man.archlinux.org/man/malloc.3 }
/// @API{ read, https://man.archlinux.org/man/read.2 }
/// @API{ fread, https://man.archlinux.org/man/fread.3 }
/// @API{ free, https://man.archlinux.org/man/free.3 }
void readPreScanFiles() {
   /// If not #readFileContents then immediately exit
   if( !readFileContents ) {
      return ;
   }

   /// `--block` and `--stream` files will be read into a buffer that's 4
   /// times getPageSizeInBytes().
   size_t fileBufferSize = getPageSizeInBytes() * 4 ;

   void* fileBuffer = malloc( fileBufferSize ) ;
   if( fileBuffer == NULL ) {
      FATAL_ERROR( "unable to allocate memory for a file buffer" ) ;
   }

   if( openFileWithBlockIO ) {
      ASSERT( block_fd >= 0 ) ;
      ASSERT( strlen( blockPath ) > 0 ) ;

      off_t   numRead = 0 ;  // Number of bytes read
      ssize_t result ;       // The result of the read operation

      do {
         /// Reading with `read()` is not thread safe, but we are not
         /// using this data, so it's OK.  The loop is designed to
         /// terminate when the read gets to the end of the file
         /// and then reset the file pointer.
         result = read( block_fd, fileBuffer, fileBufferSize) ;
         if( result < 0 ) {
            WARNING( "unable to read --buffer[%s]", blockPath ) ;
            continue ;
         }
         numRead += result ;
         // printf( "--block:  numRead=%ld  result=%ld\n", numRead, result ) ;
      } while( result > 0 && numRead < blockFileStat.st_size ) ;
      off_t rVal = lseek( block_fd, 0, SEEK_SET ) ;
      if( rVal < 0 ) {
         WARNING( "lseek failed in --buffer" ) ;
      }
   } // openFileWithBlockIO

   if( openFileWithStreamIO ) {
      ASSERT( stream_fd != NULL ) ;
      ASSERT( strlen( streamPath ) > 0 ) ;

      size_t numRead = 0 ;  // Number of bytes read
      size_t result ;       // The result of the read operation

      // printf( "size of stream file=[%ld]\n", streamFileStat.st_size ) ;
      do {
         /// Reading with `fread()` is not thread safe, but we are not
         /// using this data, so it's OK.  The loop is designed to
         /// terminate when the read gets to the end of the file
         /// and then reset the file pointer.
         result = fread( fileBuffer, 1, fileBufferSize, stream_fd ) ;
         numRead += result ;
         // printf( "Read %ld bytes from stream device\n", result ) ;
      } while( !feof( stream_fd ) && numRead <= ONE_MEGABYTE ) ;  /// `--stream` files will read the first 1M of data
      rewind( stream_fd ) ;
   } // openFileWithStreamIO

   if( openFileWithMapIO ) {
      ASSERT( mmap_fd >= 0 ) ;
      ASSERT( mmapBuffer != NULL ) ;
      ASSERT( mmapFileStat.st_size > 0 ) ;
      ASSERT( strlen( mapFilePath ) > 0 ) ;

      for( off_t i = 0 ; i < mmapFileStat.st_size ; i++ ) {
         unsigned char readByte = *(unsigned char*)(mmapBuffer+i) ;
         (void)readByte ;  // Get away from the unused variable warning
      }
   } // openFileWithStreamIO

   if( fileBuffer != NULL ) {
      free( fileBuffer ) ;
      fileBuffer = NULL ;
   }

} // readPreScanFiles


/// @API{ close, https://man.archlinux.org/man/close.2 }
/// @API{ fclose, https://man.archlinux.org/man/fclose.3 }
/// @API{ munmap, https://man.archlinux.org/man/munmap.2 }
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
         WARNING( "munmap unexpectedly returned %d for --map_file=[%s]\n", iRet, mapFilePath ) ;
      }
      iRet = close( mmap_fd ) ;
      if( iRet != 0 ) {
         WARNING( "close unexpectedly returned %d for --map_file=[%s]\n", iRet, mapFilePath ) ;
      }
      mmap_fd = -1 ;
      mmapBuffer = NULL ;
   }
} // closePreScanFiles
