///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process `/proc/iomem` to characterize physical pages
///
/// @file   iomem.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

/// The longest allowed description in #IOMEM_FILE
#define MAX_IOMEM_DESCRIPTION  64

/// The description attached to unmapped memory
#define UNMAPPED_MEMORY_DESCRIPTION "Unmapped memory"

/// The maximum physical address
///
/// Assumes a 48-bit physical address bus size (6 bytes).
#define MAX_PHYS_ADDR 0xffffffffffff


/// Print the iomem map.
///
/// Assumes a 48-bit physical address bus size (6 bytes or 12 characters wide).
extern void print_iomem() ;


/// Get the description of the iomem region at `physAddr`
///
/// @param physAddr The physical address
/// @return         A description of the region at `physAddr`
extern const char* get_iomem_region_description( const void* physAddr ) ;
