///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Comprehensive test of the Shannon Entropy module
///
/// @file   test_shannon.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <stdlib.h>  // For rand() srand()

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
   #include "../shannon.h"
}

BOOST_AUTO_TEST_SUITE( test_shannon )

BOOST_AUTO_TEST_CASE( test_computeShannonEntropy ) {
   BOOST_CHECK_CLOSE( computeShannonEntropy( "", 0 ), 0.000, 0.001 ) ;
   BOOST_CHECK_CLOSE( computeShannonEntropy( "I am Sam", 8 ), 2.250, 0.001 ) ;
   BOOST_CHECK_CLOSE( computeShannonEntropy( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 ), 4.700, 0.01 ) ;

   unsigned char buffer[ 4096 ] ;

   // Check a buffer that's filled with the same value:  0
   for( size_t i = 0 ; i < sizeof( buffer ) ; i++ ) {
      buffer[i] = 0 ;
   }
   BOOST_CHECK_CLOSE( computeShannonEntropy( buffer, sizeof( buffer )), 0.000, 0.001 ) ;

   // Check a buffer that's filled with an incrementing value:  i++
   for( size_t i = 0 ; i < sizeof( buffer ) ; i++ ) {
      buffer[i] = i & 0xFF ;
   }
   BOOST_CHECK_CLOSE( computeShannonEntropy( buffer, sizeof( buffer )), 8.000, 0.001 ) ;

   // Check a buffer that's filled with random values
   srand( 1001 ) ;  // Use a consistent set of random numbers
   for( size_t i = 0 ; i < sizeof( buffer ) ; i++ ) {
      buffer[i] = rand() & 0xFF ;
   }
   BOOST_CHECK_CLOSE( computeShannonEntropy( buffer, sizeof( buffer )), 7.900, 0.7 ) ;  // A tolerance of 0.69 would fail this test

   /// @todo Refactor maps so I can extract specific segments, then run shannon entropy on my own executable .text segments
} // test_computeShannonEntropy

BOOST_AUTO_TEST_CASE( test_getShannonClassification ) {
   BOOST_CHECK_EQUAL( getShannonClassification(  0.0        ), "No entropy" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification(  0.1        ), "Very low entropy" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 3.21 - 0.2  ), "English text in UNICODE" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 3.21 + 0.2  ), "English text in UNICODE" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 4.70 - 0.5  ), "English text in ASCII" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 4.70        ), "English text in ASCII" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 5.1252      ), "ARM code" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 5.940       ), "x86 code" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 5.940 + 0.4 ), "x86 code" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 7.500       ), "Encrypted/compressed" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 7.900       ), "Random" ) ;
   BOOST_CHECK_EQUAL( getShannonClassification( 8.000       ), "Random" ) ;
   BOOST_CHECK_FAIL( getShannonClassification( 8.001 ) ) ;
} // test_getShannonClassification

BOOST_AUTO_TEST_SUITE_END()
