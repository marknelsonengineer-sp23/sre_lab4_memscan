///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Comprehensive test of pagemap, pageflags and pagecount
///
/// @file   test_pagemap.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../pagecount.h"

BOOST_AUTO_TEST_SUITE( test_pagemap )

   BOOST_AUTO_TEST_CASE( test_pagecount ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGECOUNT_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pagecount_t ), PAGECOUNT_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGECOUNT_ENTRY ) ;
   }

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
