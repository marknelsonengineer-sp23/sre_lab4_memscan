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


/// Get a description of the iomem region at `physAddr`
///
/// @param physAddr The physical address
/// @return         A description of the region at `physAddr`
extern const char* get_iomem_region_description( const void* physAddr ) ;


/// Read `/proc/iomem` to characterize physical addresses
///
/// @return `true` if successful.  `false` if there was a problem.
extern bool read_iomem() ;


/// Print a sorted summary of `/proc/iomem` listing the number of bytes in each
/// region.
///
/// ````
/// Summary of /proc/iomem
/// ahci                                         8,192
/// Video ROM                                   32,768
/// ACPI Tables                                 65,536
/// System ROM                                  65,536
/// e1000                                      131,072
/// Kernel data                              2,184,448
/// Kernel code                             16,784,908
/// Reserved                                20,357,120
/// System RAM                           2,096,520,436
/// Unmapped memory                281,470,711,262,208
/// ````
extern void summarize_iomem() ;
