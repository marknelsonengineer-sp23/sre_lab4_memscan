///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Utilities to support BOOST Tests (specifically for C programs)
///
/// @file   boost_test_util.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdexcept>

#include "boost_test_util.h"

extern "C" void throwException() {
   throw std::exception() ;
}
