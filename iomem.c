///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process `/proc/iomem` to characterize physical pages
///
/// `/proc/iomem` contains the map of the system's physical memory.  Computers
/// use physical memory addresses for things other than general purpose memory
/// (the sort used by programs and data).  Memory-mapped IO maps certain
/// physical addresses directly to hardware devices like network and video cards.
///
/// Addressing details are negotiated and assigned dynamically by the cards and
/// the host computer as the system starts up.  The `/proc/iomem` contains the
/// negotiated memory map.
///
/// The first column displays the memory addresses of physical memory regions.
/// The second column has a description of the region.
/// ````
///    00000000-00000fff : Reserved
///    00001000-0009ffff : System RAM
///    80000000-fdffffff : PCI Bus 0000:00
///      80000000-81ffffff : 0000:00:02.0
///        80000000-81ffffff : vmwgfx probe
///      82000000-823fffff : 0000:00:04.0
///        82000000-823fffff : vboxguest
/// ````
///
/// The addresses are not virtual – they are physical (or bus) addresses.  The
/// datatype for a physical addresses in memscan is #pfn_t.
///
/// Combining the descriptions and sorting by size, we can summarize the total
/// amount of memory for each region:
/// ````
/// Summary of /proc/iomem
/// ahci                                         8,192
/// Video ROM                                   32,768
/// ACPI Tables                                 65,536
/// System ROM                                  65,536
/// e1000                                      131,072
/// Kernel data                              2,184,448
/// Kernel code                             16,784,908
/// Reserved                                20,357,120
/// System RAM                           2,096,520,436
/// Unmapped memory                281,470,711,262,208
/// ````
///
/// @file   iomem.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>   // For bool, true & false
#include <stdio.h>     // For printf() fopen() fgets() sscanf() fclose() and FILE
#include <stdlib.h>    // For malloc() free()
#include <string.h>    // For strncpy() memset() strlen() strtok() strncmp()

#include "config.h"    // For FATAL_ERROR() ASSERT() iomemFilePath
#include "iomem.h"     // Just cuz
#include "trim.h"      // For trim_edges()
#include "typedefs.h"  // For pfn_t const_pfn_t PFN_MASK PFN_FORMAT
#include "version.h"   // For STRINGIFY_VALUE()


/// The longest allowed line length from #iomemFilePath
#define MAX_LINE_LENGTH 1024

/// Typedef of #Iomem_region
typedef struct Iomem_region Iomem_region_t ;

/// Hold each `iomem` region as elements in a linked list
struct Iomem_region {
   /// The starting physical address of this memory region.
   pfn_t start ;

   /// Pointer to the next region.
   /// The end address of this region is `next->start - 1` unless
   /// `next == NULL`, then it's #MAX_PHYS_ADDR
   Iomem_region_t* next ;

   /// The name/description of this memory region
   char description[ MAX_IOMEM_DESCRIPTION ] ;
} ;


/// The head pointer to the #Iomem_region linked list
///
/// Unlike many linked lists... this is __always__ set
Iomem_region_t iomem_head = { 0, NULL, UNMAPPED_MEMORY_DESCRIPTION } ;


/// Typedef of #Iomem_summary
typedef struct Iomem_summary Iomem_summary_t ;

/// A summary (sum of region sizes) of #Iomem_region records in a linked list
struct Iomem_summary {
   /// The total number of bytes for all of the regions with this
   /// Iomem_region.description.
   size_t size ;

   /// For use by validate_summary().  It is set to size and then decremented
   /// to 0 for validation.
   size_t countback_bucket ;

   /// Pointer to the next summary.
   Iomem_summary_t* next ;

   /// The name/description of this memory region
   char description[ MAX_IOMEM_DESCRIPTION ] ;
} ;


/// The head pointer to the #Iomem_summary linked list
Iomem_summary_t* iomem_summary_head = NULL ;


/// Compute the end address of a region
///
/// @param region The region to process.  Can't be `NULL`.
/// @return The ending physical address which is either:
///         1) The byte before the next region's starting address
///         2) or #MAX_PHYS_ADDR
pfn_t getEnd( const Iomem_region_t* region ) {
   ASSERT( region != NULL ) ;
   return ( region->next == NULL ) ? ((pfn_t)MAX_PHYS_ADDR) : ( region->next->start - 1 ) ;
}


/// Release resources used by iomem summary
void release_iomem_summary() {
   /// Free the summary list
   Iomem_summary_t* currentSummary = iomem_summary_head ;

   while( currentSummary != NULL ) {
      Iomem_summary_t* oldSummary = currentSummary ;
      currentSummary = currentSummary->next ;
      memset( oldSummary, 0, sizeof( Iomem_summary_t ) ) ;
      free( oldSummary ) ;
   }
   iomem_summary_head = NULL ;
}

void release_iomem() {
   /// @API{ memset, https://man.archlinux.org/man/memset.3 }
   /// @API{ free, https://man.archlinux.org/man/free.3 }

   Iomem_region_t* currentRegion = iomem_head.next ;
   while( currentRegion != NULL ) {
      Iomem_region_t* oldRegion = currentRegion ;
      currentRegion = currentRegion->next ;
      memset( oldRegion, 0, sizeof( Iomem_region_t ) ) ;
      free( oldRegion ) ;
   }

   /// Set the head pointer to its initial values
   iomem_head.start = 0 ;
   iomem_head.next = NULL ;
   stringCopy( iomem_head.description, UNMAPPED_MEMORY_DESCRIPTION, MAX_IOMEM_DESCRIPTION ) ;

   release_iomem_summary() ;
}


/// Validate the #iomem_head linked list
///
/// The validation rules for #iomem_head are:
///   - The list can not be empty
///   - The first start address must be 0
///   - Every region must monotonically increase
///   - No duplicate regions
///
/// @return `true` if the structure is valid.  `false` if it's not.
bool validate_iomem() {
   if( iomem_head.start != 0 ) {
      return false ;
   }

   pfn_t monotonic_address = 0 ;

   Iomem_region_t* current = &iomem_head ;

   while( current != NULL ) {
      if( monotonic_address != 0 && current->start <= monotonic_address ) {
         return false ;
      }

      monotonic_address = current->start ;
      current = current->next ;
   }

   return true ;
}


/// Print all of the `iomem` regions.
///
/// @internal This is intended for debugging.  It's not output by any
///           of memscan's options.
///
/// Assumes a 48-bit physical address bus size (6 bytes or 12 characters wide).
void print_iomem_regions() {
   Iomem_region_t* current = &iomem_head ;

   while( current != NULL ) {
      pfn_t end = getEnd( current ) ;

      printf(  "%012" PFN_FORMAT " - ",  current->start ) ;
      printf(  "%012" PFN_FORMAT "  ",   end ) ;
      printf( "(%012" PFN_FORMAT ")  ",  end - current->start + 1 ) ;
      printf( "%s\n", current->description ) ;

      current = current->next ;
   }
}


const char* get_iomem_region_description( const_pfn_t physAddr ) {
   Iomem_region_t* region = &iomem_head ;

   while( region->next != NULL && region->next->start <= physAddr ) {
      region = region->next ;
   }

   return region->description ;
}


/// Add a new `iomem` region
///
/// @param start The starting physical address of the new region (inclusive).
///              It may overlap with an existing region.
/// @param end   The ending physical address of the new region.  It must be
///              within an existing region.  `end` must be `>= start`.
/// @param description The description of a region (up to #MAX_IOMEM_DESCRIPTION
///                    bytes).  The description may not be `NULL` or empty.
///                    It is assumed that `description` is already trimmed.
void add_iomem_region( const_pfn_t start, const_pfn_t end, const char* description ) {
   ASSERT( validate_iomem() ) ;
   ASSERT( end >= start ) ;
   ASSERT( description != NULL ) ;
   ASSERT( strlen( description ) != 0 ) ;

   // Find the start...
   Iomem_region_t* current = &iomem_head ;

   pfn_t current_end = getEnd( current ) ;

   while( current->next != NULL && current->start < start && current_end < end ) {
      current = current->next ;
      current_end = getEnd( current ) ;
   }

   // current should point to the region we need to insert into

   // printf( "addStart=%" PFN_FORMAT " addEnd=%" PFN_FORMAT " addDesc=[%s] curStart=%" PFN_FORMAT " curEnd=%" PFN_FORMAT ", curDesc=[%s]\n", start, end, description, current->start, current_end, current->description ) ;

   /// The following definitions will help decode how this is implemented:
   ///   - `cur` Current region
   ///   - `add` The region we want to add
   ///   - `old` The original region record
   ///   - `new` The new region record (added to the right of `old` in the linked
   ///           list)
   ///
   /// @API{ strncpy, https://man.archlinux.org/man/strncpy.3 }
   /// @API{ malloc, https://man.archlinux.org/man/malloc.3 }
   /// There are several scenarios for inserting regions...
   ///
   if( current->start == start && current_end == end ) {
      /// - If `addStart == currStart && addEnd == curEnd`, then just replace
      ///   the description:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, curEnd, addDesc>
      ///   ````
      // printf( "Replace region\n" ) ;
      stringCopy( current->description, description, MAX_IOMEM_DESCRIPTION ) ;
   } else if( current->start == start && current_end > end ) {
      /// - If the `addStart == curStart` and `addEnd < curEnd`, then insert a
      ///   new region in the linked list:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, addEnd, addDesc> -> <addEnd+1, curEnd, curDesc>
      ///   ````
      // printf( "Create new-left\n" ) ;

      Iomem_region_t* newRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( newRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }

      stringCopy( newRegion->description, current->description, MAX_IOMEM_DESCRIPTION ) ;
      newRegion->start = end+1 ;
      newRegion->next = current->next ;
      current->next = newRegion ;
      current->start = start ;
      stringCopy( current->description, description, MAX_IOMEM_DESCRIPTION ) ;
   } else if( current->start < start && current_end == end ) {
      /// - If the `addStart > curStart` and `addEnd == curEnd`, then insert a
      ///   new region in the linked list:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, addStart-1, curDesc> -> <addStart, addEnd, addDesc>
      ///   ````
      // printf( "Create new-right\n" ) ;
      Iomem_region_t* newRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( newRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }

      stringCopy( newRegion->description, description, MAX_IOMEM_DESCRIPTION ) ;
      newRegion->start = start ;
      newRegion->next = current->next ;
      current->next = newRegion ;
   } else if( current->start < start && current_end > end ) {
      /// - If the `addStart > curStart` and `addEnd < curEnd`, then insert two
      ///   new regions in the linked list:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, addStart-1, curDesc> -> <addStart, addEnd, addDesc> -> <addEnd+1, curEnd, curDesc>
      ///   ````
      // printf( "Create new-middle\n" ) ;
      Iomem_region_t* newRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( newRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }
      stringCopy( newRegion->description, current->description, MAX_IOMEM_DESCRIPTION ) ;
      newRegion->start = end+1 ;
      newRegion->next = current->next ;

      Iomem_region_t* middleRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( middleRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }

      stringCopy( middleRegion->description, description, MAX_IOMEM_DESCRIPTION ) ;
      middleRegion->start = start ;
      middleRegion->next = newRegion ;
      current->next = middleRegion ;
   } else {
      /// - Anything else should return an error (it's an overlapping region)
      #ifndef TESTING
         print_iomem_regions() ;
      #endif
      FATAL_ERROR( "requested region overlaps and is not valid" ) ;
   }

   ASSERT( validate_iomem() ) ;
} // add_iomem_region


void read_iomem() {
   release_iomem() ;

   FILE* file = NULL ;  // File handle to #iomemFilePath

   file = fopen( iomemFilePath, "r" ) ;  /// @API{ fopen, https://man.archlinux.org/man/fopen.3 }
   if( file == NULL ) {
      FATAL_ERROR( "Unable to open [%s]", iomemFilePath ) ;
   }

   char* pRead ;
   char szLine[ MAX_LINE_LENGTH ] ;

   pRead = fgets( szLine, MAX_LINE_LENGTH, file ) ;  /// @API{ fgets, https://man.archlinux.org/man/fgets.3 }
   trim_edges( szLine ) ;

   while( pRead != NULL ) {
      #ifdef DEBUG
         printf( "%s\n", szLine ) ;
      #endif

      /// @API{ strtok, https://man.archlinux.org/man/strtok.3 }
      char* sAddressStart = strtok( szLine, "-" ) ;
      char* sAddressEnd   = strtok( NULL, " "   ) ;
      char* sDescription  = strtok( NULL, "\n" ) + 2 ;  // The +2 skips over a ": "

      // Convert the strings holding the start & end address into pointers
      char* pEndOfpAddressStart ;  // Used for error detection of strtoul
      char* pEndOfpAddressEnd ;
      pfn_t pAddressStart ;
      pfn_t pAddressEnd ;

      pAddressStart = strtoul( sAddressStart, &pEndOfpAddressStart, BASE_16 ) ;
      pAddressEnd   = strtoul( sAddressEnd,   &pEndOfpAddressEnd,   BASE_16 ) ;

      if( *pEndOfpAddressStart != '\0' || *pEndOfpAddressEnd != '\0' ) {
         FATAL_ERROR( "iomem entry [%s] is unable parse start [%s] or end address [%s]"
         ,sDescription
         ,sAddressStart
         ,sAddressEnd
         ) ;
      }

      #ifdef DEBUG
         printf( "DEBUG:  " ) ;
         printf( "sAddressStart=[%s]  "             , sAddressStart ) ;
         printf( "pAddressStart=[%" PFN_FORMAT "]  ", pAddressStart ) ;
         printf( "sAddressEnd=[%s]  "               , sAddressEnd ) ;
         printf( "pAddressEnd=[%" PFN_FORMAT "]  "  , pAddressEnd ) ;
         printf( "sDescription=[%s]  "              , sDescription ) ;
         printf( "\n" ) ;
      #endif

      add_iomem_region( pAddressStart, pAddressEnd, sDescription ) ;

      pRead = fgets( szLine, MAX_LINE_LENGTH, file ) ;
      trim_edges( szLine ) ;
   } // while()


   int iRetVal = fclose( file ) ;  /// @API{ fclose, https://man.archlinux.org/man/fclose.3 }
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", iomemFilePath ) ;
   }
} // read_iomem


/// Print the linked list of #Iomem_summary records under #iomem_summary_head
void print_iomem_summary() {
   printf( "Summary of %s\n", iomemFilePath ) ;

   Iomem_summary_t* type = iomem_summary_head ;

   size_t totalMappedMemory = 0 ;

   while( type != NULL ) {
      if( type->next != NULL ) {  // If it's not the last record
         totalMappedMemory += type->size ;
      } else {
         printf( "---------------------------------------------------------------------------------------\n" ) ;
         printf( "%-" STRINGIFY_VALUE( MAX_IOMEM_DESCRIPTION ) "s ", "Total Mapped Memory" ) ;
         printf( "%'22zu", totalMappedMemory ) ;
         printf( "\n" ) ;
      }

      printf( "%-" STRINGIFY_VALUE( MAX_IOMEM_DESCRIPTION ) "s ", type->description ) ;
      printf( "%'22zu", type->size ) ;
      printf( "\n" ) ;

      type = type->next ;
   }
}


/// Sort the linked list of #Iomem_summary records under #iomem_summary_head
///
/// @internal Implements an efficient bubble sort of a linked list.  Because
///           we will always maintain the list, this sort will be fairly
///           efficient as we'll always be increasing/slotting just one
///           value.
void sort_iomem_summary() {
   bool swapped ;
   do {
      swapped = false ;

      Iomem_summary_t* type = iomem_summary_head ;
      Iomem_summary_t* prev = NULL ;

      while( type != NULL && type->next != NULL ) {  // While there are at least 2 types...
         // printf( "head=[%p]  prev=[%p]  type=[%p, %zu]  type->next=[%p, %zu]  type->next->next=[%p]\n", iomem_type_head, prev, type, type->size, type->next, type->next->size, type->next->next ) ;
         if( type->size > type->next->size ) {  // Swap the two types...
            swapped = true ;
            if( prev == NULL ) {  // Swap involves the head...
               // printf( "Swap head\n" ) ;
               Iomem_summary_t* temp = iomem_summary_head->next->next ;
               iomem_summary_head = type->next ;
               iomem_summary_head->next = type ;
               type->next = temp ;
               type = iomem_summary_head ;
            } else {
               // printf( "Swap interior" ) ;
               Iomem_summary_t* temp = type->next->next ;
               prev->next = type->next ;
               prev->next->next = type ;
               type->next = temp ;
               type = prev->next ;
            }
         }
         prev = type ;
         type = type->next ;
      }
   } while( swapped ) ;  // If we swapped two rows, then keep sorting
} // sort_iomem


/// Iterate over the #Iomem_region linked list from #iomem_head and either
/// find an existing #Iomem_summary (based on matching the description) and add
/// to it or create a new one.
///
/// @API{ strncmp, https://man.archlinux.org/man/strncmp.3 }
/// @API{ malloc, https://man.archlinux.org/man/malloc.3 }
/// @API{ strncpy, https://man.archlinux.org/man/strncpy.3 }
void compose_iomem_summary() {
   release_iomem_summary() ;

   Iomem_region_t* region = &iomem_head ;

   while( region != NULL ) {
      Iomem_summary_t* type = iomem_summary_head ;
      while( type != NULL ) {
         if( strncmp( region->description, type->description, MAX_IOMEM_DESCRIPTION ) == 0 ) {
            type->size += getEnd( region ) - region->start + 1 ;
            break ;
         }
         type = type->next ;
      }

      if( type == NULL ) {  // If we got to the end of the list without a hit, then add an entry
         Iomem_summary_t* newType = malloc( sizeof( Iomem_summary_t )) ;
         strncpy( newType->description, region->description, MAX_IOMEM_DESCRIPTION ) ;
         newType->size = getEnd( region ) - region->start + 1 ;
         newType->next = iomem_summary_head ;  // Add newType to the front of the list
         iomem_summary_head = newType ;
      }

      region = region->next ;
   }
}


/// Validate the #iomem_summary_head list.
///
/// Ensure that compose_iomem_summary() and sort_iomem_summary() have run first.
///
/// @return `true` if the iomem_summary_head list is healthy.  `false` if it's not.
bool validate_summary() {
   ASSERT( validate_iomem() ) ;
   ASSERT( iomem_summary_head != NULL ) ;

   size_t monotonic_size = 0 ;

   /// Iterate over the summary list and set `countback_bucket` to the summary's
   /// size.  Also, verify that size is always `>=` the previous value (it's
   /// monotonically increasing).
   for( Iomem_summary_t* i = iomem_summary_head ; i != NULL ; i = i->next ) {
      i->countback_bucket = i->size ;

      if( i->size >= monotonic_size ) {
         monotonic_size = i->size ;
      } else {
         return false ;  // Somehow size is less than monotonic_size
      }
   }

   /// Next, iterate over the regions and subtract the region's size from
   /// `countback_bucket`.
   for( Iomem_region_t* i = &iomem_head ; i != NULL ; i = i->next ) {  // i is region
      for( Iomem_summary_t* j = iomem_summary_head ; j != NULL ; j = j->next ) { // j is summary
         if( strcmp( i->description, j->description ) == 0 ) {
            j->countback_bucket -= getEnd( i ) - i->start + 1 ;
         }
      }
   }

   /// Finally, iterate back over the summaries and ensure that the
   /// `countback_bucket`s are `0`.
   for( Iomem_summary_t* i = iomem_summary_head ; i != NULL ; i = i->next ) {
      // printf( "description=%s   summary_bucket=%zu\n", i->description, i->countback_bucket ) ;
      if( i->countback_bucket != 0 ) {
         // printf( "\n" );
         // printf( "validate_summary() has failed.  Dumping structures. \n") ;
         // print_iomem_regions() ;
         // print_iomem_summary() ;
         // exit( -1 ) ;
         return false ;
      }
   }

   return true ;
} // validate_summary


void summarize_iomem() {
   ASSERT( validate_iomem() ) ;

   compose_iomem_summary() ;
   sort_iomem_summary() ;

   ASSERT( validate_summary() ) ;

   print_iomem_summary() ;
}
