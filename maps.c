///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Read, parse and process the `/proc/$PID/maps` file
///
/// @file   maps.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <ctype.h>         // For isgraph()
#include <linux/limits.h>  // For PATH_MAX
#include <stdbool.h>       // For bool, true & false
#include <stdio.h>         // For printf() fprintf() fopen() and FILE
#include <stdlib.h>        // For calloc() malloc() free()
#include <string.h>        // For strtok() strcmp() strstr() memset() strlen() strncpy()

#include "colors.h"  // For ANSI colors i.e. #ANSI_COLOR_RED
#include "config.h"  // For FATAL_ERROR and other configuration options
#include "maps.h"    // Just cuz
#include "pagemap.h" // For PageInfo getPageInfo() printFullPhysicalPage() getPageSizeInBits() printPageSummary()
#include "shannon.h" // For computeShannonEntropy() getShannonClassification() MAX_SHANNON_CLASSIFICATION_LENGTH
#include "version.h" // For STRINGIFY_VALUE()


/// Define an array of paths that should be excluded from the scan.
/// On x86 architectures, we should avoid the `[vvar]` path.
///
/// The list ends with an empty value.
///
/// @see https://lwn.net/Articles/615809/
char* ExcludePaths[] = { "[vvar]"
                        ,"/usr/lib/locale/locale-archive"
                        ,""
                       } ;


/// Is `szStr` empty?
///
/// @param szStr The string to examine
/// @return `true` if `szStr` is empty (for our purposes).  `false` if it isn't.
bool isEmpty( const char* const szStr ) {
   /// Return `true` if:
   /// - `szStr` is `NULL`
   if( szStr == NULL ) {
      return true ;
   }

   /// - `strlen( szStr ) < 1`
   if( strlen( szStr ) < 1 ) {
      return true ;
   }

   /// - The first character of szStr is not printable
   if( !isgraph( szStr[0] ) ) {
      return true ;
   }

   return false ;
}


/// Validate a MapEntry
///
/// @param mapEntry The MapEntry to validate
///
/// @return `true` if the entry is healthy.  `false` if it's not.
bool validateMap( const struct MapEntry* const mapEntry ) {
   /// - MapEntry.szLine must not be `NULL`
   if( mapEntry->szLine == NULL ) {
      return false ;
   }

   if( isEmpty( mapEntry->sAddressStart ) ) { return false; }
   if( isEmpty( mapEntry->sAddressEnd   ) ) { return false; }
   if( isEmpty( mapEntry->sPermissions  ) ) { return false; }
   if( isEmpty( mapEntry->sOffset       ) ) { return false; }
   if( isEmpty( mapEntry->sDevice       ) ) { return false; }
   if( isEmpty( mapEntry->sInode        ) ) { return false; }

   if( mapEntry->sPath != NULL ) {
      if( isEmpty( mapEntry->sPath ) ) { return false; }
   }

   if( mapEntry->pAddressStart >= mapEntry->pAddressEnd ) {
      return false ;
   }

   if( mapEntry->numBytes <= 0 ) {
      return false ;
   }

   if( mapEntry->numBytes != (size_t) ( mapEntry->pAddressEnd - mapEntry->pAddressStart ) ) {
      return false ;
   }

   if( mapEntry->numPages <= 0 ) {
      return false ;
   }

   if( mapEntry->numPages != (size_t) mapEntry->numBytes >> getPageSizeInBits() ) {
      return false ;
   }

   return true ;
}

/// Validate the Maps structure
///
/// @param maps The map structure to validate
///
/// @return `true` if the `maps` linked list is healthy.  `false` if it's not.
bool validateMaps( const struct MapEntry* const maps ) {
   size_t i = 0 ;

   for( const struct MapEntry* mapEntry = maps ; mapEntry != NULL ; mapEntry = mapEntry->next ) {
      /// - MapEntry.index must be a monotonically increasing value from 0
      if( mapEntry->index != i ) {
         return false ;
      }

      if( !validateMap( mapEntry ) ) {
         return false ;
      }

      // Make sure start is increasing
      // Make sure there's no overlap


      i++ ;
   }

   return true ;
}


struct MapEntry* getMaps() {
   struct MapEntry*  mapsHead     = NULL ;       // Head pointer returned to caller
   struct MapEntry** nextMapEntry = &mapsHead ;  // Linked lists typically insert at
                                                 // the beginning of the list, and
                                                 // will hold the list in reverse
                                                 // order.  This variable will always
                                                 // point to the end of the list and
                                                 // makes it easier to create a list in
                                                 // the correct order.

   size_t mapIndex = 0 ;

   FILE* maps_fd = NULL ;  // File handle to #mapsFilePath

   maps_fd = fopen( mapsFilePath, "r" ) ;
   if( maps_fd == NULL ) {
      FATAL_ERROR( "unable to open [%s]", mapsFilePath ) ;
   }

   char* pRead ;
   char szLine[ 73 + PATH_MAX ] ; ///< String buffer for the entire line.  Maps has 73 bytes of data + the path.  @NOLINT(readability-magic-numbers):  A magic number is OK here

   while( true ) {
      // Get the next line from maps_fd
      pRead = fgets( szLine, sizeof( szLine ), maps_fd ) ;
      if( pRead == NULL ) {
         break ;  // We're done when fgets() is done
      }

      #ifdef DEBUG
         printf( "[%s]", szLine ) ;  // A \n is already in .szLine
      #endif

      // Look at the length of the new map entry
      size_t szLineLength = strlen( szLine ) ;
      if( szLineLength == 0 ) {
         continue ;  // Nothing to see here, move on
      }

      /// For each valid row, allocate a new MapEntry and zero it out
      /// @API{ calloc, https://man.archlinux.org/man/calloc.3 }
      struct MapEntry* newMap = calloc( 1, sizeof( struct MapEntry ) ) ;
      if( newMap == NULL ) {
         FATAL_ERROR( "unable to allocate a new MapEntry" ) ;
      }

      newMap->index = mapIndex ;

      // Allocate a new szLine in newMap
      newMap->szLine = malloc( szLineLength + 1 ) ;  // One extra byte for the \0
      if( newMap->szLine == NULL ) {
         FATAL_ERROR( "Unable to allocate maps line" ) ;
      }

      stringCopy( newMap->szLine, szLine, szLineLength+1 ) ;

      // Store data into newMap
      newMap->sAddressStart = strtok( newMap->szLine, "-" ) ;
      newMap->sAddressEnd   = strtok( NULL, " "   ) ;
      newMap->sPermissions  = strtok( NULL, " "   ) ;
      newMap->sOffset       = strtok( NULL, " "   ) ;
      newMap->sDevice       = strtok( NULL, " "   ) ;
      newMap->sInode        = strtok( NULL, " "   ) ;
      newMap->sPath         = strtok( NULL, " \n" ) ;
      /// @todo Add tests to check if anything returns `NULL` or does anything
      ///       out of the ordinary

      // Convert the strings holding the start & end address into pointers
      int retVal1 ;
      int retVal2 ;
      retVal1 = sscanf( newMap->sAddressStart, "%p", &(newMap->pAddressStart) ) ;
      retVal2 = sscanf( newMap->sAddressEnd,   "%p", &(newMap->pAddressEnd  ) ) ;

      if( retVal1 != 1 || retVal2 != 1 ) {
         FATAL_ERROR( "Map entry %zu is unable parse start [%s] or end address [%s]"
         ,newMap->index
         ,newMap->sAddressStart
         ,newMap->sAddressEnd
         ) ;
      }

      newMap->numBytes = newMap->pAddressEnd - newMap->pAddressStart ;
      newMap->numPages = newMap->numBytes >> getPageSizeInBits() ;

      if( filterHead == NULL ) {  /// If FILTER is not specified, then include all mapped regions
         newMap->include = true ;
      } else {  /// If FILTER is specified, then only include regions that match a filter
         newMap->include = false ;

         struct Filter* current = filterHead ;
         while( current != NULL ) {
            if( current->type == INDEX ) {
               if( newMap->index == current->index ) {
                  newMap->include = true ;
               }
               goto Next ;
            }
            if( current->type == ADDRESS ) {
               // printf( "current->address=%p  newMap->pAddressStart=%p  newMap->pAddressEnd=%p\n", current->address, newMap->pAddressStart, newMap->pAddressEnd ) ;
               if( current->address >= newMap->pAddressStart && current->address <= newMap->pAddressEnd ) {
                  newMap->include = true ;
               }
               goto Next ;
            }
            if( current->type == PERMISSION ) {
               if( ( newMap->sPermissions[0] == ((current->read)    ? 'r' : '-' ))
               &&  ( newMap->sPermissions[1] == ((current->write)   ? 'w' : '-' ))
               &&  ( newMap->sPermissions[2] == ((current->execute) ? 'x' : '-' )) ) {
                  newMap->include = true ;
               }
               goto Next ;
            }
            if( current->type == PATTERN ) {
               if( newMap->sPath != NULL) {
                  if( strstr( newMap->sPath, current->pattern ) != NULL ) {
                     newMap->include = true ;
                     goto Next ;
                  }
               }
            }

            Next:
            current = current->next ;
         } // while( current != NULL )
      }

      // Skip excluded paths
      if( newMap->sPath != NULL ) {
         for( size_t j = 0 ; ExcludePaths[j][0] != '\0' ; j++ ) {
            if( strcmp( newMap->sPath, ExcludePaths[j] ) == 0 ) {
               newMap->include = false ;
            }
         }
      }

      #ifdef DEBUG
         printf( "DEBUG:  " ) ;
         printf( "index=[%zu]  ",        newMap->index ) ;
         printf( "sAddressStart=[%s]  ", newMap->sAddressStart ) ;
         printf( "pAddressStart=[%p]  ", newMap->pAddressStart ) ;
         printf( "sAddressEnd=[%s]  ",   newMap->sAddressEnd ) ;
         printf( "pAddressEnd=[%p]  ",   newMap->pAddressEnd ) ;
         printf( "sPermissions=[%s]  ",  newMap->sPermissions ) ;
         printf( "sOffset=[%s]  ",       newMap->sOffset ) ;
         printf( "sDevice=[%s]  ",       newMap->sDevice ) ;
         printf( "sInode=[%s]  ",        newMap->sInode ) ;
         printf( "sPath=[%s]  ",         newMap->sPath ) ;
         printf( "include=[%d]  ",       newMap->include ) ;
         printf( "numBytes=[%zu]  ",     newMap->numBytes ) ;
         printf( "numPages=[%zu]  ",     newMap->numPages ) ;
         printf( "\n" ) ;
      #endif

      // @todo validate the new map entry

      // Insert newMap at the end of the linked list
      *nextMapEntry = newMap ;
      newMap->next = NULL ;
      nextMapEntry = &newMap->next ;

      // Increment mapIndex
      mapIndex++ ;
   } // while( pRead != NULL )

   int iRetVal = fclose( maps_fd ) ;
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", mapsFilePath ) ;
   }

   return mapsHead ;
} // getMaps()


struct MapEntry* getMap( struct MapEntry* const maps, const void* virtualAddress ) {
   struct MapEntry* currentMap = maps ;
   while( currentMap != NULL ) {
      if( virtualAddress >= currentMap->pAddressStart && virtualAddress <= currentMap->pAddressEnd ) {
         break ;
      }
      currentMap = currentMap->next ;
   } // while( currentMap != NULL )

   return currentMap ;
}


void scanMaps( struct MapEntry* const maps ) {  // Process --scan_byte, --shannon
   struct MapEntry* currentMap = maps ;

   while( currentMap != NULL ) {
      if( currentMap->sPermissions[0] != 'r' ) {  // Skip non-readable regions
         goto Next ;
      }

      if( !currentMap->include ) {  // Skip excluded paths
         goto Next ;
      }

      // Do the --scan_byte scan
      if( scanForByte ) {
         for( void* scanThisAddress = currentMap->pAddressStart ; scanThisAddress < currentMap->pAddressEnd ; scanThisAddress++ ) {
            if( *(unsigned char*)scanThisAddress == byteToScanFor ) {
               currentMap->numBytesFound++ ;
            }
         }
      }

      if( scanForShannon ) {
         currentMap->shannonEntropy = computeShannonEntropy( currentMap->pAddressStart, currentMap->numBytes ) ;
      }

      Next:
      currentMap = currentMap->next ;
   } // while( currentMap != NULL )
} // scanMaps


void readPagemapInfo( struct MapEntry* const maps ) {  // Process --pfn --phys
   if( !includePhysicalPageNumber && !includePhysicalPageSummary ) {
      return ;
   }

   struct MapEntry* currentMap = maps ;

   while( currentMap != NULL ) {

      bool okToRead = currentMap->include && currentMap->sPermissions[0] == 'r' ;

      /// @API{ calloc, https://man.archlinux.org/man/calloc.3 }
      currentMap->pages = calloc( currentMap->numPages, sizeof( struct PageInfo ) ) ;
      if( currentMap->pages == NULL ) {
         FATAL_ERROR( "unable to allocate memory for map entry [%zu]", currentMap->index ) ;
      }

      for( size_t j = 0 ; j < currentMap->numPages ; j++ ) {
         // printf( "%p\n", currentMap->pAddressStart + (j << getPageSizeInBits() ) ) ;
         currentMap->pages[j] = getPageInfo( currentMap->pAddressStart + (j << getPageSizeInBits() ), okToRead ) ;
      } // for( each page )

      currentMap = currentMap->next ;
   } // while( currentMap != NULL )
} // readPagemapInfo


void printMaps( const struct MapEntry* const maps ) {
   const struct MapEntry* currentMap = maps ;

   while( currentMap != NULL ) {
      // If we are filtering on patterns, and it's not included, then skip the line.
      if( filterHead != NULL && !currentMap->include ) {
         goto done ;
      }

      printf( ANSI_COLOR_CYAN "%2zu: " ANSI_COLOR_RESET, currentMap->index ) ;
      printf( "%p - %p ", currentMap->pAddressStart, currentMap->pAddressEnd - 1 ) ;
      printf( ANSI_COLOR_CYAN "%'10zu " ANSI_COLOR_RESET, currentMap->numBytes ) ;

      printf( ANSI_COLOR_BRIGHT_GREEN "%c" ANSI_COLOR_RESET, currentMap->sPermissions[0] ) ;
      if( currentMap->sPermissions[1] == 'w' ) {
         printf( ANSI_COLOR_MAGENTA "%c" ANSI_COLOR_RESET, currentMap->sPermissions[1] ) ;
      } else {
         printf( "%c", currentMap->sPermissions[1] ) ;
      }
      if( currentMap->sPermissions[2] == 'x' ) {
         printf( ANSI_COLOR_RED "%c" ANSI_COLOR_RESET, currentMap->sPermissions[2] ) ;
      } else {
         printf( "%c", currentMap->sPermissions[2] ) ;
      }
      printf( "%c ", currentMap->sPermissions[3] ) ;

      if( currentMap->sPermissions[0] != 'r' ) {
         printf( ANSI_COLOR_RED "read permission not set" ANSI_COLOR_RESET ) ;
         goto finishRegion ;
      }

      // Skip excluded paths
      if( !currentMap->include ) {
         printf( ANSI_COLOR_RED "%s excluded" ANSI_COLOR_RESET, currentMap->sPath ) ;
         goto finishRegion ;
      }

      if( scanForByte ) {
         printf( ANSI_COLOR_BRIGHT_YELLOW ) ;
         printf( "w/ %'7zu of ", currentMap->numBytesFound ) ;
         printf( "0x%02x ", byteToScanFor ) ;
         printf( ANSI_COLOR_RESET ) ;
      }

      if( scanForShannon ) {
         printf( ANSI_COLOR_BRIGHT_YELLOW ) ;
         printf( "H: %5.3lf ", currentMap->shannonEntropy ) ;
         printf( "%-" STRINGIFY_VALUE( MAX_SHANNON_CLASSIFICATION_LENGTH ) "s", getShannonClassification( currentMap->shannonEntropy ) ) ;
         printf( ANSI_COLOR_RESET ) ;
      }

      // Print the path
      if( printPath ) {
         printf( ANSI_COLOR_BRIGHT_WHITE "%s" ANSI_COLOR_RESET, currentMap->sPath != NULL ? currentMap->sPath : "" ) ;
      }

      finishRegion:
      printf( "\n" ) ;

      if( includePhysicalPageNumber ) {
         for( size_t j = 0 ; j < currentMap->numPages ; j++ ) {
            printFullPhysicalPage( &currentMap->pages[j] ) ;
         }
      }

      if( includePhysicalPageSummary ) {
         printPageSummary( currentMap->pages, currentMap->numPages ) ;
      }

      done:
      currentMap = currentMap->next ;
   } // while( currentMap != NULL )
} // printMaps


/// @API{ memset, https://man.archlinux.org/man/memset.3 }
/// @API{ free,   https://man.archlinux.org/man/free.3 }
///
/// @param maps A linked list of MapEntry nodes
void releaseMaps( struct MapEntry* const maps ) {
   struct MapEntry* currentMap = maps ;

   while( currentMap != NULL ) {
      free( currentMap->pages ) ;
      free( currentMap->szLine ) ;

      struct MapEntry* oldMap = currentMap ;
      currentMap = currentMap->next ;
      memset( oldMap, 0, sizeof( struct MapEntry ) ) ;
      free( oldMap ) ;
   } // while( currentMap != NULL )
}


// When reviewing this for the class:
//   - Describe what happens with a fork()
//   - Then parlay that into a discussion of backing memory (file vs. swap)
