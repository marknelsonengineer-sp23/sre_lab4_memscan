///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of iomap module
///
/// @file   test_iomap.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_MODULE Memscan
//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN  // in only one cpp file

#include <boost/test/included/unit_test.hpp>  // include this to get main()

#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

// using namespace std;


BOOST_AUTO_TEST_SUITE( test_xxx )

BOOST_AUTO_TEST_CASE( test_yyy ) {
   BOOST_CHECK_EQUAL( "A", "A" );
}

BOOST_AUTO_TEST_SUITE_END()
