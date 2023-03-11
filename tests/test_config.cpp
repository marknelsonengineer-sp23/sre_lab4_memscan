///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the config module
///
/// @file   test_config.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

//#include <boost/test/included/unit_test.hpp>  // include this to get main()

//#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

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
}

BOOST_AUTO_TEST_SUITE_END()
