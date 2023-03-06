///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Read, parse and process the `/proc/$PID/maps` file
///
/// @file   maps.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>   // For printf() fprintf() fopen() and FILE
#include <stdlib.h>  // For EXIT_SUCCESS and EXIT_FAILURE
#include <string.h>  // For strtok() strcmp() and memset()

#include "colors.h"
#include "config.h"
#include "maps.h"
#include "memscan.h"
#include "pagemap.h"


/// The `maps` file we intend to read from `/proc`
#define MEMORY_MAP_FILE "/proc/self/maps"

/// The longest allowed length from #MEMORY_MAP_FILE
#define MAX_LINE_LENGTH 1024

/// The maximum number of MapEntry records in #map
#define MAX_ENTRIES     256

/// The byte to scan for
#define CHAR_TO_SCAN_FOR 'A'

/// Define an array of paths that should be excluded from the scan.
/// On x86 architectures, we should avoid the `[vvar]` path.
///
/// The list ends with an empty value.
///
/// @see https://lwn.net/Articles/615809/
char* ExcludePaths[] = { "[vvar]", "" };


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


/// Holds up to #MAX_ENTRIES of MapEntry structs
struct MapEntry map[MAX_ENTRIES] ;


/// The number of entries in #map
size_t numMaps = 0 ;


void readMaps() {
   memset( map, 0, sizeof( map ));  // Zero out the map data structure

   FILE* file = NULL ;  // File handle to #MEMORY_MAP_FILE

   file = fopen( MEMORY_MAP_FILE, "r" ) ;

   if( file == NULL ) {
      FATAL_ERROR( "Unable to open [%s]", MEMORY_MAP_FILE ) ;
   }

   char* pRead ;

   pRead = fgets( (char *)&map[numMaps].szLine, MAX_LINE_LENGTH, file ) ;

   while( pRead != NULL ) {
      #ifdef DEBUG
         printf( "%s", map[numMaps].szLine ) ;
      #endif

      map[numMaps].sAddressStart = strtok( map[numMaps].szLine, "-" ) ;
      map[numMaps].sAddressEnd   = strtok( NULL, " "   ) ;
      map[numMaps].sPermissions  = strtok( NULL, " "   ) ;
      map[numMaps].sOffset       = strtok( NULL, " "   ) ;
      map[numMaps].sDevice       = strtok( NULL, " "   ) ;
      map[numMaps].sInode        = strtok( NULL, " "   ) ;
      map[numMaps].sPath         = strtok( NULL, " \n" ) ;
      /// @todo Add tests to check if anything returns `NULL` or does anything
      ///       out of the ordinary

      // Convert the strings holding the start & end address into pointers
		int retVal1;
		int retVal2;
      retVal1 = sscanf( map[numMaps].sAddressStart, "%p", &(map[numMaps].pAddressStart) ) ;
      retVal2 = sscanf( map[numMaps].sAddressEnd,   "%p", &(map[numMaps].pAddressEnd  ) ) ;

		if( retVal1 != 1 || retVal2 != 1 ) {
         FATAL_ERROR( "Map entry %zu is unable parse start [%s] or end address [%s]"
               ,numMaps
               ,map[numMaps].sAddressStart
               ,map[numMaps].sAddressEnd
         ) ;
		}

      #ifdef DEBUG
         printf( "DEBUG:  " ) ;
         printf( "numMaps[%zu]  ",       numMaps );
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

   int iRetVal;
   iRetVal = fclose( file ) ;
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", MEMORY_MAP_FILE ) ;
   }

} // readMaps()


void scanMaps() {
   for( size_t i = 0 ; i < numMaps ; i++ ) {
      printf( "%2zu: %p - %p  %s  ",
               i
              ,map[i].pAddressStart
              ,map[i].pAddressEnd - 1
              ,map[i].sPermissions
      ) ;

      int numBytesScanned = 0 ;  // Number of bytes scanned in this region
      int numBytesFound   = 0 ;  // Number of CHAR_TO_SCAN_FOR found in this region

      // Skip non-readable regions
      if( map[i].sPermissions[0] != 'r' ) {
         printf( ANSI_COLOR_RED "read permission not set on %s" ANSI_COLOR_RESET, map[i].sPath );
         goto finishRegion;
      }

      // Skip excluded paths
      if( map[i].sPath != NULL ) {
      	for( size_t j = 0 ; ExcludePaths[j][0] != '\0' ; j++ ) {
         	if( strcmp( map[i].sPath, ExcludePaths[j] ) == 0 ) {
            	printf( ANSI_COLOR_RED "%s excluded" ANSI_COLOR_RESET, map[i].sPath );
            	goto finishRegion;
         	}
      	}
      }

      for( void* scanThisAddress = map[i].pAddressStart ; scanThisAddress < map[i].pAddressEnd ; scanThisAddress++ ) {
         if( *(char*)scanThisAddress == CHAR_TO_SCAN_FOR ) {
            numBytesFound++ ;
         }
         numBytesScanned++ ;
      }

      printf( "Number of bytes read %'10d  Count of 0x%02x is %'7d",
           numBytesScanned
          ,CHAR_TO_SCAN_FOR
          ,numBytesFound) ;

      finishRegion:
      if( printPath ) {
         printf( "  %s", map[i].sPath != NULL ? map[i].sPath : "" );
      }
      printf( "\n" );

      size_t numPages = ( map[i].pAddressEnd - map[i].pAddressStart ) / getPageSizeInBytes();

      for( size_t i = 0 ; i < numPages ; i++ ) {
         doPagemap( map[i].pAddressStart + i * getPageSizeInBytes() );
      }
   } // for()
} // scanMaps()


// When reviewing this for the class:
//   - Describe what happens with a fork()
//   - Then parlay that into a discussion of backing memory (file vs. swap)
