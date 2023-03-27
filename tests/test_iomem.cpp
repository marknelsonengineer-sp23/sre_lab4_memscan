///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of iomem module
///
/// @file   test_iomem.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
   #include "../iomem.h"
}

/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because these are white box tests, it's on
   the tester to ensure the code is in sync.                                 */

extern "C" bool validate_iomem() ;
extern "C" void add_iomem_region( const void* start, const void* end, const char* description ) ;

/* ***************************************************************************/


BOOST_AUTO_TEST_SUITE( test_iomem )

BOOST_AUTO_TEST_CASE( test_iomem_release ) {
   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_NO_THROW( release_iomem() ) ;
   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_NO_THROW( release_iomem() ) ;
   BOOST_CHECK( validate_iomem() ) ;
}


BOOST_AUTO_TEST_CASE( test_iomem_add ) {
   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_NO_THROW( release_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*)         0x000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*)         0x100 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( 0, (void*) MAX_PHYS_ADDR, "replaced - all") ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*)         0x000 ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*)         0x100 ), "replaced - all" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x000, (void*) 0x0FF, "region 0 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x100, (void*) 0x1FF, "region 1 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x200, (void*) 0x2FF, "region 2 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x300, (void*) 0x3FF, "region 3 - new") ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x000, (void*) 0x0FF, "region 0") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x100, (void*) 0x1FF, "region 1") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x200, (void*) 0x2FF, "region 2") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x300, (void*) 0x3FF, "region 3") ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x060, (void*) 0x0AF, "region 0 - inside") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x160, (void*) 0x1AF, "region 1 - inside") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x260, (void*) 0x2AF, "region 2 - inside") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x360, (void*) 0x3AF, "region 3 - inside") ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x000, (void*) 0x02F, "region 0 - left") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x100, (void*) 0x12F, "region 1 - left") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x200, (void*) 0x22F, "region 2 - left") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x300, (void*) 0x32F, "region 3 - left") ) ;

   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x0D0, (void*) 0x0FF, "region 0 - right") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x1D0, (void*) 0x1FF, "region 1 - right") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x2D0, (void*) 0x2FF, "region 2 - right") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x3D0, (void*) 0x3FF, "region 3 - right") ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x000 ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x010 ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x02f ), "region 0 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x030 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x040 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x05f ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x060 ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x070 ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0af ), "region 0 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0b0 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0c0 ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0cf ), "region 0" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0d0 ), "region 0 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0e0 ), "region 0 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x0ff ), "region 0 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x100 ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x110 ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x12f ), "region 1 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x130 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x140 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x15f ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x160 ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x170 ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1af ), "region 1 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1b0 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1c0 ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1cf ), "region 1" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1d0 ), "region 1 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1e0 ), "region 1 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x1ff ), "region 1 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x200 ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x210 ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x22f ), "region 2 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x230 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x240 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x25f ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x260 ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x270 ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2af ), "region 2 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2b0 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2c0 ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2cf ), "region 2" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2d0 ), "region 2 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2e0 ), "region 2 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x2ff ), "region 2 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x300 ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x310 ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x32f ), "region 3 - left" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x330 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x340 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x35f ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x360 ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x370 ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3af ), "region 3 - inside" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3b0 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3c0 ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3cf ), "region 3" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3d0 ), "region 3 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3e0 ), "region 3 - right" ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) 0x3ff ), "region 3 - right" ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) MAX_PHYS_ADDR ), "replaced - all" ) ;

   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_NO_THROW( release_iomem() ) ;
   BOOST_CHECK( validate_iomem() ) ;

   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*)         0x000 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*)         0x100 ), UNMAPPED_MEMORY_DESCRIPTION ) ;
   BOOST_CHECK_EQUAL( get_iomem_region_description( (void*) MAX_PHYS_ADDR ), UNMAPPED_MEMORY_DESCRIPTION ) ;
} // test_iomem_add


BOOST_AUTO_TEST_CASE( test_iomem_add_bad ) {
   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_FAIL( add_iomem_region( (void*) 0x000, (void*) 0x000, "test" ) ) ;
   BOOST_CHECK_FAIL( add_iomem_region( (void*) 0x001, (void*) 0x000, "test" ) ) ;
   BOOST_CHECK_FAIL( add_iomem_region( (void*) 0x000, (void*) 0x001, NULL ) ) ;
   BOOST_CHECK_FAIL( add_iomem_region( (void*) 0x000, (void*) 0x001, "" ) ) ;
   // @TODO Add some 1 byte regions (they should fail)

   BOOST_CHECK( validate_iomem() ) ;
}

BOOST_AUTO_TEST_CASE( test_iomem_add_overlap ) {
   BOOST_CHECK( validate_iomem() ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x000, (void*) 0x0FF, "region 0 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x100, (void*) 0x1FF, "region 1 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x200, (void*) 0x2FF, "region 2 - new") ) ;
   BOOST_CHECK_NO_THROW( add_iomem_region( (void*) 0x300, (void*) 0x3FF, "region 3 - new") ) ;

   BOOST_CHECK_FAIL( add_iomem_region( (void*) 0x000, (void*) 0x100, "overlap 1 - bad") ) ;
   BOOST_CHECK_FAIL( add_iomem_region( (void*) 0x0FF, (void*) 0x101, "overlap 2 - bad") ) ;
   // @TODO Add some more overlapping test cases someday
   BOOST_CHECK( validate_iomem() ) ;
}

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
