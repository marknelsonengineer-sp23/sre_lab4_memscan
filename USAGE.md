Usage
=====
````
Usage: memscan [OPTION]... [PATTERN]... 
       memscan -i|--iomem

  When PATTERN is present, only process sections with a path that includes PATTERN
  If PATTERN is 'r' 'w' or 'x' then include sections with that permission
  When PATTERN is not present, process all sections

The options below may be used to select memscan's operation

PRE-SCAN OPTIONS
  -b, --block=FILE         open FILE using block I/O before the memscan
      --stream=FILE        open FILE using stream I/O before the memscan
      --map_file=FILE      open FILE using memory mapped I/O before the memscan
  -r, --read               read the contents of the files
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
      --scan_byte[=HEX]    scan for HEX (a byte from 00 to ff)
                           or c3 (the x86 RET instruction) by default
      --shannon            compute Shannon Entropy for each mapped region
                           and physical page

OUTPUT OPTIONS
  -i, --iomem              print a summary of /proc/iomem
      --path               print the path (if available) in the memscan
  -p, --phys               print a summary of the physical pages w/ flags
  -P  --pfn                print each physical page number w/ flags

PROGRAM OPTIONS
  -h, --help               display this help and exit
  -k, --key                display key to the --phys flags
  -v, --version            output version information and exit

````
