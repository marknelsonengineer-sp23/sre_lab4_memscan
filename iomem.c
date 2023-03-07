///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process `/proc/iomem` to characterize physical pages
///
/// Here's a sample of `proc\iomem`
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
/// @file   iomem.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h> // For true & false
#include <stddef.h>  // For NULL
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For malloc() free()
#include <string.h>  // For strncpy() memset()

#include "config.h"
#include "iomem.h"


/// The file to read from `/proc`
#define IOMEM_FILE "/proc/iomem"

/// The longest allowed length from #IOMEM_FILE
#define MAX_LINE_LENGTH 1024

/// Typedef of #Iomem_region
typedef struct Iomem_region Iomem_region_t ;

/// Hold the iomem data structure as a linked list
struct Iomem_region {
                   /// The starting physical address of this memory region.
                   /// This will **_NOT_** be a valid pointer as it's a physical address.
   void*           start;
                   /// Pointer to the next region.
                   /// The end address of this region is `next->start - 1` unless
                   /// `next == NULL`, then it's #MAX_PHYS_ADDR
   Iomem_region_t* next;
                   /// The name/description of this memory region
   char            description[ MAX_IOMEM_DESCRIPTION ];
} ;


/// The head pointer to the iomem linked list
///
/// Unlike many linked lists... this is __always__ set
Iomem_region_t iomem_head = { 0, NULL, UNMAPPED_MEMORY_DESCRIPTION } ;


/// Recursively zero out and free the linked list
/// @param region
void free_iomem_region( Iomem_region_t* region ) {  /// @NOLINT(misc-no-recursion): Recursion is authorized
   if( region->next != NULL ) {
      free_iomem_region( region->next ) ;
   }
   // Zero out the region
   memset( region, 0, sizeof( Iomem_region_t ) ) ;
   free( region ) ;
}


/// Reset the iomem data structure
void reset_iomem() {
   // Recursively zero out the linked list
   if( iomem_head.next != NULL ) {
      free_iomem_region( iomem_head.next ) ;
   }

   // Set the head pointer to its initial values
   iomem_head.start = 0 ;
   iomem_head.next = NULL ;
   strcpy( iomem_head.description, UNMAPPED_MEMORY_DESCRIPTION ) ;
}


/// Validate the iomem linked list
///
/// The validation rules are:
///   - The list can not be empty
///   - The first start address must be 0
///   - Every region must monotonically increase
///
/// @return `true` if the structure is valid.  `false` if it's not.
bool validate_iomem() {
   if( iomem_head.start != 0 ) {
      return false ;
   }

   void* cumulative_start = 0 ;

   Iomem_region_t* current = &iomem_head;

   while( current != NULL ) {
      if( cumulative_start != 0 && current->start <= cumulative_start ) {
         return false ;
      }
      cumulative_start = current->start ;
      current = current->next;
   }

   return true;
}


const char* get_iomem_region_description( const void* physAddr ) {
   Iomem_region_t* region = &iomem_head;

   while( region->next != NULL && region->next->start <= physAddr ) {
      region = region->next;
   }

   return region->description ;
}


/// Add a new iomem region
///
/// @param start The starting physical address of the new region (inclusive).
///              It may overlap with an existing region.
/// @param end   The ending physical address of the new region.  It must be
///              within an existing region.  `end` may not be `NULL`.
/// @param description The description of a region (up to #MAX_IOMEM_DESCRIPTION
///                    bytes).  The description may not be `NULL` or empty.
void add_iomem_region( const void* start, const void* end, const char* description ) {
   if( !validate_iomem() ) {
      FATAL_ERROR( "invalid iomem structure" ) ;
   }

   if( end == NULL ) {
      FATAL_ERROR( "end can not be null" ) ;
   }

   if( description == NULL ) {
      FATAL_ERROR( "invalid iomem description" ) ;
   }

   if( strlen( description ) == 0 ) {
      FATAL_ERROR( "description can not be empty" ) ;
   }

   // Find the start...
   Iomem_region_t* current = &iomem_head;

   void* current_end = ( current->next == NULL ) ? ((void*) MAX_PHYS_ADDR) : ( current->next->start - 1 );

   while( current->next != NULL && current->start < start && current_end < end ) {
      current = current->next;
      current_end = ( current->next == NULL ) ? ((void*) MAX_PHYS_ADDR) : ( current->next->start - 1 );
   }

   // current should point to the region we need to insert into

   // printf( "addStart=%p addEnd=%p addDesc=[%s] curStart=%p curEnd=%p, curDesc=[%s]\n", start, end, description, current->start, current_end, current->description ) ;

   /// The following definitions will help decode how this is implemented:
   ///   - `cur` Current region
   ///   - `add` The region we want to add
   ///   - `old` The original region record
   ///   - `new` The new region record (added to the right of `old` in the linked
   ///           list)
   ///
   /// There are several scenarios for inserting regions...
   ///
   if( current->start == start && current_end == end ) {
      /// - If `addStart == currStart && addEnd == curEnd`, then just replace
      ///   the description:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, curEnd, addDesc>
      ///   ````
      // printf( "Replace region\n" );
      strncpy( current->description, description, MAX_IOMEM_DESCRIPTION ) ;
   } else if( current->start == start && current_end > end ) {
      /// - If the `addStart == curStart` and `addEnd < curEnd`, then insert a
      ///   new region in the linked list:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, addEnd, addDesc> -> <addEnd+1, curEnd, curDesc>
      ///   ````
      // printf( "Create new-left\n" );

      Iomem_region_t* newRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( newRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }

      strncpy( newRegion->description, current->description, MAX_IOMEM_DESCRIPTION ) ;
      newRegion->start = (void*) end+1 ;
      newRegion->next = current->next ;
      current->next = newRegion ;
      current->start = (void*) start ;
      strncpy( current->description, description, MAX_IOMEM_DESCRIPTION ) ;
   } else if( current->start < start && current_end == end ) {
      /// - If the `addStart > curStart` and `addEnd == curEnd`, then insert a
      ///   new region in the linked list:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, addStart-1, curDesc> -> <addStart, addEnd, addDesc>
      ///   ````
      // printf( "Create new-right\n" );
      Iomem_region_t* newRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( newRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }

      strncpy( newRegion->description, description, MAX_IOMEM_DESCRIPTION ) ;
      newRegion->start = (void*) start ;
      newRegion->next = current->next ;
      current->next = newRegion ;
   } else if( current->start < start && current_end > end ) {
      /// - If the `addStart > curStart` and `addEnd < curEnd`, then insert two
      ///   new regions in the linked list:
      ///   ````
      ///   <curStart, curEnd, curDesc> --> <curStart, addStart-1, curDesc> -> <addStart, addEnd, addDesc> -> <addEnd+1, curEnd, curDesc>
      ///   ````
      // printf( "Create new-middle\n" );
      Iomem_region_t* newRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( newRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }
      strncpy( newRegion->description, current->description, MAX_IOMEM_DESCRIPTION ) ;
      newRegion->start = (void*) end+1 ;
      newRegion->next = current->next ;

      Iomem_region_t* middleRegion = malloc( sizeof( Iomem_region_t ) ) ;
      if( middleRegion == NULL ) {
         FATAL_ERROR( "Unable to allocate new iomem region" ) ;
      }

      strncpy( middleRegion->description, description, MAX_IOMEM_DESCRIPTION ) ;
      middleRegion->start = (void*) start ;
      middleRegion->next = newRegion ;
      current->next = middleRegion ;
   } else {
      /// - Anything else should return an error (it's an overlapping region)
      print_iomem() ;
      FATAL_ERROR( "requested region overlaps and is not valid" );
   }

   if( !validate_iomem() ) {
      FATAL_ERROR( "iomem is not valid before add" ) ;
   }
}


void print_iomem() {
   Iomem_region_t* current = &iomem_head;

   while( current != NULL ) {
      void* end = ( current->next == NULL ) ? ((void*)MAX_PHYS_ADDR) : ( current->next->start - 1 );

      printf( "%012zx - %012zx  %s\n", (size_t) current->start, (size_t) end, current->description );
      current = current->next;
   }
}
