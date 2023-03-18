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
#include <string.h>  // For strtok() strcmp() strstr() memset()

#include "colors.h"  // For ANSI colors i.e. #ANSI_COLOR_RED
#include "config.h"  // For FATAL_ERROR and other configuration options
#include "maps.h"    // Just cuz
#include "pagemap.h" // For PageInfo getPageInfo() printPageInfo() getPageSizeInBits()
#include "shannon.h" // For scanForShannon()
#include "version.h" // For STRINGIFY_VALUE()


/// The `maps` file we intend to read from `/proc`
#define MEMORY_MAP_FILE "/proc/self/maps"

/// The longest allowed length from #MEMORY_MAP_FILE
#define MAX_LINE_LENGTH 1024

/// The maximum number of MapEntry records in #map
/// @todo Consider converting to a linked list
#define MAX_ENTRIES     256

/// Define an array of paths that should be excluded from the scan.
/// On x86 architectures, we should avoid the `[vvar]` path.
///
/// The list ends with an empty value.
///
/// @see https://lwn.net/Articles/615809/
char* ExcludePaths[] = { "[vvar]", "/usr/lib/locale/locale-archive", "" };


/// Holds the original (and some processed data) from each map entry
struct MapEntry {
   char  szLine[MAX_LINE_LENGTH];  ///< String buffer for the entire line
                                   ///  All of the string pointers in this
                                   ///  struct will point to strings in this
                                   ///  buffer after they've been tokenized
   char*  sAddressStart;    ///< String pointer to the start of the address range
   char*  sAddressEnd;      ///< String pointer to the end of the address range
   void*  pAddressStart;    ///< Pointer to the start of the memory mapped region
   void*  pAddressEnd;      ///< Pointer to the byte just *after* the end of
                            ///  the memory mapped region
   char*  sPermissions;     ///< String pointer to the permissions
   char*  sOffset;          ///< String pointer to the offset
   char*  sDevice;          ///< String pointer to the device name
   char*  sInode;           ///< String pointer to the iNode number
   char*  sPath;            ///< String pointer to the path (may be NULL)
   bool   include;          ///< `true` if the entry should be processed.  `false` if it should be excluded.
   size_t numBytes;         ///< Number of bytes in this region
   size_t numPages;         ///< The number of #PageInfo records allocated for this region
   struct PageInfo* pages;  ///< Pointer to a #PageInfo array
   size_t numBytesFound;    ///< Number of #byteToScanFor bytes found in this region when #scanForByte is set
   double shannonEntropy;   ///< Shannon Entropy of the region when #scanForShannon is set
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

      map[numMaps].numBytes = map[numMaps].pAddressEnd - map[numMaps].pAddressStart ;
      map[numMaps].numPages = map[numMaps].numBytes >> getPageSizeInBits() ;

      if( patternHead == NULL ) {  /// If PATTERN is not specified, then include all mapped regions
         map[numMaps].include = true ;
      } else {  /// If PATTERN is specified, then only include regions that include PATTERN
         map[numMaps].include = false ;

         if( map[numMaps].sPath != NULL) {
            struct IncludePattern* current = patternHead ;
            while( current != NULL ) {
               // Compare pattern with 'r' 'w' and 'x' permissions
               if( strlen( current->pattern ) == 1 ) {
                  if( current->pattern[0] == 'r' && map[numMaps].sPermissions[0] == 'r' ) {
                     map[numMaps].include = true ;
                     break ;
                  }
                  if( current->pattern[0] == 'w' && map[numMaps].sPermissions[1] == 'w' ) {
                     map[numMaps].include = true ;
                     break ;
                  }
                  if( current->pattern[0] == 'x' && map[numMaps].sPermissions[2] == 'x' ) {
                     map[numMaps].include = true ;
                     break ;
                  }
                  if(    current->pattern[0] == 'r'
                      || current->pattern[0] == 'w'
                      || current->pattern[0] == 'x' ) {
                     break ;
                  }
               }

               // If not `r`, `w` or `x`, then compare with `pattern`
               if( strstr( map[numMaps].sPath, current->pattern ) != NULL ) {
                  map[numMaps].include = true ;
                  break ;
               }

               current = current->next ;
            } // while( current != NULL )
         }
      }

      // Skip excluded paths
      if( map[numMaps].sPath != NULL ) {
         for( size_t j = 0 ; ExcludePaths[j][0] != '\0' ; j++ ) {
            if( strcmp( map[numMaps].sPath, ExcludePaths[j] ) == 0 ) {
               map[numMaps].include = false ;
            }
         }
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
         printf( "include=[%d]  ",       map[numMaps].include ) ;
         printf( "numBytes=[%zu]  ",     map[numMaps].numBytes ) ;
         printf( "numPages=[%zu]  ",     map[numMaps].numPages ) ;
         printf( "\n" ) ;
      #endif

      numMaps++;
      pRead = fgets( (char *)&map[numMaps].szLine, MAX_LINE_LENGTH, file );
   } // while( pRead != NULL )

   int iRetVal = fclose( file ) ;
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", MEMORY_MAP_FILE ) ;
   }

} // readMaps()


void scanMaps() {
   for( size_t i = 0 ; i < numMaps ; i++ ) {
      if( map[i].sPermissions[0] != 'r' ) {  // Skip non-readable regions
         continue ;
      }

      if( !map[i].include ) {  // Skip excluded paths
         continue ;
      }

      // Do the --scan_byte scan
      if( scanForByte ) {
         for( void* scanThisAddress = map[i].pAddressStart ; scanThisAddress < map[i].pAddressEnd ; scanThisAddress++ ) {
            if( *(unsigned char*)scanThisAddress == byteToScanFor ) {
               map[i].numBytesFound++ ;
            }
         } // for( each address )
      } // if( scanForByte )

      if( scanForShannon ) {
            map[i].shannonEntropy = computeShannonEntropy( map[i].pAddressStart, map[i].numBytes ) ;
      } // if( scanForShannon )

   } // for()
} // scanMaps()


void readPagemapInfo() {
   if( !includePhysicalPageNumber ) {  // Process --pfn
      return ;
   }

   for( size_t i = 0 ; i < numMaps ; i++ ) {
      if( map[i].include && map[i].sPermissions[0] == 'r' ) {
         map[i].pages = malloc( sizeof( struct PageInfo ) * map[i].numPages ) ;
         if( map[i].pages == NULL ) {
            printf( ANSI_COLOR_RED "%s: unable to allocate memory for map entry [%zu]\n" ANSI_COLOR_RESET, getProgramName(), numMaps ) ;
         } else {
            for( size_t j = 0 ; j < map[i].numPages ; j++ ) {
               // printf( "%p\n", map[i].pAddressStart + (j << getPageSizeInBits() ) ) ;
               map[i].pages[j] = getPageInfo( map[i].pAddressStart + (j << getPageSizeInBits() ) ) ;
            } // for( each page )
         } // if( pages was allocated )
      } // if( included and readable )
   } // for( each map )
} // readPagemapInfo


void printMaps() {
   for( size_t i = 0 ; i < numMaps ; i++ ) {
      // If we are filtering on patterns, and it's not included, then skip the line.
      if( patternHead != NULL && !map[i].include ) {
         continue ;
      }

      printf( ANSI_COLOR_CYAN "%2zu: " ANSI_COLOR_RESET, i ) ;
      printf( "%p - %p ", map[i].pAddressStart, map[i].pAddressEnd - 1 ) ;
      printf( ANSI_COLOR_BRIGHT_GREEN "%c" ANSI_COLOR_RESET, map[i].sPermissions[0] ) ;
      if( map[i].sPermissions[1] == 'w' ) {
         printf( ANSI_COLOR_MAGENTA "%c" ANSI_COLOR_RESET, map[i].sPermissions[1] ) ;
      } else {
         printf( "%c", map[i].sPermissions[1] ) ;
      }
      if( map[i].sPermissions[2] == 'x' ) {
         printf( ANSI_COLOR_RED "%c" ANSI_COLOR_RESET, map[i].sPermissions[2] ) ;
      } else {
         printf( "%c", map[i].sPermissions[2] ) ;
      }
      printf( "%c", map[i].sPermissions[3] ) ;
      printf( ANSI_COLOR_CYAN "%'10zu " ANSI_COLOR_RESET, map[i].numBytes ) ;

      if( map[i].sPermissions[0] != 'r' ) {
         printf( ANSI_COLOR_RED "read permission not set on %s" ANSI_COLOR_RESET, map[i].sPath );
         goto finishRegion;
      }

      // Skip excluded paths
      if( !map[i].include ) {
         printf( ANSI_COLOR_RED "%s excluded" ANSI_COLOR_RESET, map[i].sPath );
         goto finishRegion;
      }

      if( scanForByte ) {
         printf( ANSI_COLOR_BRIGHT_YELLOW ) ;
         printf( "w/ %'7zu of ", map[i].numBytesFound ) ;
         printf( "0x%02x ", byteToScanFor ) ;
         printf( ANSI_COLOR_RESET ) ;
      }

      if( scanForShannon ) {
         printf( ANSI_COLOR_BRIGHT_YELLOW ) ;
         printf( "H: %5.3lf ", map[i].shannonEntropy ) ;
         printf( "%-" STRINGIFY_VALUE( MAX_SHANNON_CLASSIFICATION_LENGTH ) "s", getShannonClassification( map[i].shannonEntropy ) ) ;
         printf( ANSI_COLOR_RESET ) ;
      }

      // Print the path
      if( printPath ) {
         printf( ANSI_COLOR_BRIGHT_WHITE "%s" ANSI_COLOR_RESET, map[i].sPath != NULL ? map[i].sPath : "" ) ;
      }

      finishRegion:
      printf( "\n" ) ;

      if( includePhysicalPageNumber ) {
         // Print the physical page information
         if( map[i].include && map[i].sPermissions[0] == 'r' ) {
            for( size_t j = 0 ; j < map[i].numPages ; j++ ) {
               printPageInfo( &map[i].pages[j] ) ;
               /// @todo think about generalizing this to avoid duplicating the .include and read permission checks
            } // for( each page )
         } // if( read permission )
      } // if( includePhysicalMemoryInfo )
   } // for( each map )
} // printMaps


// When reviewing this for the class:
//   - Describe what happens with a fork()
//   - Then parlay that into a discussion of backing memory (file vs. swap)
