Usage
=====
````
Usage: memscan [PRE-SCAN OPTIONS]... [SCAN OPTIONS]... [OUTPUT OPTIONS]... [FILTER]... 
       memscan -i|--iomem

  When FILTER is present, only process sections that match a filter
  If FILTER is a decimal number, match it to the region's index
  If FILTER is a hex number, include regions with that virtual address
  If FILTER starts with + and r w or x then include sections with that permission
  When FILTER is not present, process all sections

The options below may be used to select memscan's operation

TARGETING OPTIONS
      --iomem              print a summary of /proc/iomem

PRE-SCAN OPTIONS
  -b, --block=FILE         open FILE using block I/O before the memscan
      --stream=FILE        open FILE using stream I/O before the memscan
      --map_file=FILE      open FILE using memory mapped I/O before the memscan
  -r, --read               read the contents of the files
  -l, --local=NUM[K|M|G]   allocate NUM bytes in local variables before the
                           memscan
      --numLocal=NUM       number of local allocations
  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan
      --numMalloc=NUM      number of malloc'd allocations
      --map_mem=NUM[K|M|G] allocate NUM bytes of memory via mmap before the
                           memscan
      --map_addr=ADDR      the starting address of the memory map
                           by default, the OS will select an address
      --fill               fill the local, malloc'd and/or mapped memory
                           with data before the memscan
  -t, --threads=NUM        create NUM threads before the memscan
      --sleep=SECONDS      pause the primary thread before scanning

SCAN OPTIONS
      --scan_byte[=NUM]    scan for NUM (a byte from 0x00 to 0xff)
                           or c3 (the x86 RET instruction) by default
      --shannon            compute Shannon Entropy for each mapped region
                           and physical page

OUTPUT OPTIONS
      --path               print the path (if available) in the memscan
  -p, --phys               print a summary of the physical pages w/ flags
  -P  --pfn                print each physical page number w/ flags
                           --pnf and --phys are exclusive

PROGRAM OPTIONS
  -h, --help               display this help and exit
  -k, --key                display key to the --phys flags
  -v, --version            output version information and exit

````
