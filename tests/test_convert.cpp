///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the convert module
///
/// @file   test_convert.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#include <boost/test/unit_test.hpp>

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
#include "../convert.h"
}


BOOST_AUTO_TEST_SUITE( test_convert )

BOOST_AUTO_TEST_CASE( test_getOptargNumericValue ) {
        BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( NULL ) ) ;

        // Test regular numbers
        { char buff[10] =   "0"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1 ) ; }
        { char buff[10] = "  1 " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1 ) ; }
        { char buff[10] =  "10"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10 ) ; }
        { char buff[10] = " 10 " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10 ) ; }
        { char buff[10] = "100"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 100 ) ; }

        // Test k unit
        { char buff[10] =   "0k"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0k " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1k"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1000 ) ; }
        { char buff[10] = "  1k " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1000 ) ; }
        { char buff[10] =  "10k"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10000 ) ; }
        { char buff[10] = " 10k " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10000 ) ; }
        { char buff[10] = "100k"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 100000 ) ; }

        // Test K unit
        { char buff[10] =   "0K"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0K " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1K"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1024 ) ; }
        { char buff[10] = "  1K " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1024 ) ; }
        { char buff[10] =  "10K"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10240 ) ; }
        { char buff[10] = " 10K " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10240 ) ; }
        { char buff[10] = "100K"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 102400 ) ; }

        // Test m unit
        { char buff[10] =   "0m"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0m " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1m"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1000000 ) ; }
        { char buff[10] = "  1m " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1000000 ) ; }
        { char buff[10] =  "10m"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10000000 ) ; }
        { char buff[10] = " 10m " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10000000 ) ; }
        { char buff[10] = "100m"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 100000000 ) ; }

        // Test M unit
        { char buff[10] =   "0M"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0M " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1M"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1048576 ) ; }
        { char buff[10] = "  1M " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1048576 ) ; }
        { char buff[10] =  "10M"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10485760 ) ; }
        { char buff[10] = " 10M " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10485760 ) ; }
        { char buff[10] = "100M"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 104857600 ) ; }

        // Test g unit
        { char buff[10] =   "0g"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0g " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1g"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1000000000 ) ; }
        { char buff[10] = "  1g " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1000000000 ) ; }
        { char buff[10] =  "10g"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10000000000 ) ; }
        { char buff[10] = " 10g " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10000000000 ) ; }
        { char buff[10] = "100g"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 100000000000 ) ; }

        // Test G unit
        { char buff[10] =   "0G"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] = "  0G " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 0 ) ; }
        { char buff[10] =   "1G"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1073741824 ) ; }
        { char buff[10] = "  1G " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 1073741824 ) ; }
        { char buff[10] =  "10G"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10737418240 ) ; }
        { char buff[10] = " 10G " ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 10737418240 ) ; }
        { char buff[10] = "100G"  ;  BOOST_CHECK_EQUAL( stringToUnsignedLongLongWithScale( buff ), 107374182400 ) ; }

        // Test negative numbers
        { char buff[10] =   "-0" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "  -0" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] =   "-1" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "  -1" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] =  "-10" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = " -10" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "-100" ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }

        // Test improper units
        { char buff[10] =   "0a"  ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "  0b " ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] =   "1c"  ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "  1d " ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] =  "10e"  ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = " 10f " ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "100h"  ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }

        // Other bad things
        { char buff[10] = "0x00"  ;   BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "1.0 " ;    BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "" ;        BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = " " ;       BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "  " ;      BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "Blob" ;    BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }
        { char buff[10] = "  Bx  " ;  BOOST_CHECK_FAIL( stringToUnsignedLongLongWithScale( buff ) ) ; }

} // test_getOptargNumericValue

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
