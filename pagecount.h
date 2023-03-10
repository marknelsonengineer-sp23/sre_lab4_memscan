///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get the number of times each physical page is mapped from `kpagecount`
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
///
/// @file   pagecount.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>  // For uint64_t


/// Get a 64-bit count of the number of times each physical page is mapped
///
/// @param pfn The Page Frame Number
/// @return    The number of times the PFN is mapped
extern uint64_t getPagecount( void* pfn ) ;


/// Close any open pagecount resources
extern void closePagecount() ;
