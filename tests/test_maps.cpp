///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the maps module
///
/// @file   test_maps.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

#include <string>       // For strcpy()
#include <filesystem>   // For std::filesystem

extern "C" {
   #include "../config.h"
   #include "../maps.h"
}

BOOST_AUTO_TEST_SUITE( test_maps )

BOOST_AUTO_TEST_CASE( test_getMaps ) {
   /// Read the contents of `./tests/test_maps`, which contains sample `maps` files from
   /// a variety of Linux systems.
   const std::filesystem::path sandbox{"test_maps"};

   for (auto const& dir_entry : std::filesystem::directory_iterator{sandbox}) {
      // std::cout << dir_entry.path() << '\n';
      strcpy( mapsFilePath, dir_entry.path().u8string().c_str() ) ;
      BOOST_CHECK_NO_THROW( getMaps() ) ;
   }
}

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
