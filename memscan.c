///////////////////////////////////////////////////////////////////////////////
/// University of Hawaii, College of Engineering
/// EE 491  - Software Reverse Engineering
/// Lab 04a - Memory Scanner
///
/// @file memscan.c
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @brief  Lab 04a - Memory Scanner - EE 491F - Spr 2021
/// @date   2 Feb 21
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/// Each row in /proc/$PID/maps describes a region of contiguous 
/// virtual memory in a process or thread.  Where $PID is 'self'
/// the file is the memory map for the current process.

/// For example:
///
/// address                  perms offset   dev   inode      pathname
/// 00403000-00404000         r--p 00002000 00:30 641        /mnt/src/Src/tmp/t
/// 00404000-00405000         rw-p 00003000 00:30 641        /mnt/src/Src/tmp/t
/// 00405000-00587000         rw-p 00000000 00:00 0 
/// 00cc3000-00ce4000         rw-p 00000000 00:00 0          [heap]
/// 7f8670628000-7f867062a000 rw-p 00000000 00:00 0 
/// 7f867062a000-7f8670650000 r--p 00000000 00:1f 172006     /usr/lib64/libc-2.32.so
/// 7f8670650000-7f867079f000 r-xp 00026000 00:1f 172006     /usr/lib64/libc-2.32.so
///
/// Each row has the following fields:
///   - address:      The starting and ending address of the region in the 
///                   process's address space
///   - permissions:  This describes how pages in the region can be accessed. 
///                   There are four different permissions: read, write, 
///                   execute, and shared. If read/write/execute are disabled, 
///                   a - will appear instead of the r/w/x. If a region is not 
///                   shared, it is private, so a p will appear instead of an s. 
///                   If the process attempts to access memory in a way that is 
///                   not permitted, a segmentation fault is generated. 
///                   Permissions can be changed using the mprotect system call.
///   - offset:       If the region was mapped from a file (using mmap), this 
///                   is the offset in the file where the mapping begins. 
///                   If the memory was not mapped from a file, it's just 0.
///   - device:       If the region was mapped from a file, this is the major 
///                   and minor device number (in hex) where the file lives.
///   - inode:        If the region was mapped from a file, this is the file number.
///   - pathname:     If the region was mapped from a file, this is the name 
///                   of the file. This field is blank for anonymous mapped 
///                   regions. There are also special regions with names like 
///                   [heap], [stack], or [vdso]. [vdso] stands for virtual 
///                   dynamic shared object. It's used by system calls to 
///                   switch to kernel mode. 
#define MEMORY_MAP_FILE "/proc/self/maps"


/// Global file handle to MEMORY_MAP_FILE
FILE* file = NULL;


/// Permissions are 4 characters in a row:  read, write, execute and shared
struct Permission {
   char readPerm;
   char writePerm;
   char executePerm;
   char sharedPerm;
};


/// Map the permissions onto the sPermissions string
union PermissionUnion {
   char sPermissions[1024];
   struct Permission permissions;   
};


/// Holds the original (and some processed data) from each map entry
struct MapEntry {
   char sAddresses[1024];  ///< String buffer for the original address range
   char* sAddressStart;    ///< String pointer to the start of the address range
   char* sAddressEnd;      ///< String pointer to the end of the address range
   void* pAddressStart;    ///< Pointer to the start of the memory mapped region
   void* pAddressEnd;      ///< Pointer to the byte just *after* the end of the memory mapped region
   union PermissionUnion uPerms;  ///< Permissions to the region
   char sOffset[1024];     ///< String buffer for the offset
   char sDevice[1024];     ///< String buffer for the device name
   char sInode[1024];      ///< String buffer for the iNode number
   char sPath[1024];       ///< String buffer for the path
};

/// The maximum number of MapEntry records in map
#define MAX_ENTRIES (256)

/// Holds up to MAX_ENTRIES MapEntry structs 
struct MapEntry map[256];   /// @todo Fix hardcoded value

/// The number of entries in map
int numMaps = 0;


/// Process the buffers in map... converting strings to pointers
void processEntries() {
   for ( int i = 0 ; i < numMaps ; i++ ) {
      map[i].sAddressStart = map[i].sAddresses;
      
      /// At this point, sAddress looks like this:  "00dfe000-00e1f000"
      ///   - Set the sAddressStart to the first byte
      ///   - Change the - to a string null terminator
      ///   - Set sAddressEnd to point to the first byte of the ending address
      for (int j = 0 ; j < strlen(map[i].sAddresses) ; j++) {
         if (map[i].sAddresses[j] == '-') {
            map[i].sAddressEnd = map[i].sAddresses + j + 1;
            map[i].sAddresses[j] = 0;  // Change the - to a string null-terminator
         }
      }
      
      sscanf( map[i].sAddressStart, "%p", &(map[i].pAddressStart) );
      sscanf( map[i].sAddressEnd,   "%p", &(map[i].pAddressEnd) );
   }
}


/// Print an entry (print the processed/pointer) values 
void printEntry (int i) {
   printf ("%d:  0x%p - 0x%p  %c%c%c%c  %s  %s  %s  %s\n", 
           i
          ,map[i].pAddressStart
          ,map[i].pAddressEnd
          ,map[i].uPerms.permissions.readPerm
          ,map[i].uPerms.permissions.writePerm
          ,map[i].uPerms.permissions.executePerm
          ,map[i].uPerms.permissions.sharedPerm
          ,map[i].sOffset
          ,map[i].sDevice
          ,map[i].sInode
          ,map[i].sPath 
          );
}


/// This is the workhorse of this program... Scan all readable memory
/// regions, counting the number of bytes scanned and the number of 
/// times the letter 'A' appears in the region...
void scanEntries() {
   int c = 0;
   int a = 0;
   for ( int i = 0 ; i < numMaps ; i++ ) {

      printEntry ( i );
      
      /// Skip non-readable regions
      if (map[i].uPerms.permissions.readPerm != 'r' ) {
         continue;
      }

      for (void* j = map[i].pAddressStart ; j < map[i].pAddressEnd ; j++ ) {
         // printf ("0x%p < 0x%p\n ", j, map[i].pAddressEnd );
         if (*(char*)j == 'A') {
            a++;
         } 
         c++;
      }
      printf ("Number of bytes read [%d]   Number of 'A' is [%d]\n", c, a);
      c = 0;
      a = 0;
   } 
}


/// Print the contents of the map array
void printEntries() {
   for ( int i = 0 ; i < numMaps ; i++ ) {
      printEntry( i );
   }
}


/// Return true if c is whitespace
bool isWhitespace (char c) {
   if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
      return true;

   return false;
}


/// Parse each line from MEMORY_MAP_FILE, mapping the data into
/// a MapEntry field.  
/// 
/// We will use a state machine to keep track if we are in a 
/// word or whitespace.  
///
/// We will use another state machine to populate the MapEntry
/// buffers from left-to-right.
///
void readEntries() {
   char buffer[1024];
   memset( buffer, 0, sizeof(buffer) );
   int offset = 0;
   enum Mode { UNKNOWN, IN_WORD, IN_WHITESPACE } mode = UNKNOWN;
   
   enum Item { ADDRESS, PERM, OFFSET, DEVICE, INODE, PATH } item = ADDRESS;
   
   /// This loop reads the file character-by-character until EOF (end of file)
   for (char c = fgetc( file ) ; c != EOF ; c = fgetc ( file )) {
      printf("%c", c);
   
      // Deal with the starting condition
      if (mode == UNKNOWN) {
         if ( isWhitespace (c) ) {
            mode = IN_WHITESPACE;
         } else {
            mode = IN_WORD;
         }
      }
      
      // If we are at the end of a word... it's time to process it
      if (mode == IN_WORD && isWhitespace (c) ) {
         switch (item) {
            case (ADDRESS): {
               strcpy( map[numMaps].sAddresses, buffer );
               break;
            }
            case (PERM): {
               strcpy( map[numMaps].uPerms.sPermissions, buffer );
               break;
            }
            case (OFFSET): {
               strcpy( map[numMaps].sOffset, buffer );
               break;
            }
            case (DEVICE): {
               strcpy( map[numMaps].sDevice, buffer );
               break;
            }
            case (INODE): {
               strcpy( map[numMaps].sInode, buffer );
               break;
            }
            case (PATH): {
               strcpy( map[numMaps].sPath, buffer );
               break;
            }
         }

         // For debugging
         // printf ("Word=[%s]  numMaps=[%d]  Item=[%d]\n", buffer, numMaps, item);

         item++;
         memset( buffer, 0, sizeof(buffer) );
         offset = 0;

         if (c == '\n') {
            numMaps++;
            item = ADDRESS;
         }

         mode = IN_WHITESPACE;
         continue;
      }

      // If reading a bunch of whitespace... just continue right through it
      if (mode == IN_WHITESPACE && isWhitespace (c) ) {
         if (c == '\n') {
            numMaps++;
            item = ADDRESS;
         }

         continue;
      }
      
      // If starting a new word... clear the buffer and collect the word
      if (mode == IN_WHITESPACE && !isWhitespace (c) ) {
         memset( buffer, 0, sizeof(buffer) );
         offset = 0;
         
         mode = IN_WORD;
         
         buffer[offset++] = c;
         continue;
      }
      
      // If in a word... and we are still in a word, just continue to collect the word
      if (mode == IN_WORD && !isWhitespace (c) ) {
         buffer[offset++] = c;
         continue;      
      }
      
   }
}


/// Memory scanner
int main( int argc, char* argv[] ) {
   printf ("Memory scanner\n");
   
   file = fopen ( MEMORY_MAP_FILE, "r" );
   
   if ( file == NULL ) {
      printf ("%s: Unable to open [%s].  Exiting.\n", argv[0], MEMORY_MAP_FILE );
      return EXIT_FAILURE;
   }
   
   readEntries();
   
   processEntries();
   
   scanEntries();
   
   printEntries();
 
   fclose (file);
   
	return EXIT_SUCCESS;
}
