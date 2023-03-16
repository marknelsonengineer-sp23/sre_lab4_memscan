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

#include <stdio.h>  // For fopen() fclose()

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
   #include "../config.h"
}

BOOST_AUTO_TEST_SUITE( test_config )

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


/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because it's white box tests, it's on the
   tester to ensure the code is in sync.                                     */

extern "C" size_t getOptargNumericValue( char* optarg ) ;

/* ***************************************************************************/


BOOST_AUTO_TEST_CASE( test_getOptargNumericValue ) {
   BOOST_CHECK_FAIL( getOptargNumericValue( NULL ) ) ;

   // Test regular numbers
   { char buff[10] =   "0"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1 ) ; }
   { char buff[10] = "  1 " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1 ) ; }
   { char buff[10] =  "10"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10 ) ; }
   { char buff[10] = " 10 " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10 ) ; }
   { char buff[10] = "100"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 100 ) ; }

   // Test k unit
   { char buff[10] =   "0k"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0k " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1k"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1000 ) ; }
   { char buff[10] = "  1k " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1000 ) ; }
   { char buff[10] =  "10k"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10000 ) ; }
   { char buff[10] = " 10k " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10000 ) ; }
   { char buff[10] = "100k"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 100000 ) ; }

   // Test K unit
   { char buff[10] =   "0K"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0K " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1K"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1024 ) ; }
   { char buff[10] = "  1K " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1024 ) ; }
   { char buff[10] =  "10K"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10240 ) ; }
   { char buff[10] = " 10K " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10240 ) ; }
   { char buff[10] = "100K"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 102400 ) ; }

   // Test m unit
   { char buff[10] =   "0m"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0m " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1m"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1000000 ) ; }
   { char buff[10] = "  1m " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1000000 ) ; }
   { char buff[10] =  "10m"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10000000 ) ; }
   { char buff[10] = " 10m " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10000000 ) ; }
   { char buff[10] = "100m"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 100000000 ) ; }

   // Test M unit
   { char buff[10] =   "0M"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0M " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1M"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1048576 ) ; }
   { char buff[10] = "  1M " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1048576 ) ; }
   { char buff[10] =  "10M"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10485760 ) ; }
   { char buff[10] = " 10M " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10485760 ) ; }
   { char buff[10] = "100M"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 104857600 ) ; }

   // Test g unit
   { char buff[10] =   "0g"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0g " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1g"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1000000000 ) ; }
   { char buff[10] = "  1g " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1000000000 ) ; }
   { char buff[10] =  "10g"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10000000000 ) ; }
   { char buff[10] = " 10g " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10000000000 ) ; }
   { char buff[10] = "100g"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 100000000000 ) ; }

   // Test G unit
   { char buff[10] =   "0G"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] = "  0G " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 0 ) ; }
   { char buff[10] =   "1G"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1073741824 ) ; }
   { char buff[10] = "  1G " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 1073741824 ) ; }
   { char buff[10] =  "10G"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10737418240 ) ; }
   { char buff[10] = " 10G " ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 10737418240 ) ; }
   { char buff[10] = "100G"  ;  BOOST_CHECK_EQUAL( getOptargNumericValue( buff ), 107374182400 ) ; }

   // Test negative numbers
   { char buff[10] =   "-0" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "  -0" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] =   "-1" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "  -1" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] =  "-10" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = " -10" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "-100" ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }

   // Test improper units
   { char buff[10] =   "0a"  ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "  0b " ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] =   "1c"  ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "  1d " ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] =  "10e"  ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = " 10f " ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "100h"  ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }

   // Other bad things
   { char buff[10] = "0x00"  ;   BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "1.0 " ;    BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "" ;        BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = " " ;       BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "  " ;      BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "Blob" ;    BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }
   { char buff[10] = "  Bx  " ;  BOOST_CHECK_FAIL( getOptargNumericValue( buff ) ) ; }

} // test_getOptargNumericValue

BOOST_AUTO_TEST_SUITE_END()
