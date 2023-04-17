///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Comprehensive test of the config module
///
/// @file   test_config.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#define BOOST_TEST_MODULE memscan
#include <boost/test/included/unit_test.hpp>  // include this to get main()
#include <boost/test/unit_test.hpp>

#include <stddef.h>     // For size_t
#include <stdio.h>      // For fopen() fclose()
#include <sys/types.h>  // For ssize_t

#include "boost_test_util.h"  // For BOOST_CHECK_FAIL()

extern "C" {
   #include "../config.h"
}

/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because these are white box tests, it's on
   the tester to ensure the code is in sync.                                 */

extern "C" bool hasScanOptions() ;
extern "C" bool hasOutputOptions() ;

/* ***************************************************************************/


// Ensure that reset_config() runs before we start testing.
struct MyGlobalFixture {
   static void setup() {
      BOOST_TEST_MESSAGE( "setup fixture" );
      reset_config() ;
   }
};
BOOST_TEST_GLOBAL_FIXTURE( MyGlobalFixture );


BOOST_AUTO_TEST_SUITE( test_config )

BOOST_AUTO_TEST_CASE( test_architecture ) {
   // Make sure pointers are the same size as `size_t`
   BOOST_CHECK_EQUAL( sizeof( void* ), sizeof( size_t ) ) ;

   BOOST_CHECK_EQUAL( sizeof( ssize_t ), sizeof( size_t ) ) ;
}


BOOST_AUTO_TEST_CASE( test_getEndianness ) {
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      BOOST_CHECK_EQUAL( getEndianness(), LITTLE ) ;
   #else
      BOOST_CHECK_EQUAL( getEndianness(), BIG ) ;
   #endif
}


BOOST_AUTO_TEST_CASE( test_printUsage ) {
   FILE* dev_null = fopen( "/dev/null", "w" ) ;
   BOOST_CHECK( dev_null != NULL ) ;
   BOOST_CHECK_FAIL( printUsage( NULL ) ) ;
   BOOST_CHECK_NO_THROW( printUsage( dev_null ) ) ;
   fclose( dev_null ) ;  // NOLINT(cert-err33-c): No need to check the return value of fclose()
}


BOOST_AUTO_TEST_CASE( test_ProgramName ) {
   BOOST_CHECK_EQUAL( getProgramName(), "memscan" ) ;

   BOOST_CHECK_NO_THROW( setProgramName( "Sam" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;

   BOOST_CHECK_FAIL( setProgramName( NULL ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK_FAIL( setProgramName( "" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK_FAIL( setProgramName( "  \t\t\n\n  " ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Sam" ) ;  // The name has not changed...

   BOOST_CHECK_NO_THROW( setProgramName( "  \t\tChili\n\n  " ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "Chili" ) ;  // The name was trimmed...

   BOOST_CHECK_NO_THROW( setProgramName( "123456789012345678901234567890" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "123456789012345678901234567890" ) ;  // 30-character names are OK

   BOOST_CHECK_NO_THROW( setProgramName( "1234567890123456789012345678901" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "1234567890123456789012345678901" ) ;  // 31-character names are OK

   BOOST_CHECK_NO_THROW( setProgramName( "12345678901234567890123456789012" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "1234567890123456789012345678901" ) ;  // 32-character names map to 31 characters

   BOOST_CHECK_NO_THROW( setProgramName( "123456789012345678901234567890123" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "1234567890123456789012345678901" ) ;  // 33-character names map to 31 characters

   BOOST_CHECK_NO_THROW( setProgramName( "memscan" ) ) ;
   BOOST_CHECK_EQUAL( getProgramName(), "memscan" ) ;
} // test_ProgramName


BOOST_AUTO_TEST_CASE( test_GET_BIT ) {
   BOOST_CHECK(  GET_BIT( 0b0001, 0 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1110, 0 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b0010, 1 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1101, 1 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b0100, 2 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b1011, 2 ) ) ;

   BOOST_CHECK(  GET_BIT( 0b1000, 3 ) ) ;
   BOOST_CHECK( !GET_BIT( 0b0111, 3 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x1000000000000000, 60 ) ) ;
   BOOST_CHECK( !GET_BIT( 0xefffffffffffffff, 60 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x2000000000000000, 61 ) ) ;
   BOOST_CHECK( !GET_BIT( 0xdfffffffffffffff, 61 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x4000000000000000, 62 ) ) ;
   BOOST_CHECK( !GET_BIT( 0xbfffffffffffffff, 62 ) ) ;

   BOOST_CHECK(  GET_BIT( 0x8000000000000000, 63 ) ) ;
   BOOST_CHECK( !GET_BIT( 0x7fffffffffffffff, 63 ) ) ;
} // test_GET_BIT


struct Configuration {
   Configuration() {
      BOOST_TEST_MESSAGE( "setup memscan configuration" ) ;
      reset_config() ;

      BOOST_CHECK_EQUAL( scanSelf, true ) ;
      BOOST_CHECK_EQUAL( hasScanOptions(), false ) ;
      BOOST_CHECK_EQUAL( hasOutputOptions(), false ) ;
      BOOST_CHECK_EQUAL( iomemSummary, false ) ;
      BOOST_CHECK_EQUAL( forkProcess, false ) ;

      BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
   }
   ~Configuration() {  // NOLINT( bugprone-exception-escape ): I'm accepting the risk of the destructor throwing an exception
      BOOST_TEST_MESSAGE( "teardown memscan configuration" ) ;
      reset_config() ;
      BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
   }
} ;

#define MAKE_ARGS() std::vector<char*> argv;                        \
                    for( const auto& arg : arguments )              \
                       argv.push_back((char*)arg.data());  /* NOLINT(performance-inefficient-vector-operation) */  \
                    argv.push_back( nullptr ) ;                     \
                    BOOST_CHECK_NO_THROW( processOptions( arguments.size(), argv.data() ) )

BOOST_FIXTURE_TEST_CASE( test_config_block, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--block=/etc/passwd" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( blockPath, "/etc/passwd" ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_stream, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--stream=/etc/group" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( streamPath, "/etc/group" ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_map_file, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--map_file=/etc/shadow" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( mapFilePath, "/etc/shadow" ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_read, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--read" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( readFileContents, true ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_local, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--local=500k" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( localSize, 500000 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_numLocal, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--numLocal=12" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( numLocals, 12 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_malloc, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--malloc=500M" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( mallocSize, 524288000 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_num_malloc, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--numMalloc=0xff" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( numMallocs, 255 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_map_mem, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--map_mem=40G" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( mappedSize, 42949672960 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_map_addr, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--map_addr=0x80112233" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( mappedStart, (void*) 0x80112233 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_fill, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--fill" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( fillAllocatedMemory, true ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_threads, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--threads=0b1000" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( numThreads, 8 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_sleep, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--sleep=60" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( sleepSeconds, 60 ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_scan_byte, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--scan_byte" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( scanForByte, true ) ;
   BOOST_CHECK_EQUAL( byteToScanFor, 0xc3 ) ;

   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_scan_byte_with_value, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--scan_byte=0xee" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( scanForByte, true ) ;
   BOOST_CHECK_EQUAL( byteToScanFor, 0xee ) ;

   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_shannon, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--shannon" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( scanForShannon, true ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_path, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--path" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( printPath, true ) ;
   BOOST_CHECK_EQUAL( hasOutputOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_pfn, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--pfn" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( includePhysicalPageNumber, true ) ;
   BOOST_CHECK_EQUAL( hasOutputOptions(), true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_phys, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--phys" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( includePhysicalPageSummary, true ) ;
   BOOST_CHECK_EQUAL( hasOutputOptions(), true ) ;
   BOOST_CHECK_EQUAL( scanSelf, true ) ;

   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_phys_pfn, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--pfn", "--phys" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_iomem_pfn, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--iomem", "--phys" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_block_iomem, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--block=/bin/cat", "--iomem" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_pid, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--pid=111" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( scanSelf, false ) ;
   BOOST_CHECK_EQUAL( scanPid, 111 ) ;
   BOOST_CHECK_EQUAL( mapsFilePath, "/proc/111/maps" ) ;
   BOOST_CHECK_EQUAL( pagemapFilePath, "/proc/111/pagemap" ) ;
   BOOST_CHECK_EQUAL( hasScanOptions(), false ) ;
   BOOST_CHECK_EQUAL( hasOutputOptions(), false ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_pid_iomem, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--pid=222", "--iomem" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_pid_scan_byte, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--pid=222", "--scan_byte" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_fork, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--fork" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( forkProcess, true ) ;
   BOOST_CHECK_EQUAL( validateConfig( false ), true ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_fork_scan_byte, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--fork", "--scan_byte" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_fork_pid, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--pid=222", "--fork" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_FIXTURE_TEST_CASE( test_config_fork_iomem, Configuration ) {
   std::vector<std::string> arguments = { "memscan", "--fork", "--iomem" } ;
   MAKE_ARGS() ;

   BOOST_CHECK_EQUAL( validateConfig( false ), false ) ;
}


BOOST_AUTO_TEST_SUITE_END()
/// @endcond
