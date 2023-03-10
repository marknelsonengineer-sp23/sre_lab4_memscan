///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get flags for each physical page from `kpageflags`
///
/// @file   pageflags.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "pagemap.h"  // For PageInfo

/// Set physical frame flags in #PageInfo
///
/// @param page The page to update
extern void setPageflags( struct PageInfo* page ) ;


/// Close any open pageflags resources
extern void closePageflags() ;
