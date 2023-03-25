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

/// Get physical frame flags from #PAGEFLAG_FILE and put them into #PageInfo
///
/// @param page The page to update
extern void getPageflags( struct PageInfo* page ) ;


/// Close any open pageflags resources
extern void closePageflags() ;
