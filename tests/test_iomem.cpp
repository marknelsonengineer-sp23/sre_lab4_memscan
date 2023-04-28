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

#include <filesystem>   // For std::filesystem

extern "C" {
   #include "../config.h"
   #include "../iomem.h"
}


/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because these are white box tests, it's on
   the tester to ensure the code is in sync.                                 */

extern "C" bool validate_iomem() ;
extern "C" void add_iomem_region( const_pfn_t start, const_pfn_t end, const char* description ) ;
extern "C" void compose_iomem_summary() ;
extern "C" void sort_iomem_summary() ;
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
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
      BOOST_CHECK( validate_iomem() ) ;
   }
} ;

BOOST_AUTO_TEST_SUITE( test_iomem )

BOOST_FIXTURE_TEST_CASE( test_read_iomem_bulk, TestIomemFixture ) {
    /// Read the contents of `./tests/test_iomem`, containing sample `iomem`
    /// files from a variety of Linux systems.
   const std::filesystem::path sandbox{"test_iomem"};

   for (auto const& dir_entry : std::filesystem::directory_iterator{sandbox}) {
      // std::cout << dir_entry.path() << '\n';
      strncpy( iomemFilePath, dir_entry.path().u8string().c_str(), sizeof( iomemFilePath ) ) ;
      BOOST_CHECK_NO_THROW( read_iomem() ) ;
      BOOST_CHECK_EQUAL( validate_iomem(), true ) ;
      BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
      // BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;
      BOOST_CHECK_NO_THROW( release_iomem() ) ;
   }
}


BOOST_FIXTURE_TEST_CASE( test_iomem_release, TestIomemFixture ) {
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;

   BOOST_CHECK_NO_THROW( release_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_NO_THROW( compose_iomem_summary() ) ;
   BOOST_CHECK_NO_THROW( sort_iomem_summary() ) ;
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
   //BOOST_CHECK_NO_THROW( print_iomem_summary() ) ;

   BOOST_CHECK_NO_THROW( release_iomem() ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00000000000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x7ffffffffff ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
} // test_iomem_add


BOOST_FIXTURE_TEST_CASE( test_iomem_add_bad, TestIomemFixture ) {
   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x000, "Map a region that ends at 0" ) ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x001, 0x000, "Map a region where start > end at the extreme" ) ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x80000000000, 0x7ffffffffff, "Map a region where start > end" ) ) ;

   BOOST_CHECK_FAIL( add_iomem_region( MAX_PHYS_ADDR, MAX_PHYS_ADDR-1, "Map a region where start > end at the extreme" ) ) ;
   BOOST_CHECK_FAIL( add_iomem_region( MAX_PHYS_ADDR, 0x00000000000, "Map a region that are opposite to the extreme" ) ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x001, NULL ) ) ;  // Map a region with a NULL description
   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x001, "" ) ) ;  // Map a region with an empty description
   // @TODO Add some 1 byte regions (they should fail)

   BOOST_CHECK( validate_iomem() ) ;
}

BOOST_FIXTURE_TEST_CASE( test_iomem_add_description, TestIomemFixture ) {
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x001, "  A  " ) ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00 ), "  A  " ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x001, "  A  B  " ) ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( 0x00 ), "  A  B  " ) ;
}

BOOST_FIXTURE_TEST_CASE( test_iomem_add_overlap, TestIomemFixture ) {
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x000, 0x0FF, "region 0 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x100, 0x1FF, "region 1 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x200, 0x2FF, "region 2 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( 0x300, 0x3FF, "region 3 - new") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( 0x000, 0x100, "overlap 1 - bad") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( 0x0FF, 0x101, "overlap 2 - bad") ) ;
   // @TODO Add some more overlapping test cases someday
   BOOST_CHECK( validate_iomem() ) ;
}

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
