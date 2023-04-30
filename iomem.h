///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process `/proc/iomem` to characterize physical pages
///
/// See iomem.c for a detailed description.
///
/// @file   iomem.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "typedefs.h"  // For const_pfn_t PFN_MASK


/// The longest allowed description in #iomemFilePath
#define MAX_IOMEM_DESCRIPTION  64

/// The description attached to unmapped memory
#define UNMAPPED_MEMORY_DESCRIPTION "Unmapped memory"

/// The maximum physical address of #PFN_MASK
#define MAX_PHYS_ADDR PFN_MASK


/// Get a description of the physical memory region at `physAddr`
///
/// @param physAddr The physical address
/// @return         A description of the region at `physAddr`
extern const char* get_iomem_region_description( const_pfn_t physAddr ) ;


/// Read #iomemFilePath to characterize physical addresses
extern void read_iomem() ;


/// Print a sorted summary of #iomemFilePath listing the number of bytes in each
/// region.
///
///     Summary of /proc/iomem
///     ahci                                         8,192
///     Video ROM                                   32,768
///     ACPI Tables                                 65,536
///     System ROM                                  65,536
///     e1000                                      131,072
///     Kernel data                              2,184,448
///     Kernel code                             16,784,908
///     Reserved                                20,357,120
///     System RAM                           2,096,520,436
///     Unmapped memory                281,470,711,262,208
extern void summarize_iomem() ;


/// Release resources allocated by the `iomem` module
extern void release_iomem() ;
