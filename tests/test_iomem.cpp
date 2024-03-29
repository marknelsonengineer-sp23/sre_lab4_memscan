///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the iomem module
///
/// @file   test_iomem.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#include <boost/test/unit_test.hpp>

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

#include <filesystem>         // For std::filesystem

extern "C" {
   #include "../config.h"     // For iomemFilePath
   #include "../iomem.h"      // For testing
}


/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because these are white box tests, it's on
   the tester to ensure the code is in sync.                                 */

extern "C" bool validate_iomem() ;
extern "C" void add_iomem_region( const_pfn_t start, const_pfn_t end, const char* description ) ;
extern "C" void print_iomem_regions() ;
extern "C" void compose_iomem_summary() ;
extern "C" void sort_iomem_summary() ;
extern "C" bool validate_summary() ;
extern "C" void print_iomem_summary() ;

// extern "C" void print_iomem_summary() ;

/* ***************************************************************************/

struct TestIomemFixture {
   TestIomemFixture()  {
      BOOST_TEST_MESSAGE( "setup fixture" ) ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
      BOOST_CHECK( validate_iomem() ) ;
   }
   ~TestIomemFixture() {
      BOOST_TEST_MESSAGE( "teardown fixture" ) ;
      BOOST_CHECK( validate_iomem() ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      BOOST_CHECK( validate_summary() ) ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
      BOOST_CHECK( validate_iomem() ) ;
   }
} ;

BOOST_AUTO_TEST_SUITE( test_iomem )

BOOST_FIXTURE_TEST_CASE( test_read_iomem_bulk, TestIomemFixture ) {
    /// Read the contents of `./tests/test_iomem`, containing sample `iomem`
    /// files from a variety of Linux systems.
   const std::filesystem::path sandbox{"test_iomem"} ;

   for (auto const& dir_entry : std::filesystem::directory_iterator{sandbox}) {
      // std::cout << dir_entry.path() << std::endl ;
      strncpy( iomemFilePath, dir_entry.path().u8string().c_str(), sizeof( iomemFilePath ) ) ;
      BOOST_CHECK_NO_THROW( read_iomem() ) ;
      BOOST_CHECK_EQUAL( validate_iomem(), true ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      BOOST_CHECK( validate_summary() ) ;
      // BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
      // print_iomem_regions() ;
   }
}


BOOST_FIXTURE_TEST_CASE( test_iomem_release, TestIomemFixture ) {
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   BOOST_CHECK( validate_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;

   BOOST_CHECK_NO_THROW( release_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   BOOST_CHECK( validate_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
}


BOOST_FIXTURE_TEST_CASE( test_iomem_add, TestIomemFixture ) {
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x7ffffffffff ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x00, MAX_PHYS_ADDR, "replaced - all") ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x7ffffffffff ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x0FF, "region 0 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x100, 0x1FF, "region 1 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x200, 0x2FF, "region 2 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x300, 0x3FF, "region 3 - new") ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x000 ), "region 0 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0FF ), "region 0 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 ), "region 1 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1FF ), "region 1 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x200 ), "region 2 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2FF ), "region 2 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x300 ), "region 3 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3FF ), "region 3 - new" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x400 ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x0FF, "region 0") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x100, 0x1FF, "region 1") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x200, 0x2FF, "region 2") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x300, 0x3FF, "region 3") ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x000 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0FF ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1FF ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x200 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2FF ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x300 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3FF ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x400 ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x060, 0x0AF, "region 0 - inside") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x160, 0x1AF, "region 1 - inside") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x260, 0x2AF, "region 2 - inside") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x360, 0x3AF, "region 3 - inside") ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x000 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x059 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x060 ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0AF ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0B0 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0FF ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x159 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x160 ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1AF ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1B0 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1FF ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x200 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x259 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x260 ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2AF ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2B0 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2FF ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x300 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x359 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x360 ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3AF ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3B0 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3FF ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x400 ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x02F, "region 0 - left") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x100, 0x12F, "region 1 - left") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x200, 0x22F, "region 2 - left") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x300, 0x32F, "region 3 - left") ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x000 ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x02F ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x030 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x12F ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x130 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x200 ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x22F ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x230 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x300 ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x32F ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x330 ), "region 3" ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x0D0, 0x0FF, "region 0 - right") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x1D0, 0x1FF, "region 1 - right") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x2D0, 0x2FF, "region 2 - right") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x3D0, 0x3FF, "region 3 - right") ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x000 ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x010 ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x02f ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x030 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x040 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x05f ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x060 ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x070 ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0af ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0b0 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0c0 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0cf ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0d0 ), "region 0 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0e0 ), "region 0 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0ff ), "region 0 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x110 ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x12f ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x130 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x140 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x15f ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x160 ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x170 ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1af ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1b0 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1c0 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1cf ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1d0 ), "region 1 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1e0 ), "region 1 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1ff ), "region 1 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x200 ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x210 ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x22f ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x230 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x240 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x25f ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x260 ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x270 ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2af ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2b0 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2c0 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2cf ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2d0 ), "region 2 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2e0 ), "region 2 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x2ff ), "region 2 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x300 ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x310 ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x32f ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x330 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x340 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x35f ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x360 ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x370 ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3af ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3b0 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3c0 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3cf ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3d0 ), "region 3 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3e0 ), "region 3 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3ff ), "region 3 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x400 ), "replaced - all" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   BOOST_CHECK( validate_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
   //print_iomem_regions() ;

   BOOST_CHECK_NO_THROW( release_iomem() ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x7ffffffffff ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
} // test_iomem_add


BOOST_FIXTURE_TEST_CASE( test_iomem_add_bad, TestIomemFixture ) {
   BOOST_CHECK_FAIL( add_iomem_region( 0x001, 0x000, "Map a region where start > end at the extreme" ) ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x80000000000, 0x7ffffffffff, "Map a region where start > end" ) ) ;

   BOOST_CHECK_FAIL( add_iomem_region( MAX_PHYS_ADDR, MAX_PHYS_ADDR-1, "Map a region where start > end at the extreme" ) ) ;
   BOOST_CHECK_FAIL( add_iomem_region( MAX_PHYS_ADDR, 0x00000000000, "Map a region that are opposite to the extreme" ) ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x001, NULL ) ) ;  // Map a region with a NULL description
   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x001, "" ) ) ;  // Map a region with an empty description
}


BOOST_FIXTURE_TEST_CASE( test_iomem_add_description, TestIomemFixture ) {
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x001, "  A  " ) ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00 ), "  A  " ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x001, "  A  B  " ) ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00 ), "  A  B  " ) ;

   BOOST_CHECK_NO_THROW( release_iomem() ) ;

   // Test regions that overlay on top of each other
   for( int i = MAX_IOMEM_DESCRIPTION - 3 ; i <= MAX_IOMEM_DESCRIPTION + 3 ; i++ ) {
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x010, 0x020, "C" ) ) ;
      std::string referenceDesc( (i < MAX_IOMEM_DESCRIPTION - 1) ? i : (MAX_IOMEM_DESCRIPTION - 1), 'x' ) ;
      std::string largeDescription( i, 'x' ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x010, 0x020, largeDescription.c_str() ) ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x018 ), referenceDesc ) ;
      BOOST_CHECK( validate_iomem() ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      BOOST_CHECK( validate_summary() ) ;
      //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
      // print_iomem_regions() ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
   }

   // Test new region anchored to the left
   for( int i = MAX_IOMEM_DESCRIPTION - 3 ; i <= MAX_IOMEM_DESCRIPTION + 3 ; i++ ) {
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x010, 0x020, "D" ) ) ;
      std::string referenceDesc( (i < MAX_IOMEM_DESCRIPTION - 1) ? i : (MAX_IOMEM_DESCRIPTION - 1), 'x' ) ;
      std::string largeDescription( i, 'x' ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x010, 0x012, largeDescription.c_str() ) ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x011 ), referenceDesc ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x014 ), "D" ) ;
      BOOST_CHECK( validate_iomem() ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      BOOST_CHECK( validate_summary() ) ;
      //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
      // print_iomem_regions() ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
   }

   // Test new region anchored to the right
   for( int i = MAX_IOMEM_DESCRIPTION - 3 ; i <= MAX_IOMEM_DESCRIPTION + 3 ; i++ ) {
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x010, 0x020, "E" ) ) ;
      std::string referenceDesc( (i < MAX_IOMEM_DESCRIPTION - 1) ? i : (MAX_IOMEM_DESCRIPTION - 1), 'x' ) ;
      std::string largeDescription( i, 'x' ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x01D, 0x020, largeDescription.c_str() ) ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x01C ), "E" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x01E ), referenceDesc ) ;
      BOOST_CHECK( validate_iomem() ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      BOOST_CHECK( validate_summary() ) ;
      //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
      // print_iomem_regions() ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
   }

   // Test new region created in the middle
   for( int i = MAX_IOMEM_DESCRIPTION - 3 ; i <= MAX_IOMEM_DESCRIPTION + 3 ; i++ ) {
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x010, 0x020, "F" ) ) ;
      std::string referenceDesc( (i < MAX_IOMEM_DESCRIPTION - 1) ? i : (MAX_IOMEM_DESCRIPTION - 1), 'x' ) ;
      std::string largeDescription( i, 'x' ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x017, 0x019, largeDescription.c_str() ) ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x016 ), "F" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x018 ), referenceDesc ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x01A ), "F" ) ;
      BOOST_CHECK( validate_iomem() ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      BOOST_CHECK( validate_summary() ) ;
      //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
      // print_iomem_regions() ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
   }
} // test_iomem_add_description


BOOST_FIXTURE_TEST_CASE( test_iomem_add_overlap, TestIomemFixture ) {
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x100, 0x1FF, "region 1 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x300, 0x3FF, "region 3 - new") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x00000000000, MAX_PHYS_ADDR, "jump regions - extreme") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x0FF, 0x400, "jump regions - tight") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x0FF, 0x200, "jump region 1 - tight") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x2FF, 0x400, "jump region 3 - tight") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x100, "overlap region 1 - left edge - extreme left") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x0FF, 0x100, "overlap region 1 - left edge - tight") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x0FF, 0x101, "overlap region 1 - left edge - tight+1") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x07F, 0x17F, "overlap region 1 - left middle") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x0FF, MAX_PHYS_ADDR, "overlap region 1 - left edge - extreme right") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x1FF, 0x2FF, "overlap region 1 - right edge - extreme left") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x1FF, 0x200, "overlap region 1 - right edge - tight") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x1FF, 0x201, "overlap region 1 - right edge - tight+1") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x17F, 0x27F, "overlap region 1 - right middle") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x1FF, MAX_PHYS_ADDR, "overlap region 1 - right edge - extreme right") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x200, 0x300, "overlap region 3 - left edge - extreme left") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x2FF, 0x300, "overlap region 3 - left edge - tight") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x2FF, 0x301, "overlap region 3 - left edge - tight+1") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x27F, 0x37F, "overlap region 3 - left middle") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x2FF, MAX_PHYS_ADDR, "overlap region 3 - left  - extreme right") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x3FF, MAX_PHYS_ADDR, "overlap region 3 - right edge - extreme left") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x3FF, 0x400, "overlap region 3 - right edge - tight") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x3FF, 0x401, "overlap region 3 - right edge - tight+1") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x37F, 0x47F, "overlap region 3 - right middle") ) ;

   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   BOOST_CHECK( validate_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
   // print_iomem_regions() ;
}


BOOST_FIXTURE_TEST_CASE( test_iomem_add_one_byte_regions, TestIomemFixture ) {
   for( size_t i = 0x0000 ; i <= 0xF000 ; i += 0x1000 ) {
      // Update the first byte (as a boundry condition)
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x000 + i, 0x000 + i, "One byte") ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x000 + i ), "One byte" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x001 + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;

      // Split a region into 2
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x100 + i, 0x100 + i, "One byte") ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0FF + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 + i ), "One byte" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x101 + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK( validate_iomem() ) ;

      // Replace a one-byte region
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x100 + i, 0x100 + i, "Still One byte") ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x0FF + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x100 + i ), "Still One byte" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x101 + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK( validate_iomem() ) ;

      // Insert a one-byte region on the left of an existing region
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x200 + i, 0x300 + i, "A region") ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x200 + i, 0x200 + i, "One byte on left") ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x1FF + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x200 + i ), "One byte on left" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x201 + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x300 + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x301 + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK( validate_iomem() ) ;

      // Insert a one-byte region on the right of an existing region
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x400 + i, 0x500 + i, "A region") ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x500 + i, 0x500 + i, "One byte on right") ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x3FF + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x400 + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x4FF + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x500 + i ), "One byte on right" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x501 + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK( validate_iomem() ) ;

      // Insert a one-byte region in the middle of an existing region (just to be sure)
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x600 + i, 0x700 + i, "A region") ) ;
      BOOST_CHECK_NO_THROW( add_iomem_region( 0x680 + i, 0x680 + i, "One byte in the middle") ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x5FF + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x600 + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x67F + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x680 + i ), "One byte in the middle" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x681 + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x700 + i ), "A region" ) ;
      BOOST_CHECK_EQUAL( get_iomem_region_description( 0x701 + i ), UNMAPPED_MEMORY_DESCRIPTION ) ;
      BOOST_CHECK( validate_iomem() ) ;
   }
   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   BOOST_CHECK( validate_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
   // print_iomem_regions() ;
}

BOOST_FIXTURE_TEST_CASE( test_iomem_iomem6, TestIomemFixture ) {
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x00000000, 0x00000fff, "Reserved") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x00000000, 0x00000000, "PCI Bus 0000:00") ) ;
   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   BOOST_CHECK( validate_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
   // print_iomem_regions() ;
}

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
