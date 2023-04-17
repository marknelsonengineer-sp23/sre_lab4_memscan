///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Comprehensive test of the assembly module
///
/// @file   test_assembly.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#include <boost/test/unit_test.hpp>

extern "C" {
   #include "../assembly.h"
}

BOOST_AUTO_TEST_SUITE( test_assembly )

   BOOST_AUTO_TEST_CASE( test_getBaseOfStack ) {
      void* base0 = NULL ;
      BOOST_CHECK( base0 == NULL ) ;
      GET_BASE_OF_STACK( base0 ) ;
      BOOST_CHECK( base0 != NULL ) ;

      void* base1 = NULL ;
      BOOST_CHECK( base1 == NULL ) ;
      GET_BASE_OF_STACK( base1 ) ;
      BOOST_CHECK( base1 != NULL ) ;

      BOOST_CHECK_EQUAL( base0, base1 ) ;
   }

   BOOST_AUTO_TEST_CASE( test_allocateLocalStorage ) {
      for( size_t i = 0 ; i < 128 ; i++ ) {
         void* base0 = NULL ;
         GET_BASE_OF_STACK( base0 ) ;
         BOOST_CHECK( base0 != NULL ) ;

         ALLOCATE_LOCAL_STORAGE( i ) ;

         void* base1 = NULL ;
         GET_BASE_OF_STACK( base1 ) ;
         BOOST_CHECK( base1 != NULL ) ;

         BOOST_CHECK_EQUAL( (uint8_t*)base0 - i, base1 ) ;
      }
   }

BOOST_AUTO_TEST_SUITE_END()

/// @endcond
