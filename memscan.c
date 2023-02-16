///////////////////////////////////////////////////////////////////////////////
///         University of Hawaii, College of Engineering
/// @brief  Lab 03a - Memory Scanner - EE 491F - Spr 2022
///////////////////////////////////////////////////////////////////////////////
///
/// @file memscan.c
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   2 Feb 21
///////////////////////////////////////////////////////////////////////////////

#include <locale.h>  // For set_locale() LC_NUMERIC
#include <stdio.h>   // For printf() fopen() and FILE
#include <stdlib.h>  // For EXIT_SUCCESS and EXIT_FAILURE
#include <string.h>  // For strtok() strcmp() and memset()

/// Each row in /proc/$PID/maps describes a region of contiguous
/// virtual memory in a process or thread.  Where $PID is 'self'
/// the file is the memory map for the current process.

/// For example:
///
/// address                  perms offset   dev   inode      pathname
/// 00403000-00404000         r--p 00002000 00:30 641        /mnt/src/Src/tmp/t
/// 00404000-00405000         rw-p 00003000 00:30 641        /mnt/src/Src/tmp/t
/// 00405000-00587000         rw-p 00000000 00:00 0
/// 00cc3000-00ce4000         rw-p 00000000 00:00 0          [heap]
/// 7f8670628000-7f867062a000 rw-p 00000000 00:00 0
/// 7f867062a000-7f8670650000 r--p 00000000 00:1f 172006     /usr/lib64/libc-2.32.so
/// 7f8670650000-7f867079f000 r-xp 00026000 00:1f 172006     /usr/lib64/libc-2.32.so
///
/// Each row has the following fields:
///   - address:      The starting and ending address of the region in the
///                   process's address space
///   - permissions:  This describes how pages in the region can be accessed.
///                   There are four different permissions: read, write,
///                   execute, and shared. If read/write/execute are disabled,
///                   a - will appear instead of the r/w/x. If a region is not
///                   shared, it is private, so a p will appear instead of an s.
///                   If the process attempts to access memory in a way that is
///                   not permitted, a segmentation fault is generated.
///                   Permissions can be changed using the mprotect system call.
///   - offset:       If the region was mapped from a file (using mmap), this
///                   is the offset in the file where the mapping begins.
///                   If the memory was not mapped from a file, it's just 0.
///   - device:       If the region was mapped from a file, this is the major
///                   and minor device number (in hex) where the file lives.
///   - inode:        If the region was mapped from a file, this is the file number.
///   - pathname:     If the region was mapped from a file, this is the name
///                   of the file. This field is blank for anonymous mapped
///                   regions. There are also special regions with names like
///                   [heap], [stack], or [vdso]. [vdso] stands for virtual
///                   dynamic shared object. It's used by system calls to
///                   switch to kernel mode.

///
/// Notes:
///   - maps reports addresses like this:  [ 00403000-00404000 )...
///     the "end address" is just outside the valid range.  When memscan prints
///     the range, it shows inclusive addresses like this: [ 00403000-00403fff ]

/// The maps file we intend to read from /proc
#define MEMORY_MAP_FILE "/proc/self/maps"

/// The longest allowed length we will process from MEMORY_MAP_FILE
#define MAX_LINE_LENGTH 1024

/// The maximum number of MapEntry records in map
#define MAX_ENTRIES     256

/// The byte to scan for
#define CHAR_TO_SCAN_FOR 'A'


/// Holds the original (and some processed data) from each map entry
struct MapEntry {
   char  szLine[MAX_LINE_LENGTH];  ///< String buffer for the entire line
                                   ///  All of the string pointers in this
                                   ///  struct will point to strings in this
                                   ///  buffer after they've been tokenized
   char* sAddressStart;  ///< String pointer to the start of the address range
   char* sAddressEnd;    ///< String pointer to the end of the address range
   void* pAddressStart;  ///< Pointer to the start of the memory mapped region
   void* pAddressEnd;    ///< Pointer to the byte just *after* the end of
                         ///  the memory mapped region
   char* sPermissions;   ///< String pointer to the permissions
   char* sOffset;        ///< String pointer to the offset
   char* sDevice;        ///< String pointer to the device name
   char* sInode;         ///< String pointer to the iNode number
   char* sPath;          ///< String pointer to the path (may be NULL)
} ;


/// Holds up to MAX_ENTRIES MapEntry structs
struct MapEntry map[MAX_ENTRIES] ;


/// The number of entries in map
size_t numMaps = 0 ;


/// Parse each line from MEMORY_MAP_FILE, mapping the data into
/// a MapEntry field.  This function makes heavy use of strtok().
///
void readEntries( FILE* file ) {
   char* pRead ;

   pRead = fgets( (char *)&map[numMaps].szLine, MAX_LINE_LENGTH, file ) ;

   while( pRead != NULL ) {
      map[numMaps].sAddressStart = strtok( map[numMaps].szLine, "-" ) ;
      map[numMaps].sAddressEnd   = strtok( NULL, " "   ) ;
      map[numMaps].sPermissions  = strtok( NULL, " "   ) ;
      map[numMaps].sOffset       = strtok( NULL, " "   ) ;
      map[numMaps].sDevice       = strtok( NULL, " "   ) ;
      map[numMaps].sInode        = strtok( NULL, " "   ) ;
      map[numMaps].sPath         = strtok( NULL, " \n" ) ;
      /// @todo Add tests to check if anything returns NULL or does anything
      ///       out of the ordinary

      // Convert the strings holding the start & end address into pointers
      sscanf( map[numMaps].sAddressStart, "%p", &(map[numMaps].pAddressStart) ) ;
      sscanf( map[numMaps].sAddressEnd,   "%p", &(map[numMaps].pAddressEnd  ) ) ;

      #ifdef DEBUG
         printf( "DEBUG:  " ) ;
         printf( "numMaps[%lu]  ",       numMaps );
         printf( "sAddressStart=[%s]  ", map[numMaps].sAddressStart ) ;
         printf( "pAddressStart=[%p]  ", map[numMaps].pAddressStart ) ;
         printf( "sAddressEnd=[%s]  ",   map[numMaps].sAddressEnd ) ;
         printf( "pAddressEnd=[%p]  ",   map[numMaps].pAddressEnd ) ;
         printf( "sPermissions=[%s]  ",  map[numMaps].sPermissions ) ;
         printf( "sOffset=[%s]  ",       map[numMaps].sOffset ) ;
         printf( "sDevice=[%s]  ",       map[numMaps].sDevice ) ;
         printf( "sInode=[%s]  ",        map[numMaps].sInode ) ;
         printf( "sPath=[%s]  ",         map[numMaps].sPath ) ;
         printf( "\n" ) ;
      #endif

      numMaps++;
      pRead = fgets( (char *)&map[numMaps].szLine, MAX_LINE_LENGTH, file );
   } // while()
} // readEntries()


/// This is the workhorse of this program... Scan all readable memory
/// regions, counting the number of bytes scanned and the number of
/// times CHAR_TO_SCAN_FOR appears in the region...
void scanEntries() {
   for( size_t i = 0 ; i < numMaps ; i++ ) {
      int numBytesScanned = 0 ;  ///< Number of bytes scanned in this region
      int numBytesFound   = 0 ;  ///< Number of CHAR_TO_SCAN_FOR found in this region

      // Skip non-readable regions
      if( map[i].sPermissions[0] != 'r' ) {
         continue ;
      }

      // Skip [vvar]
      if( map[i].sPath != NULL ) {
         if( strcmp( map[i].sPath, "[vvar]" ) == 0 ) {
            printf( "%2ld: %s skipped\n", i, map[i].sPath );
            continue ;
         }
      }

      for( void* scanThisAddress = map[i].pAddressStart ; scanThisAddress < map[i].pAddressEnd ; scanThisAddress++ ) {
         if( *(char*)scanThisAddress == CHAR_TO_SCAN_FOR ) {
            numBytesFound++ ;
         }
         numBytesScanned++ ;
      }

      printf( "%2ld: %p - %p  %s  Number of bytes read %'10d  Count of 0x%02x is %'7d\n",
           i
          ,map[i].pAddressStart
          ,map[i].pAddressEnd - 1
          ,map[i].sPermissions
          ,numBytesScanned
          ,CHAR_TO_SCAN_FOR
          ,numBytesFound) ;
   } // for()
} // scanEntries()


/// Memory scanner
int main( int argc __attribute__((unused)), char* argv[] ) {
   printf( "Memory scanner\n" ) ;
   
   setlocale( LC_NUMERIC, "" ) ;

   /// File handle to MEMORY_MAP_FILE
   FILE* file = NULL ;

   file = fopen( MEMORY_MAP_FILE, "r" ) ;

   if( file == NULL ) {
      printf( "%s: Unable to open [%s].  Exiting.\n", argv[0], MEMORY_MAP_FILE ) ;
      return EXIT_FAILURE ;
   }

   memset( map, 0, sizeof( map ));  // Zero out the map data structure

   readEntries( file ) ;

   scanEntries() ;

   fclose( file ) ;

   return EXIT_SUCCESS ;
}
