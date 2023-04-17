Usage
=====

@brief Documents the command line parameters for memscan

````
Usage: memscan [PRE-SCAN OPTIONS]... [SCAN OPTIONS]... [OUTPUT OPTIONS]... [FILTER]... 
       memscan --pid=NUM [OUTPUT OPTIONS]... [FILTER]...
       memscan --iomem
       memscan-static

  When FILTER is present, only process memory regions that match:
    - If FILTER is a decimal number, match on the region's index
    - If FILTER is a hex number (address), match the region that has the address
    - If FILTER prefix is +, followed by r w or x, include regions with that
      permission set
  When no FILTERs are present, process all regions

memscan-static has the same usage as memscan.  The difference is that it's
built statically.

TARGETING OPTIONS
  --pid=NUM          Scan process ID (by default, it scans itself)
  --iomem            Print a summary of /proc/iomem and exit

PRE-SCAN OPTIONS
  --block=FILE       Open FILE using block I/O before the scan
  --stream=FILE      Open FILE using stream I/O before the scan
  --map_file=FILE    Open FILE using memory mapped I/O before the scan
  --read             Read the contents of the files
  --local=SIZE       Allocate SIZE bytes in local variables before the scan
  --numLocal=NUM     Number of local allocations
  --malloc=SIZE      Malloc SIZE bytes before the scan
  --numMalloc=NUM    Number of malloc'd allocations
  --map_mem=SIZE     Allocate SIZE bytes of memory via mmap before the scan
  --map_addr=ADDR    The starting address of the memory map; by default, the OS
                     will select an address
  --fill             Fill the local, malloc'd and/or mapped memory with data
                     before the scan
  --threads=NUM      Create NUM threads before the scan
  --sleep=NUM        Pause the primary thread for NUM seconds before scanning

SCAN OPTIONS
  --scan_byte[=NUM]  Scan for NUM (a byte from 0x00 to 0xff)
                     or c3 (the x86 RET instruction) by default
  --shannon          Compute Shannon Entropy for each mapped region
                     and physical page

OUTPUT OPTIONS
  --path             Print the path (if available)
  --pfn              Print each physical page number w/ flags
  --phys             Print a summary of the physical pages w/ flags
                     --pfn and --phys are exclusive

PROGRAM OPTIONS
  -h, --help         Display this help and exit
  -k, --key          Display key to the --pfn & --phys flags and exit
  -v, --version      Display memscan's version and exit

SIZE may end in K|M|G to scale by 1,024 or k|m|g to scale by 1,000
NUM and ADDR may be prefixed by 0x (for hex) or 0b (for binary)


````
