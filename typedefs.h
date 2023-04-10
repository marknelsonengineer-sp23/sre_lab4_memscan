///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Consolidate all datatype definitions in one place.  In C, header files
/// *should* form Directional Acyclic Graphs (DAGs) but the preprocessor will
/// allow programmers to create cycles.  Good library definitions usually
/// stratify into 3 layers:
///   - Type declarations
///   - Type definitions (`class`, `struct`, etc.), forward declarations of
///     functions and variables
///   - Implementations
///
/// @file   typedefs.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

/// The type definition for PFNs (Page Frame Numbers)
///
/// Normally, it's bad form to typedef a pointer, but we aren't
/// defining a regular pointer in C, rather this is a distinct datatype
/// that happens to be a pointer.  Memscan will never dereference
/// this pointer.
typedef void* pfn_t ;

/// The type definition for a constant PFN (Page Frame Number)
typedef const pfn_t const_pfn_t ;


/// The datatype of the pagemap data in #pagemapFilePath
typedef uint64_t pagemap_t ;

/// The size of each #pagemap_t entry
#define PAGEMAP_ENTRY sizeof( pagemap_t )


/// The datatype of the pagecount in #PAGECOUNT_FILE
typedef uint64_t pagecount_t ;

/// The size of each #pagecount_t entry
#define PAGECOUNT_ENTRY sizeof( pagecount_t )


/// The datatype of the pageflag data in #PAGEFLAG_FILE
typedef uint64_t pageflags_t ;

/// The size of each #pageflags_t entry
#define PAGEFLAG_ENTRY sizeof( pageflags_t )
