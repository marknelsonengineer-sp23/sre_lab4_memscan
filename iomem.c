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
/// Here's a sample summary
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

#include <stdbool.h> // For bool, true & false
#include <stdio.h>   // For printf() fopen() fgets() sscanf() fclose() and FILE
#include <stdlib.h>  // For malloc() free()
#include <string.h>  // For strncpy() memset() strlen() strtok() strncmp()

#include "config.h"  // For FATAL_ERROR()
#include "iomem.h"   // Just cuz
#include "trim.h"    // For trim_edges()
#include "version.h" // For STRINGIFY_VALUE()


/// The longest allowed line length from #iomemFilePath
#define MAX_LINE_LENGTH 1024

/// Typedef of #Iomem_region
typedef struct Iomem_region Iomem_region_t ;

/// Hold each `iomem` region as elements in a linked list
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


/// The head pointer to the #Iomem_region linked list
///
/// Unlike many linked lists... this is __always__ set
Iomem_region_t iomem_head = { 0, NULL, UNMAPPED_MEMORY_DESCRIPTION } ;


/// Typedef of #Iomem_summary
typedef struct Iomem_summary Iomem_summary_t ;


/// A summary (sum of region sizes) of #Iomem_region records in a linked list
struct Iomem_summary {
   /// The total number of bytes for all of the regions with this `description`.
   size_t           size;
   /// Pointer to the next summary.
   Iomem_summary_t* next;
   /// The name/description of this memory region
   char             description[ MAX_IOMEM_DESCRIPTION ];
} ;


/// The head pointer to the #Iomem_summary linked list
Iomem_summary_t* iomem_summary_head = NULL ;


/// Compute the end address of a region
///
/// @param region The region to process
/// @return The ending physical address which is either:
///         1) The byte before the next region's starting address
///         2) or #MAX_PHYS_ADDR
void* getEnd( const Iomem_region_t* region ) {
   return ( region->next == NULL ) ? ((void*)MAX_PHYS_ADDR) : ( region->next->start - 1 );
}


/// Recursively zero out and free the `iomem` region linked list
///
/// @param region The region to zero out and free
void free_iomem_region( Iomem_region_t* region ) {  /// @NOLINT(misc-no-recursion): Recursion is authorized
   if( region->next != NULL ) {
      free_iomem_region( region->next ) ;
   }
   // Zero out the region
   memset( region, 0, sizeof( Iomem_region_t ) ) ;
   free( region ) ;
}


void release_iomem() {
   // Recursively zero out the linked list
   if( iomem_head.next != NULL ) {
      free_iomem_region( iomem_head.next ) ;
   }

   // Set the head pointer to its initial values
   iomem_head.start = 0 ;
   iomem_head.next = NULL ;
   strncpy( iomem_head.description, UNMAPPED_MEMORY_DESCRIPTION, MAX_IOMEM_DESCRIPTION ) ;

   // Free the summary list
   Iomem_summary_t* currentSummary = iomem_summary_head ;

   while( currentSummary != NULL ) {
      Iomem_summary_t* oldSummary = currentSummary ;
      currentSummary = currentSummary->next ;
      free( oldSummary ) ;
   }
   iomem_summary_head = NULL ;
}


/// Validate the `iomem` linked list
///
/// The validation rules are:
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

   void* monotonic_address = 0 ;

   Iomem_region_t* current = &iomem_head;

   while( current != NULL ) {
      if( monotonic_address != 0 && current->start <= monotonic_address ) {
         return false ;
      }
      monotonic_address = current->start ;
      current = current->next;
   }

   return true;
}


/// Print all of the `iomem` regions.
///
/// Assumes a 48-bit physical address bus size (6 bytes or 12 characters wide).
void print_iomem_regions() {
   Iomem_region_t* current = &iomem_head ;

   while( current != NULL ) {
      void* end = getEnd( current ) ;

      printf( "%012zx - %012zx  %s\n", (size_t) current->start, (size_t) end, current->description ) ;
      current = current->next ;
   }
}


const char* get_iomem_region_description( const void* physAddr ) {
   Iomem_region_t* region = &iomem_head;

   while( region->next != NULL && region->next->start <= physAddr ) {
      region = region->next;
   }

   return region->description ;
}


/// Add a new `iomem` region
///
/// @param start The starting physical address of the new region (inclusive).
///              It may overlap with an existing region.
/// @param end   The ending physical address of the new region.  It must be
///              within an existing region.  `end` may not be `NULL` and must be
///              `> start`.
/// @param description The description of a region (up to #MAX_IOMEM_DESCRIPTION
///                    bytes).  The description may not be `NULL` or empty.
void add_iomem_region( const void* start, const void* end, const char* description ) {
   ASSERT( validate_iomem() ) ;
   ASSERT( end != NULL ) ;
   ASSERT( end > start ) ;
   ASSERT( description != NULL ) ;
   ASSERT( strlen( description ) != 0 ) ;

   // Find the start...
   Iomem_region_t* current = &iomem_head ;

   void* current_end = getEnd( current ) ;

   while( current->next != NULL && current->start < start && current_end < end ) {
      current = current->next ;
      current_end = getEnd( current ) ;
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
      #ifndef TESTING
         print_iomem_regions() ;
      #endif
      FATAL_ERROR( "requested region overlaps and is not valid" );
   }

   ASSERT( validate_iomem() ) ;
} // add_iomem_region


void read_iomem() {
   release_iomem() ;

   FILE* file = NULL ;  // File handle to #iomemFilePath

   file = fopen( iomemFilePath, "r" ) ;
   if( file == NULL ) {
      FATAL_ERROR( "Unable to open [%s]", iomemFilePath ) ;
   }

   char* pRead ;
   char szLine[ MAX_LINE_LENGTH ] ;

   pRead = fgets( szLine, MAX_LINE_LENGTH, file ) ;
   trim_edges( szLine ) ;

   while( pRead != NULL ) {
      #ifdef DEBUG
         printf( "%s\n", szLine ) ;
      #endif

      char* sAddressStart = strtok( szLine, "-" ) ;
      char* sAddressEnd = strtok( NULL, " "   ) ;
      char* sDescription = strtok( NULL, "\n" ) + 2 ;  // The +2 skips over a ": "

      // Convert the strings holding the start & end address into pointers
      int retVal1 ;
      int retVal2 ;
      void* pAddressStart ;
      void* pAddressEnd ;
      retVal1 = sscanf( sAddressStart, "%p", &pAddressStart ) ;
      retVal2 = sscanf( sAddressEnd,   "%p", &pAddressEnd   ) ;

      if( retVal1 != 1 || retVal2 != 1 ) {
         FATAL_ERROR( "iomem entry [%s] is unable parse start [%s] or end address [%s]"
         ,sDescription
         ,sAddressStart
         ,sAddressEnd
         ) ;
      }

      #ifdef DEBUG
         printf( "DEBUG:  " ) ;
         printf( "sAddressStart=[%s]  ", sAddressStart ) ;
         printf( "pAddressStart=[%p]  ", pAddressStart ) ;
         printf( "sAddressEnd=[%s]  ",   sAddressEnd ) ;
         printf( "pAddressEnd=[%p]  ",   pAddressEnd ) ;
         printf( "sDescription=[%s]  ",  sDescription ) ;
         printf( "\n" ) ;
      #endif

      add_iomem_region( pAddressStart, pAddressEnd, sDescription ) ;

      pRead = fgets( szLine, MAX_LINE_LENGTH, file );
      trim_edges( szLine ) ;
   } // while()


   int iRetVal = fclose( file ) ;
   if( iRetVal != 0 ) {
      FATAL_ERROR( "Unable to close [%s]", iomemFilePath ) ;
   }
} // read_iomem


/// Print the linked list of #Iomem_summary records under #iomem_summary_head
void print_iomem_summary() {
   printf( "Summary of %s\n", iomemFilePath );

   Iomem_summary_t* type = iomem_summary_head;

   while( type != NULL ) {
      printf( "%-" STRINGIFY_VALUE( MAX_IOMEM_DESCRIPTION ) "s  ", type->description ) ;
      printf( "%'20zu", type->size ) ;
      printf( "\n" ) ;
      type = type->next ;
   }
}


/// Sort the linked list of #Iomem_summary records under #iomem_summary_head
///
/// Implementation of a bubble sort of a linked list
void sort_iomem_summary() {
   bool swapped;
   do {
      swapped = false;

      Iomem_summary_t* type = iomem_summary_head ;
      Iomem_summary_t* prev = NULL ;

      while( type != NULL && type->next != NULL ) {  // While there are at least 2 types...
         // printf( "head=[%p]  prev=[%p]  type=[%p, %zu]  type->next=[%p, %zu]  type->next->next=[%p]\n", iomem_type_head, prev, type, type->size, type->next, type->next->size, type->next->next ) ;
         if( type->size > type->next->size ) {  // Swap the two types...
            swapped = true;
            if( prev == NULL ) {  // Swap involves the head...
               // printf( "Swap head\n" );
               Iomem_summary_t* temp = iomem_summary_head->next->next;
               iomem_summary_head = type->next;
               iomem_summary_head->next = type;
               type->next = temp;
               type = iomem_summary_head;
            } else {
               // printf( "Swap interior" );
               Iomem_summary_t* temp = type->next->next;
               prev->next = type->next;
               prev->next->next = type;
               type->next = temp;
               type = prev->next;
            }
         }
         prev = type;
         type = type->next;
      }
   } while( swapped ) ;  // If we swapped two rows, then keep sorting
} // sort_iomem


/// Iterate over the #Iomem_region linked list from #iomem_head and either
/// find an existing #Iomem_summary (based on matching the description) and add
/// to it or create a new one.
void compose_iomem_summary() {
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


void summarize_iomem() {
   compose_iomem_summary() ;
   sort_iomem_summary() ;
   print_iomem_summary() ;
}
