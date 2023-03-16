///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Utilities to support BOOST Tests (specifically for C programs)
///
/// @file   boost_test_util.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Throw an exception
///
/// throwException() exists to support [Boost Test], specifically [BOOST_CHECK_THROW].
/// Memscan uses FATAL_ERROR() and ASSERT() for error checking.  Normally,
/// these functions will call `exit()`, however when `TESTING` is defined,
/// they will call throwException() so [BOOST_CHECK_THROW] can catch it.
///
/// Furthermore, when an unexpected call to FATAL_ERROR() or ASSERT() happens,
/// the [Boost Test] framework can catch the exception rather than immediately
/// `exit()`.
///
/// This utility function is a unique hybrid of C and C++ -- as C does not support
/// exceptions.
///
/// [Boost Test]:  https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html
/// [BOOST_CHECK_THROW]: https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/boost_test/utf_reference/testing_tool_ref/assertion_boost_level_throw.html
///
#ifdef __cplusplus
   extern "C" void throwException() ;
#else
   extern void throwException() ;
#endif


/// Ensure `expression` fails (throws `std::exception`)
///
/// throwException() always throws `std::exception`.  This helper macro is
/// simply a concise way to call `BOOST_CHECK_THROW( (expression), std::exception )`
#ifdef __cplusplus
   #define BOOST_CHECK_FAIL( expression ) \
      BOOST_CHECK_THROW( (expression), std::exception ) ;
#else
   #define BOOST_CHECK_FAIL( expression ) ;
#endif
