///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the config module
///
/// @file   test_config.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#define BOOST_TEST_MODULE memscan
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>  // include this to get main()
#include <boost/test/unit_test.hpp>

#include <stddef.h>     // For size_t
#include <stdio.h>      // For fopen() fclose()
#include <sys/types.h>  // For ssize_t

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
   #include "../config.h"
}

BOOST_AUTO_TEST_SUITE( test_config )

BOOST_AUTO_TEST_CASE( test_architecture ) {
   // Make sure pointers are the same size as `size_t`
   BOOST_CHECK_EQUAL( sizeof( void* ), sizeof( size_t ) ) ;

   BOOST_CHECK_EQUAL( sizeof( ssize_t ), sizeof( size_t ) ) ;
}


BOOST_AUTO_TEST_CASE( test_getEndianness ) {
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      BOOST_CHECK_EQUAL( getEndianness(), LITTLE ) ;
   #else
      BOOST_CHECK_EQUAL( getEndianness(), BIG ) ;
   #endif
}


BOOST_AUTO_TEST_CASE( test_printUsage ) {
   FILE* dev_null = fopen( "/dev/null", "w" ) ;
   BOOST_CHECK( dev_null != NULL ) ;
   BOOST_CHECK_FAIL( printUsage( NULL ) ) ;
   BOOST_CHECK_NO_THROW( printUsage( dev_null ) ) ;
   fclose( dev_null ) ;  // NOLINT(cert-err33-c): No need to check the return value of fclose()
}


BOOST_AUTO_TEST_CASE( test_ProgramName ) {
   BOOST_CHECK_EQUAL( getProgramName(), "memscan" ) ;

   BOOST_CHECK_NO_THROW( setProgramName( "Sam" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;

   BOOST_CHECK_FAIL( setProgramName( NULL ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK_FAIL( setProgramName( "" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK_FAIL( setProgramName( "  \t\t\n\n  " ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK_NO_THROW( setProgramName( "  \t\tChili\n\n  " ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Chili" ) ;  // The name was trimmed...

   BOOST_CHECK_NO_THROW( setProgramName( "123456789012345678901234567890" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "123456789012345678901234567890" ) ;  // 30-character names are OK

   BOOST_CHECK_NO_THROW( setProgramName( "1234567890123456789012345678901" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "1234567890123456789012345678901" ) ;  // 31-character names are OK

   BOOST_CHECK_NO_THROW( setProgramName( "12345678901234567890123456789012" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "1234567890123456789012345678901" ) ;  // 32-character names map to 31 characters

   BOOST_CHECK_NO_THROW( setProgramName( "123456789012345678901234567890123" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "1234567890123456789012345678901" ) ;  // 33-character names map to 31 characters

   BOOST_CHECK_NO_THROW( setProgramName( "memscan" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "memscan" ) ;
} // test_ProgramName


BOOST_AUTO_TEST_CASE( test_GET_BIT ) {
   BOOST_CHECK(  GET_BIT( 0b0001, 0 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1110, 0 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b0010, 1 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1101, 1 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b0100, 2 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1011, 2 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b1000, 3 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b0111, 3 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x1000000000000000, 60 ) ) ;
   BOOST_CHECK( !GET_BIT( 0xefffffffffffffff, 60 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x2000000000000000, 61 ) ) ;
   BOOST_CHECK( !GET_BIT( 0xdfffffffffffffff, 61 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x4000000000000000, 62 ) ) ;
   BOOST_CHECK( !GET_BIT( 0xbfffffffffffffff, 62 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x8000000000000000, 63 ) ) ;
   BOOST_CHECK( !GET_BIT( 0x7fffffffffffffff, 63 ) ) ;
} // test_GET_BIT

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
