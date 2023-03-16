///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Compute the Shannon Entropy of various buffers
///
/// @see https://en.wikipedia.org/wiki/Entropy_(information_theory)
/// @see https://rosettacode.org/wiki/Entropy#C
///
/// @file   shannon.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <math.h>    // For log2()
#include <string.h>  // For memset()

#include "config.h"  // For ASSERT()
#include "shannon.h"

double computeShannonEntropy( const void* buffer, const size_t length ) {
   // printf( "buffer=[%s]  length=%zu\n", (const char*)buffer, length ) ;

   int histogram[256] ;  /// @NOLINT(readability-magic-numbers): In this context, it's OK to use 256 here
   double H = 0.0 ;  /// @NOLINT(readability-identifier-length): H is the standard symbol for Shannon Entropy
   size_t number_of_unique_values = 0 ;

   memset( histogram, 0, sizeof( histogram ) ) ;

   for( size_t i = 0 ; i < length ; i++ ) {
      unsigned char value = ((const char*)buffer)[i] ;
      histogram[ value ] += 1 ;
   }

   for( size_t i = 0 ; i < 256 ; i++ ) {  // @NOLINT(readability-magic-numbers)
      // printf( "%3zu: %3d  %s", i, histogram[i], ( i > 0 && i % 16 == 0 ) ? "\n" : "  " ) ;
      number_of_unique_values++ ;
   }

   // printf( "number_of_unique_values=%zu\n", number_of_unique_values );

   for (int i = 0 ; i < 256 ; i++) {  // @NOLINT(readability-magic-numbers)
      if( histogram[i] != 0 ) {
         // printf( "i=%d  histogram[i]=%d  H=%lf\n", i, histogram[i], H ) ;
         H -= (double)histogram[i] / (double)length * log2( (double)histogram[i] / (double)length );
      }
   }

   return H ;
}

/// Shannon entropy can be used to classify data.  For example, data between
/// 7.5 and 7.9 tends to be encrypted or compressed, where data > 7.9 tends
/// to be random.  This table holds the relationship between Shannon entropy
/// and a guess (classification) as to the content.
///
/// The table, like many of the data structures in memscan, contains a single
/// boundary and a label.  The adjoining record holds the other boundary.  This
/// way, we can't have gaps or overlapping regions.
///
/// @NOLINTBEGIN(readability-magic-numbers): Classification will have magic numbers
struct Classification {
   double atLeast ;          ///< The lower bound of the Shannon Entropy for this classification.
                             ///< The upper bound is the next record.
   char   description[MAX_SHANNON_CLASSIFICATION_LENGTH] ;  ///< A label for this classification
} ;


/// Shannon entropy values & labels for classifying data.
/// Shannon entropy (on 8-bit data) has a range from 0.000 to 8.000.
static struct Classification classification[] = {
         { 0.000, "No entropy" }
        ,{ 0.001, "Very low entropy" }
        ,{ 1.001, "Unknown" }
        ,{ 3.000, "Heap (filled)" }
        ,{ 3.001, "Unknown" }
        ,{ 3.010, "English text in UNICODE" }
        ,{ 3.411, "Unknown" }
        ,{ 4.200, "English text in ASCII" }
        ,{ 4.908, "ARM code" }
        ,{ 5.588, "x86 code" }
        ,{ 6.341, "Unknown" }
        ,{ 7.500, "Encrypted/compressed" }
        ,{ 7.900, "Random" }
        ,{ 8.001, "OUT OF BOUNDS" }
};


/// The maximum value of Shannon Entropy for 8-bit bytes
#define MAX_SHANNON_ENTROPY 8.0

char* getShannonClassification( const double entropy ) {
   ASSERT( entropy <= MAX_SHANNON_ENTROPY ) ;

   size_t current = 0 ;

   while( classification[ current + 1 ].atLeast <= entropy ) {
      current += 1 ;
   }

   // @NOLINTEND(readability-magic-numbers)
   return classification[ current ].description ;
}
