///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the config module
///
/// @file   test_config.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_MODULE memscan
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>  // include this to get main()
#include <boost/test/unit_test.hpp>

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
   #include "../config.h"
}

BOOST_AUTO_TEST_SUITE( test_config )

BOOST_AUTO_TEST_CASE( test_ProgramName ) {
   BOOST_CHECK_EQUAL( getProgramName(), "memscan" ) ;

   BOOST_CHECK( setProgramName( "Sam" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;

   BOOST_CHECK( !setProgramName( NULL ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK( !setProgramName( "" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK( !setProgramName( "  \t\t\n\n  " ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK( setProgramName( "  \t\tChili\n\n  " ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Chili" ) ;
} // test_ProgramName

BOOST_AUTO_TEST_CASE( test_getEndianness ) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   BOOST_CHECK_EQUAL( getEndianness(), LITTLE ) ;
#else
   BOOST_CHECK_EQUAL( getEndianness(), BIG ) ;
#endif
}

BOOST_AUTO_TEST_CASE( test_GET_BIT ) {
   BOOST_CHECK(  GET_BIT( 0b0001, 0 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1110, 0 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b0010, 1 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1101, 1 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b0100, 2 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1011, 2 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b1000, 3 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b0111, 3 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x8000000000000000, 63 ) ) ;
   BOOST_CHECK( !GET_BIT( 0x7fffffffffffffff, 63 ) ) ;
} // test_GET_BIT


/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because it's white box tests, it's on the
   tester to ensure the code is in sync.                                     */

extern "C" size_t getOptargNumericValue( char* optarg ) ;

/* ***************************************************************************/


BOOST_AUTO_TEST_CASE( test_getOptargNumericValue ) {
   { char buff[10] = "100" ;   BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 100 ) ; }
   { char buff[10] = "-100" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "100X" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
} // test_getOptargNumericValue

BOOST_AUTO_TEST_SUITE_END()
