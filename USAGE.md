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
      --mmap=FILE          open FILE using memory mapped I/O before the memscan
  -r, --read               read the contents of the files
  -f, --fork               fork a process and display the combined parent and
                           child memscan
  -l, --local=NUM[K|M|G]   allocate NUM bytes in local variables before the
                           memscan
  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan
  -s, --shared=NUM[K|M|G]  allocate NUM bytes of shared memory before the
                           memscan
      --fill               fill the local, malloc'd and/or shared memory
                           with data before the memscan
  -t, --threads=NUM        create NUM threads before the memscan

SCAN OPTIONS
      --scan_byte[=HEX]    scan for HEX (a byte from 00 to ff)
                           or c3 (the x86 RET instruction) by default
      --shannon            compute Shannon Entropy for each mmap region
                           and physical page

OUTPUT OPTIONS
  -i, --iomem              print a summary of /proc/iomem
      --path               print the path (if available) in the memscan
  -p, --phys               print physical page numbers w/ flags in the memscan

PROGRAM OPTIONS
  -h, --help               display this help and exit
  -k, --key                display key to the --phys flags
  -v, --version            output version information and exit

````
