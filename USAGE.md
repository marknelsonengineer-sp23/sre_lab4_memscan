Usage
=====
````
Usage: memscan [OPTION]...

The options below may be used to select memscan's operation
  -b, --block=FILE         open FILE using block I/O before the memscan
      --stream=FILE        open FILE using stream I/O before the memscan
      --mmap=FILE          open FILE using memory mapped I/O before the memscan
  -f, --fork               fork a process and display the combined parent and
                           child memscan
  -m, --malloc=NUM[K|M|G]  malloc NUM bytes before the memscan
      --path               print the path (if available in the memscan
  -p, --phys               include physical addresses (w/ flags) in the memscan
  -s, --shared=NUM[K|M|G]  create a shared memory region of NUM bytes before
                           the memscan
  -t, --threads=NUM        create NUM threads before the memscan
  -h, --help               display this help and exit
  -v, --version            output version information and exit

````
