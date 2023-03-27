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

#include <filesystem>   // For std::filesystem
#include <string>       // For strncpy()

extern "C" {
   #include "../config.h"
   #include "../maps.h"
}

BOOST_AUTO_TEST_SUITE( test_maps )

BOOST_AUTO_TEST_CASE( test_getMaps_bulk ) {
   /// Read the contents of `./tests/test_maps`, which contains sample `maps` files from
   /// a variety of Linux systems.
   const std::filesystem::path sandbox{"test_maps"};

   for (auto const& dir_entry : std::filesystem::directory_iterator{sandbox}) {
      // std::cout << dir_entry.path() << '\n';
      strncpy( mapsFilePath, dir_entry.path().u8string().c_str(), sizeof( mapsFilePath ) ) ;
      BOOST_CHECK_NO_THROW( getMaps() ) ;
   }
}

/* WHITEBOX TEST */
#define MAX_ENTRIES     256
extern "C" struct MapEntry map[MAX_ENTRIES] ;


// This isn't totally comprehensive, but it's better than nothing
BOOST_AUTO_TEST_CASE( test_getMaps_unit ) {
   strcpy( mapsFilePath, "test_maps/maps.test" ) ;
   BOOST_CHECK_NO_THROW( getMaps() ) ;

   BOOST_CHECK_EQUAL( map[0].sAddressStart, "00000000" ) ;
   BOOST_CHECK_EQUAL( map[0].pAddressStart, (void*) 0x00000000 ) ;

   BOOST_CHECK_EQUAL( map[0].sAddressEnd, "ffffffff" ) ;
   BOOST_CHECK_EQUAL( map[0].pAddressEnd, (void*) 0xffffffff ) ;

   BOOST_CHECK_EQUAL( map[0].sPermissions, "rwxp" ) ;
   BOOST_CHECK_EQUAL( map[0].sOffset, "01234567" ) ;
   BOOST_CHECK_EQUAL( map[0].sDevice, "00:02" ) ;

   BOOST_CHECK_EQUAL( map[0].sPath, "/init" ) ;
   BOOST_CHECK_EQUAL( map[0].include, true ) ;

   BOOST_CHECK_EQUAL( map[0].numBytes, 0xffffffff ) ;
   BOOST_CHECK_EQUAL( map[0].numPages,    0xfffff ) ;

   BOOST_CHECK_EQUAL( map[0].pages, (void*) NULL ) ;

   BOOST_CHECK_EQUAL( map[0].numBytesFound, 0 ) ;
   BOOST_CHECK_EQUAL( map[0].shannonEntropy, 0.0 ) ;
}

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
