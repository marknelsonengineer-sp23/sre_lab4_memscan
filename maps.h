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

/// Parse each line from #MEMORY_MAP_FILE, mapping the data into
/// a MapEntry field.  This function makes heavy use of [strtok()][1].
///
/// [1]: https://man7.org/linux/man-pages/man3/strtok_r.3.html
extern void readEntries() ;

/// This is the workhorse of this program... Scan all readable memory
/// regions, counting the number of bytes scanned and the number of
/// times #CHAR_TO_SCAN_FOR appears in the region...
extern void scanEntries() ;
