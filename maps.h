///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Read, parse and process the `/proc/$PID/maps` file
///
/// @file   maps.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Parse each line from #MEMORY_MAP_FILE and put the data into a #MapEntry
/// field.  This function makes heavy use of [strtok()][1].
///
/// Anything that changes #MEMORY_MAP_FILE should be done before calling
/// getMaps()
///
/// [1]: https://man7.org/linux/man-pages/man3/strtok_r.3.html
extern void getMaps() ;

/// Scan all readable memory regions.
///
/// Anything that changes the physical pagemap information should be done
/// before calling scanMaps()
extern void scanMaps() ;

/// Read data from `pagemap`, `pageflags` and `pagecount` and get information
/// about the physical pages referenced in `maps`
extern void readPagemapInfo() ;

/// Print the map, results of the scan and physical page info
extern void printMaps() ;
