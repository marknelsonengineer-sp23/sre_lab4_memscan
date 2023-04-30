///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Comprehensive test of `pagemap`, `pageflags` and `pagecount`
///
/// We've developed a lot of code to test `pagemap`, `pageflags` and `pagecount`
/// data.  These tests are important as we want to understand and
/// characterize the data coming back from the Kernel.  As we run these tests
/// on more diverse systems, a test may fail and we can reason about why.
/// Without a rigorous test suite like this, we would not have visibility to
/// what the Kernel is doing with these flags.  In other words, we'd be accepting
/// the flags without testing them.
///
/// In writing these tests, it's important to stay focused on testing memscan's
/// code and not try to test `libc` or the Kernel.  We also leave testing
/// Shannon entropy to another test module as this is complex enough.
///
/// The end result is to write an expressive test like this:
///
///       BOOST_CHECK( validatePageInfo( &somePage, VALID, PRESENT, NOT_EXCLUSIVE, NEUTRAL_LRU, COUNT_IS_PLURAL, OTHERWISE_0 ) ) ;
///
/// ... where every value in PageInfo is considered.
///
/// @file   test_pagemap.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include <cstdarg>   // For va_list va_start, et. al.
#include <iostream>  // For cout endl

extern "C" {
   #include "../config.h"   // For getProgramName()
   #include "../pagemap.h"
}

using namespace std ;


/// The rule for each entry in `flagState[]`
enum FlagState {
    UNDEFINED    = 0     ///< No rule has been set yet
   ,FLAG_SET     = 128   ///< Rule:  validatePageInfo() will verify that the PageInfo flag is `true`
   ,FLAG_CLEAR   = 129   ///< Rule:  validatePageInfo() will verify that the PageInfo flag is `false`
   ,FLAG_NEUTRAL = 130   ///< Rule:  validatePageInfo() does not care if the PageInfo flag is `true` or `false`
} ;


/// Make the #PageInfoValidation enum rules (`HUGE`, `NOT_HUGE` and
/// `NEUTRAL_HUGE`) from a `flag` (`HUGE`)
///
/// @param flag The name of the flag (in UPPERCASE) from the PageInfo struct.
///             For example, `VALID`, `EXCLUSIVE`, `HUGE`, etc.
#define MAKE_ENUMS( flag )            \
    flag,  NOT_##flag,  NEUTRAL_##flag


/// Enum containing every rule for testing the PageInfo structure
///
/// PageInfo members that are **not** tested include:
///   - The `soft_dirty` flag as it will always be neutral.
///   - The swap flags `swap_type`, `swap_offset` as the pages should never be
///     swapped and it would also be very difficult to reliably predict what
///     these values would be.  We are testing the `swap` flag.
///   - `pfn` as any value is possible.
enum PageInfoValidation {
    VALID              ,NOT_VALID,     NEUTRAL_VALID
   ,MAKE_ENUMS( EXCLUSIVE )     ,MAKE_ENUMS( FILE_MAPPED )
   ,MAKE_ENUMS( SWAPPED )       ,MAKE_ENUMS( PRESENT )
   ,MAKE_ENUMS( LOCKED )        ,MAKE_ENUMS( ERROR )
   ,MAKE_ENUMS( REFERENCED )    ,MAKE_ENUMS( UPTODATE )
   ,MAKE_ENUMS( DIRTY )         ,MAKE_ENUMS( LRU )
   ,MAKE_ENUMS( ACTIVE )        ,MAKE_ENUMS( SLAB )
   ,MAKE_ENUMS( WRITEBACK )     ,MAKE_ENUMS( RECLAIM )
   ,MAKE_ENUMS( BUDDY )         ,MAKE_ENUMS( MMAP )
   ,MAKE_ENUMS( ANON )          ,MAKE_ENUMS( SWAPCACHE )
   ,MAKE_ENUMS( SWAPBACKED )    ,MAKE_ENUMS( COMP_HEAD )
   ,MAKE_ENUMS( COMP_TAIL )     ,MAKE_ENUMS( HUGE )
   ,MAKE_ENUMS( UNEVICTABLE )   ,MAKE_ENUMS( HWPOISON )
   ,MAKE_ENUMS( NOPAGE )        ,MAKE_ENUMS( KSM )
   ,MAKE_ENUMS( THP )           ,MAKE_ENUMS( BALLOON )
   ,MAKE_ENUMS( ZERO_PAGE )     ,MAKE_ENUMS( IDLE )
   ,MAKE_ENUMS( PGTABLE )
   ,COUNT_IS_0         ,COUNT_IS_1
   ,COUNT_IS_PLURAL    ,COUNT_NEUTRAL
   ,OTHERWISE_1       = FLAG_SET
   ,OTHERWISE_0       = FLAG_CLEAR
   ,OTHERWISE_NEUTRAL = FLAG_NEUTRAL
   ,VALIDATION_LAST
} ;


/// Return `true` if this is the last `rule` for validatePageInfo()
///
/// validatePageInfo() is a [variadic] function.  Normally, [variadic] functions
/// require a count, but to keep the parameter count low, I decided that we'd end
/// after we process an `OTHERWISE_` rule.
///
/// @param rule The validation rule to examine
/// @return `true` if we see an `OTHERWISE_` rule.  `false` if we
///         should keep processing [variadic] parameters.
///
/// [variadic]: https://en.cppreference.com/w/cpp/utility/variadic
bool lastArg( enum PageInfoValidation rule ) {
   switch( rule ) {
      case OTHERWISE_1:
      case OTHERWISE_0:
      case OTHERWISE_NEUTRAL:
         return true ;
      default:
         return false ;
   }
}


/// Make the setState() `case:` statements that collect the rules
///
/// @param flag Use this `flag` to build three rules
#define MAKE_STATE( flag )                                                  \
   case flag:             flagState[ flag ]       = FLAG_SET     ; break ;  \
   case NOT_##flag:       flagState[ flag ]       = FLAG_CLEAR   ; break ;  \
   case NEUTRAL_##flag:   flagState[ flag ]       = FLAG_NEUTRAL ; break


/// Interpret `rule` and set the appropriate value in `flagState[]`
///
///   - If `rule == BUDDY`, set `flagState[ BUDDY ] to FLAG_SET`
///   - If `rule == NOT_BUDDY`, set `flagState[ BUDDY ] to FLAG_CLEAR`
///   - If `rule == NEUTRAL_BUDDY`, set `flagState[ BUDDY ] to FLAG_NEUTRAL`
///
/// When an `OTHERWISE_` argument is processed, then iterate over `flagState[]`
/// and set all `UNDEFINED` flags to `rule`.  At this point, every flagState
/// has a rule... either `FLAG_SET`, `FLAG_CLEAR` or `FLAG_NEUTRAL`.
///
/// One of the four `COUNT_` flags should be set and the other three should
/// be clear.
///
/// @param rule A [variadic] argument passed to validatePageInfo()
/// @param flagState An array holding the instructions on how to interpret
///                  the values in `struct PageInfo`
///
/// [variadic]: https://en.cppreference.com/w/cpp/utility/variadic
void setState( enum PageInfoValidation rule, enum FlagState flagState[] ) {
   switch( rule ) {
      MAKE_STATE( VALID ) ;
      MAKE_STATE( EXCLUSIVE ) ;     MAKE_STATE( FILE_MAPPED ) ;
      MAKE_STATE( SWAPPED ) ;       MAKE_STATE( PRESENT ) ;
      MAKE_STATE( LOCKED ) ;        MAKE_STATE( ERROR ) ;
      MAKE_STATE( REFERENCED ) ;    MAKE_STATE( UPTODATE ) ;
      MAKE_STATE( DIRTY ) ;         MAKE_STATE( LRU ) ;
      MAKE_STATE( ACTIVE ) ;        MAKE_STATE( SLAB ) ;
      MAKE_STATE( WRITEBACK ) ;     MAKE_STATE( RECLAIM ) ;
      MAKE_STATE( BUDDY ) ;         MAKE_STATE( MMAP ) ;
      MAKE_STATE( ANON ) ;          MAKE_STATE( SWAPCACHE ) ;
      MAKE_STATE( SWAPBACKED ) ;    MAKE_STATE( COMP_HEAD ) ;
      MAKE_STATE( COMP_TAIL ) ;     MAKE_STATE( HUGE ) ;
      MAKE_STATE( UNEVICTABLE ) ;   MAKE_STATE( HWPOISON ) ;
      MAKE_STATE( NOPAGE ) ;        MAKE_STATE( KSM ) ;
      MAKE_STATE( THP ) ;           MAKE_STATE( BALLOON ) ;
      MAKE_STATE( ZERO_PAGE ) ;     MAKE_STATE( IDLE ) ;
      MAKE_STATE( PGTABLE ) ;

      case COUNT_IS_0:      flagState[ COUNT_IS_0 ]      = FLAG_SET ; break ;
      case COUNT_IS_1:      flagState[ COUNT_IS_1 ]      = FLAG_SET ; break ;
      case COUNT_IS_PLURAL: flagState[ COUNT_IS_PLURAL ] = FLAG_SET ; break ;
      case COUNT_NEUTRAL:   flagState[ COUNT_NEUTRAL ]   = FLAG_SET ; break ;

      case OTHERWISE_1:
      case OTHERWISE_0:
      case OTHERWISE_NEUTRAL:
         for( int i = 0 ; i < VALIDATION_LAST ; i++ ) {
            if( ( i == COUNT_IS_0 || i == COUNT_IS_1 || i == COUNT_IS_PLURAL || i == COUNT_NEUTRAL ) && flagState[ i ] == UNDEFINED ) {
               flagState[ i ] = FLAG_CLEAR ;
            }
            if( flagState[ i ] == UNDEFINED ) {
               flagState[ i ] = (enum FlagState) rule ;
               // cout << "Setting flag " << i << " to " << arg << endl ;
            }
         }
         break ;
      default:
         assert( false ) ;
   }
}


/// Associates each `flag` with a rule and a PageInfo `.member` value -- validating
/// the rule against the value in the PageInfo member variable.
///
///   - If the rule is `FLAG_SET` and the member is `false`, then print a
///     message and return `false`
///   - If the rule is `FLAG_CLEAR` and the member is `true`, then print a
///     message and return `false`
///   - If we are neutral, then we don't care, so just return `true`
///
/// @param flag The flag to check:  `BUDDY`, `MMAP`, `ANON`, etc.
/// @param member The member variable in PageInfo:  `.buddy`, `.mmap`, `.anon`, etc.
#define CHECK_FLAG( flag, member )                                                             \
   /* cout << "flag = " #flag "   member = " #member << "   flagState[flag] = " << flagState[ flag ] << "   value = " << pPage->member << endl ; */  \
   if( flagState[ flag ] == FLAG_SET && ! pPage->member ) {                                    \
      cout << getProgramName() << ":  Flag " #flag " is clear when it shouldn't be"  << endl ; \
      return false ;                                                                           \
   }                                                                                           \
   if( flagState[ flag ] == FLAG_CLEAR && pPage->member )   {                                  \
      cout << getProgramName() << ":  Flag " #flag " is set when it shouldn't be"  << endl ;   \
      return false ;                                                                           \
   }


/// Test struct PageInfo populated by getPageInfo()
///
/// - Ensure the `virtualAddress` is in PageInfo
/// - Iterate over a [variadic] set of #PageInfoValidation parameters and set
///   rules in `flagState`
/// - Validate every flag in PageInfo
/// - Validate PageInfo.page_count
///
/// @param virtualAddress The address to compare against PageInfo.virtualAddress
/// @param pPage A pointer to a PageInfo structure that was populated by getPageInfo()
/// @param ... A [variadic] of #PageInfoValidation rules.  Callers must end with one
///            of the 3 `OTHERWISE_` enums
/// @return If there are any problems, return `false`.  If every validation
///         passes, return `true`
///
/// [variadic]: https://en.cppreference.com/w/cpp/utility/variadic
///
/// NOLINTNEXTLINE( cert-dcl50-cpp ):  Using a C-style [variadic]
bool validatePageInfo( void* virtualAddress, PageInfo* pPage, ... ) {

   size_t pageMask = ~( ( 1 << getPageSizeInBits() ) - 1 ) ;

   if( (size_t) pPage->virtualAddress != ((size_t) virtualAddress & pageMask ) ) {
      cout << getProgramName() << ":  Page address " << pPage->virtualAddress
           << " is not in the same page as " << virtualAddress
           << "  mask = " << hex << pageMask << dec << endl ;
      return false ;
   }

   va_list args ;
   va_start( args, pPage ) ;
   enum PageInfoValidation result ;
   enum FlagState flagState[ VALIDATION_LAST ] ;

   memset( flagState, 0, sizeof( flagState ) ) ;  // Set all flagStates to UNDEFINED

   do {
      result = (enum PageInfoValidation) va_arg( args, int ) ;
      setState( result, flagState ) ;
   } while( !lastArg( result ) ) ;
   va_end( args ) ;

   CHECK_FLAG( VALID, valid ) ;
   CHECK_FLAG( EXCLUSIVE, exclusive ) ;     CHECK_FLAG( FILE_MAPPED, file_mapped ) ;
   CHECK_FLAG( SWAPPED, swapped ) ;         CHECK_FLAG( PRESENT, present ) ;
   CHECK_FLAG( LOCKED, locked ) ;           CHECK_FLAG( ERROR, error ) ;
   CHECK_FLAG( REFERENCED, referenced ) ;   CHECK_FLAG( UPTODATE, uptodate ) ;
   CHECK_FLAG( DIRTY, dirty ) ;             CHECK_FLAG( LRU, lru ) ;
   CHECK_FLAG( ACTIVE, active ) ;           CHECK_FLAG( SLAB, slab ) ;
   CHECK_FLAG( WRITEBACK, writeback ) ;     CHECK_FLAG( RECLAIM, reclaim ) ;
   CHECK_FLAG( BUDDY, buddy ) ;             CHECK_FLAG( MMAP, mmap ) ;
   CHECK_FLAG( ANON, anon ) ;               CHECK_FLAG( SWAPCACHE, swapcache ) ;
   CHECK_FLAG( SWAPBACKED, swapbacked ) ;   CHECK_FLAG( COMP_HEAD, comp_head ) ;
   CHECK_FLAG( COMP_TAIL, comp_tail ) ;     CHECK_FLAG( HUGE, huge ) ;
   CHECK_FLAG( UNEVICTABLE, unevictable ) ; CHECK_FLAG( HWPOISON, hwpoison ) ;
   CHECK_FLAG( NOPAGE, nopage ) ;           CHECK_FLAG( KSM, ksm ) ;
   CHECK_FLAG( THP, thp ) ;                 CHECK_FLAG( BALLOON, balloon ) ;
   CHECK_FLAG( ZERO_PAGE, zero_page ) ;     CHECK_FLAG( IDLE, idle ) ;
   CHECK_FLAG( PGTABLE, pgtable ) ;

   if( flagState[ COUNT_IS_0 ] == FLAG_SET && pPage->page_count != 0 ) {
      cout << getProgramName() << ":  Count is " << pPage->page_count << " when it should be 0" << endl ;
      return false ;
   }

   if( flagState[ COUNT_IS_1 ] == FLAG_SET && pPage->page_count != 1 ) {
      cout << getProgramName() << ":  Count is " << pPage->page_count << " when it should be 1" << endl ;
      return false ;
   }

   if( flagState[ COUNT_IS_PLURAL ] == FLAG_SET && pPage->page_count <= 1 ) {
      cout << getProgramName() << ":  Count is " << pPage->page_count << " when it should be greater than 1" << endl ;
      return false ;
   }

   return true ;
}


/// @cond Suppress Doxygen warnings
BOOST_AUTO_TEST_SUITE( test_pagemap )

   /// This is an unusual test as it's not testing memscan.  Instead, it's exercising
   /// code in test_pagemap.cpp.
   BOOST_AUTO_TEST_CASE( test_lastArg ) {
      for( int i = 0 ; i < VALIDATION_LAST ; i++ ) {
         enum PageInfoValidation validation = (enum PageInfoValidation) i ;
         if( validation == OTHERWISE_1 || validation == OTHERWISE_0 || validation == OTHERWISE_NEUTRAL ) {
            BOOST_CHECK_EQUAL( lastArg( validation ), true ) ;
            continue ;
         }
         BOOST_CHECK_EQUAL( lastArg( validation ), false ) ;
      }
   }


   /// This is an unusual test as it's not testing memscan.  Instead, it's exercising
   /// code in test_pagemap.cpp.
   BOOST_AUTO_TEST_CASE( test_setState_VALID ) {
      enum FlagState flagState[ VALIDATION_LAST ] ;
      memset( flagState, 0, sizeof( flagState ) ) ;  // Set all flagStates to UNDEFINED

      setState( VALID, flagState ) ;
      for( int i = 0 ; i < VALIDATION_LAST ; i++ ) {
         if( i == VALID ) {
            BOOST_CHECK_EQUAL( flagState[ i ], FLAG_SET ) ;
         } else {
            BOOST_CHECK_EQUAL( flagState[ i ], UNDEFINED ) ;
         }
      }
   }


   /// This is an unusual test as it's not testing memscan.  Instead, it's exercising
   /// code in test_pagemap.cpp.
   BOOST_AUTO_TEST_CASE( test_setState_NOT_ACTIVE ) {
      enum FlagState flagState[ VALIDATION_LAST ] ;
      memset( flagState, 0, sizeof( flagState ) ) ;  // Set all flagStates to UNDEFINED

      setState( NOT_ACTIVE, flagState ) ;
      for( int i = 0 ; i < VALIDATION_LAST ; i++ ) {
         if( i == ACTIVE ) {
            BOOST_CHECK_EQUAL( flagState[ i ], FLAG_CLEAR ) ;
         } else {
            BOOST_CHECK_EQUAL( flagState[ i ], UNDEFINED ) ;
         }
      }
   }


   /// This is an unusual test as it's not testing memscan.  Instead, it's exercising
   /// code in test_pagemap.cpp.
   BOOST_AUTO_TEST_CASE( test_setState_NEUTRAL_PGTABLE ) {
      enum FlagState flagState[ VALIDATION_LAST ] ;
      memset( flagState, 0, sizeof( flagState ) ) ;  // Set all flagStates to UNDEFINED

      setState( NEUTRAL_PGTABLE, flagState ) ;
      for( int i = 0 ; i < VALIDATION_LAST ; i++ ) {
         if( i == PGTABLE ) {
            BOOST_CHECK_EQUAL( flagState[ i ], FLAG_NEUTRAL ) ;
         } else {
            BOOST_CHECK_EQUAL( flagState[ i ], UNDEFINED ) ;
         }
      }
   }


   /// This is an unusual test as it's not testing memscan.  Instead, it's exercising
   /// code in test_pagemap.cpp.
   BOOST_AUTO_TEST_CASE( test_setState_COUNT ) {
      struct PageInfo aPage ;
      memset( &aPage, 0, sizeof( struct PageInfo ) ) ;
      aPage.virtualAddress = NULL ;
      BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_0, OTHERWISE_0 ), true ) ;
      // BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_1, OTHERWISE_0 ), false ) ;
      // BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_PLURAL, OTHERWISE_0 ), false ) ;
      BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_NEUTRAL, OTHERWISE_0 ), true ) ;

      aPage.valid = aPage.exclusive = aPage.file_mapped = aPage.swapped = aPage.present
      = aPage.locked = aPage.error = aPage.referenced = aPage.uptodate = aPage.dirty = aPage.lru
      = aPage.active = aPage.slab = aPage.writeback = aPage.reclaim = aPage.buddy = aPage.mmap
      = aPage.anon = aPage.swapcache = aPage.swapbacked = aPage.comp_head = aPage.comp_tail
      = aPage.huge = aPage.unevictable = aPage.hwpoison = aPage.nopage = aPage.ksm
      = aPage.thp = aPage.balloon = aPage.zero_page = aPage.idle = aPage.pgtable
      = true ;
      aPage.page_count = 2 ;
      // BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_0, OTHERWISE_1 ), false ) ;
      // BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_1, OTHERWISE_1 ), false ) ;
      BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_PLURAL, OTHERWISE_1 ), true ) ;
      BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_NEUTRAL, OTHERWISE_1 ), true ) ;

      aPage.page_count = 1 ;
      // BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_0, OTHERWISE_1 ), false ) ;
      BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_1, OTHERWISE_1 ), true ) ;
      // BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_IS_PLURAL, OTHERWISE_1 ), false ) ;
      BOOST_CHECK_EQUAL( validatePageInfo( NULL, &aPage, COUNT_NEUTRAL, OTHERWISE_1 ), true ) ;
   }


   BOOST_AUTO_TEST_CASE( test_pagecount ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGECOUNT_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pagecount_t ), PAGECOUNT_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGECOUNT_ENTRY ) ;
   }


   BOOST_AUTO_TEST_CASE( test_pageflags ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGEFLAG_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pageflags_t ), PAGEFLAG_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGEFLAG_ENTRY ) ;
   }


   BOOST_AUTO_TEST_CASE( test_pagemap ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGEMAP_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pagemap_t ),   PAGEMAP_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGEMAP_ENTRY ) ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageSize ) {
      // Get a cue when we run across a non 4K page system
      BOOST_CHECK_EQUAL( getPageSizeInBytes(), 4096 ) ;    // Get into #IFDEFs if we find a nonstandard system
      BOOST_CHECK_EQUAL( getPageSizeInBits(), 12 ) ;

      BOOST_CHECK_EQUAL( getPageSizeInBytes(), 1 << getPageSizeInBits() ) ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageInfo_local ) {
      // aLocalInt is at the top of the stack and should be mapped into memory
      int aLocalInt = 0xff ;
      struct PageInfo stackPage = getPageInfo( &aLocalInt, true ) ;
      BOOST_CHECK( validatePageInfo( &aLocalInt, &stackPage, VALID, PRESENT, COUNT_IS_1, EXCLUSIVE, UPTODATE, LRU, MMAP, ANON, SWAPBACKED, OTHERWISE_0 ) ) ;

      // 4 pages up from the top of the stack should be unmapped
      void* topOfStackPlusFourPages = &aLocalInt + getPageSizeInBytes()*4 ;
      struct PageInfo stackPage4 = getPageInfo( topOfStackPlusFourPages, true ) ;
      BOOST_CHECK( validatePageInfo( topOfStackPlusFourPages, &stackPage4, VALID, NOT_PRESENT, COUNT_IS_0, OTHERWISE_0 ) ) ;
      closePagemap() ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageInfo_malloc ) {
      char* smallMalloc = (char*) malloc( 64 ) ;
      smallMalloc[ 0 ] = (char) 0xff ;  // Make sure it's paged into memory
      struct PageInfo smallMallocPage = getPageInfo( smallMalloc, true ) ;
      BOOST_CHECK( validatePageInfo( smallMalloc, &smallMallocPage, VALID, PRESENT, COUNT_IS_1, EXCLUSIVE, NEUTRAL_UPTODATE, NEUTRAL_LRU, NEUTRAL_ACTIVE, MMAP, ANON, SWAPBACKED, OTHERWISE_0 ) ) ;
      closePagemap() ;
      free( smallMalloc ) ;
      smallMalloc = NULL ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageInfo_readOnlyData ) {
      // This should come from libc's read-only section
      char* errorMessage = strerror( E2BIG ) ;
      char errorCharacter = errorMessage[0] ;  // Make sure it's paged into memory
      (void) errorCharacter ;  // Avoid an unused variable warning
      struct PageInfo errorMessagePage = getPageInfo( errorMessage, true ) ;
      BOOST_CHECK( validatePageInfo( errorMessage, &errorMessagePage, VALID, PRESENT, REFERENCED, FILE_MAPPED, COUNT_IS_PLURAL, NEUTRAL_UNEVICTABLE, NOT_EXCLUSIVE, UPTODATE, LRU, MMAP, OTHERWISE_0 ) ) ;
      closePagemap() ;
   }

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
