///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Compute the Shannon Entropy of various buffers
///
/// @see https://en.wikipedia.org/wiki/Entropy_(information_theory)
/// @see https://rosettacode.org/wiki/Entropy#C
///
/// @file   shannon.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// The maximum length of a Shannon Entropy classification string
#define MAX_SHANNON_CLASSIFICATION_LENGTH 24


/// Compute the Shannon Entropy for `buffer`
///
/// @see https://en.wikipedia.org/wiki/Entropy_(information_theory)
/// @see https://rosettacode.org/wiki/Entropy#C
///
/// @param buffer  The buffer to scan
/// @param length  The length of the buffer
/// @return The Shannon Entropy (from 0.0 to 8.0 )
extern double computeShannonEntropy( const void* buffer, const size_t length ) ;


/// Lookup `entropy` and make a guess (classify it) as to what the data represents
///
/// @param entropy The Shannon Entropy of a buffer
/// @return A string a la `English text in UNICODE`, `x86 code`, et al.
extern char* getShannonClassification( const double entropy ) ;
