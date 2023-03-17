Memscan - Memory Scanner
========================

<img src=".doxygen/images/logo_memscan_660x495.png" style="width:300px; float: left; margin: 0 10px 10px 0;" alt="Tinfoil Hat Cat"/>

We live in an ocean of illegal addresses dotted with islands of legal
addresses.  Let's explore every grain of sand on our islands.

#### Links
The project's home page (hosted by GitHub) is:  https://github.com/marknelsonengineer-sp23/sre_lab4_memscan

The source code documentation (hosted by UH) is:  https://www2.hawaii.edu/~marknels/sre/memscan

#### The Lab
This lab looks easy at first, but (hopefully) proves to be quite challenging.

As you know, user-processes run in "Virtual Memory" process space — a process space that is unique for each process.  
In actuality, the process space is made up of "regions".  Some the regions are completely unique to the process 
(such as the read-write data pages and the stack for the process). 
There are other regions that are shared amongst many processes.

It's the Kernel's responsibility to manage (and assign/map) the memory regions for each process — however, 
there's nothing secret about how they are mapped.  Therefore, Linux publishes the memory map information on the 
filesystem for every process.  Information about Linux processes are contained in a directory called `/proc`.

One interesting file is `/proc/iomem`.  This file shows the physical memory regions on your computer.  You need to be 
`root` to view it with addresses.

`/proc` contains a "special process directory" for every process currently running and another directory called `self` 
that links to the currently running process.  Each of these directories contain information about the process, such 
as the environment `environ`, command line `cmdline`, and overall status `status`.

`maps` describes a region of contiguous virtual memory in a process or thread.  Here's an example:

    55cb8756b000-55cb87598000 r--p 00000000 08:04 1574849  /usr/bin/bash
    55cb87598000-55cb87669000 r-xp 0002d000 08:04 1574849  /usr/bin/bash
    55cb87669000-55cb876a2000 r--p 000fe000 08:04 1574849  /usr/bin/bash
    55cb876a2000-55cb876a6000 r--p 00136000 08:04 1574849  /usr/bin/bash
    55cb876a6000-55cb876af000 rw-p 0013a000 08:04 1574849  /usr/bin/bash
    55cb876af000-55cb876b9000 rw-p 00000000 00:00 0        
    55cb8832a000-55cb88494000 rw-p 00000000 00:00 0        [heap]
    7f3ff0ecf000-7f3ffe3ff000 r--p 00000000 08:04 1838829  /usr/lib/locale/locale
    7f3ffe3ff000-7f3ffecd3000 r--s 00000000 08:07 131095   /var/lib/sss/mc/passwd
    7f3ffecd3000-7f3ffecd5000 r--p 00000000 08:04 1588322  /usr/lib64/libnss.so.2

Each row has the following fields:

| Command       | Purpose                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
|---------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `address`     | The starting and ending address of the region in the process's address space (See Notes below).                                                                                                                                                                                                                                                                                                                                                              |
| `permissions` | This describes how pages in the region can be accessed.  There are four different permissions: `r`ead, `w`rite, e`x`ecute, and `s`hared. If read/write/execute are disabled, a `-` will appear instead of the `r`/`w`/`x`. If a region is not shared, it is private, and a `p` will appear instead of an `s`.  If the process attempts to access memory in a way that is not permitted, the memory manager will interrupt the CPU with a segmentation fault. |
| `offset`      | If the region was mapped from a file (using [mmap()][2]), this is the offset in the file where the mapping begins.  If the memory was not mapped from a file, it's `0`.                                                                                                                                                                                                                                                                                      |
| `device`      | If the region was mapped from a file, this is the major and minor device number (in hex) where the file lives.                                                                                                                                                                                                                                                                                                                                               |
| `inode`       | If the region was mapped from a file, this is the file number.                                                                                                                                                                                                                                                                                                                                                                                               |
| `pathname`    | If the region was mapped from a file, this is the name of the file. This field is blank for anonymously mapped regions. There are also special regions with names like `[heap]`, `[stack]`, or `[vdso]`.                                                                                                                                                                                                                                                     |

Notes:
  - `maps` reports addresses like this:  [ `00403000-00404000` )... the "end address" one byte past the valid range.  
    When memscan prints a range, it shows inclusive addresses like this: [ `00403000-00403fff` ]
  - `[vdso]` stands for virtual dynamic shared object.  It's used by system calls to switch to kernel mode.
  - Permissions can be changed using the [mprotect()][1] system call.

[1]: https://man7.org/linux/man-pages/man2/mprotect.2.html
[2]: https://man7.org/linux/man-pages/man2/mmap.2.html


#### Makefile
Memscan uses the following `Makefile` targets:

| Command      | Purpose                                                    |
|--------------|------------------------------------------------------------|
| `make`       | Compile your program                                       |
| `make test`  | Compile your program and run it                            |
| `make debug` | Compile your program with debug mode ( `DEBUG` is defined) |
| `make clean` | Remove any compiler output                                 |
| `make doc`   | Make a Doxygen website and push it to UH UNIX              |
| `make lint`  | Use clang-tidy to do static analysis on the source code    |


# Memscan 2.0
Memscan 1.0 is an assignment for my Software Reverse Engineering class.
Memscan 2.0 is my own project designed to explore some details of modern
memory management in Linux.  Memscan 2.0 has _**a lot**_ more options than
earlier versions.

## Notes
  - Have the file readers, like `--block`, `--stream` and `--map_file` read files
    with known or unique entropy values and then try to find them in memory.
  - The file readers, `--block` and `--stream` read into a malloc`d buffer
    that's 4x the size of a physical page.  This way, you're guaranteed to
    see a few pages with the unique entropy.
  - When `--malloc` and `--fill` are used, a special constant, 
    #SHANNON_CONSTANT_FOR_ALLOCATIONS (with a Shannon entropy of 3.000) will fill 
    memory.  This can be searched for with `--shannon` or `--scan_byte=11`.
  - `--malloc` can be used with `--numMalloc` to explore the difference between
    one large malloc and many small ones.  Note that `DEFAULT_MMAP_THRESHOLD`
    is 131,072 bytes.  See [Malloc Internals] and [mallopt].
  - memscan does not explore System V shared memory segments.

[Malloc Internals]: https://sourceware.org/glibc/wiki/MallocInternals
[mallopt]: https://man7.org/linux/man-pages/man3/mallopt.3.html


# Toolchain
This project is the product of a tremendous amount of R&D and would not be possible without the following world-class tools:

| Tool           | Website                    |                                  Logo                                                                                  |
|----------------|----------------------------|:----------------------------------------------------------------------------------------------------------------------:|
| **gcc**        | https://gcc.gnu.org        |        <img src=".doxygen/images/logo_gcc.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="GCC"/>        |
| **llvm/clang** | https://clang.llvm.org     |      <img src=".doxygen/images/logo_llvm.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="clang"/>       |
| **Boost**      | https://boost.org          |      <img src=".doxygen/images/logo_boost.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="Boost"/>      |
| **Doxygen**    | https://doxygen.nl         |    <img src=".doxygen/images/logo_doxygen.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="Doxygen"/>    |
| **DOT**        | https://graphviz.org       |        <img src=".doxygen/images/logo_dot.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="Dot"/>        |
| **Git**        | https://git-scm.com        |        <img src=".doxygen/images/logo_git.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="Git"/>        |
| **GitHub**     | https://github.com         |     <img src=".doxygen/images/logo_github.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="GitHub"/>     |
| **Linux**      | https://kernel.org         |      <img src=".doxygen/images/logo_linux.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="Linux"/>      |
| **ArchLinux**  | https://archlinux.org      |  <img src=".doxygen/images/logo_archlinux.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="ArchLinux"/>  |
| **VirtualBox** | https://www.virtualbox.org | <img src=".doxygen/images/logo_virtualbox.png" style="height:40px; float: center; margin: 0 0 0 0;" alt="VirtualBox"/> |


# The Assignment

You are to write a program from scratch.  Feel free to incorporate artifacts from other labs such as `.gitignore`, 
header blocks or a simple `Makefile`.  Your lab must include a Makefile, which must have (at least) the following targets:
  - `make clean`
  - `make`
  - `make test`

You can name your program whatever you want and `$ make test` must run it.

The program will open and read `/proc/self/maps` and parse the contents.

You can skip the `[vvar]` region.  You can also skip non-readable regions (of course).

Finally, scan each region and print the following information:

    $ ./memscan
    Memory scanner
     0: 0x559681970000 - 0x559681970fff  r--p  Number of bytes read      4,096  Count of 0x41 is       0
     1: 0x559681971000 - 0x559681971fff  r-xp  Number of bytes read      4,096  Count of 0x41 is       2
     2: 0x559681972000 - 0x559681972fff  r--p  Number of bytes read      4,096  Count of 0x41 is       3
     3: 0x559681973000 - 0x559681973fff  r--p  Number of bytes read      4,096  Count of 0x41 is       3
     4: 0x559681974000 - 0x559681974fff  rw-p  Number of bytes read      4,096  Count of 0x41 is       0
     5: 0x559681975000 - 0x5596819b8fff  rw-p  Number of bytes read    278,528  Count of 0x41 is       1
     6: 0x559681b35000 - 0x559681b55fff  rw-p  Number of bytes read    135,168  Count of 0x41 is      12
     7: 0x7f7348800000 - 0x7f7348ae8fff  r--p  Number of bytes read  3,051,520  Count of 0x41 is   3,477
     8: 0x7f7348c5d000 - 0x7f7348c5efff  rw-p  Number of bytes read      8,192  Count of 0x41 is       0
     9: 0x7f7348c5f000 - 0x7f7348c80fff  r--p  Number of bytes read    139,264  Count of 0x41 is     126
    10: 0x7f7348c81000 - 0x7f7348ddbfff  r-xp  Number of bytes read  1,421,312  Count of 0x41 is  19,589
    11: 0x7f7348ddc000 - 0x7f7348e32fff  r--p  Number of bytes read    356,352  Count of 0x41 is   5,061
    12: 0x7f7348e33000 - 0x7f7348e36fff  r--p  Number of bytes read     16,384  Count of 0x41 is       7
    13: 0x7f7348e37000 - 0x7f7348e38fff  rw-p  Number of bytes read      8,192  Count of 0x41 is       3
    14: 0x7f7348e39000 - 0x7f7348e47fff  rw-p  Number of bytes read     61,440  Count of 0x41 is       0
    15: 0x7f7348e4f000 - 0x7f7348e4ffff  r--p  Number of bytes read      4,096  Count of 0x41 is       5
    16: 0x7f7348e50000 - 0x7f7348e75fff  r-xp  Number of bytes read    155,648  Count of 0x41 is   1,870
    17: 0x7f7348e76000 - 0x7f7348e7ffff  r--p  Number of bytes read     40,960  Count of 0x41 is     611
    18: 0x7f7348e80000 - 0x7f7348e81fff  r--p  Number of bytes read      8,192  Count of 0x41 is      19
    19: 0x7f7348e82000 - 0x7f7348e83fff  rw-p  Number of bytes read      8,192  Count of 0x41 is       0
    20: 0x7fff39cd2000 - 0x7fff39cf2fff  rw-p  Number of bytes read    135,168  Count of 0x41 is      18
    21: 0x7fff39d3e000 - 0x7fff39d41fff  r--p  [vvar] excluded
    22: 0x7fff39d42000 - 0x7fff39d43fff  r-xp  Number of bytes read      8,192  Count of 0x41 is      40
    23: 0xffffffffff600000 - 0xffffffffff600fff  --xp  read permission not set on [vsyscall]

This program counts the number of `A`s in each region.  It doesn't matter what you do as long as you read every single 
byte you can read.

Your program should compile clean (no warnings) and must not core dump when it runs.

Note:  The goal of this program is to focus on 2 things:
  - Parsing the `maps` file
  - Scanning memory

To that end, your program does not have to match the output perfectly... here are some exceptions:
  - You don't have to use commas in your numbers, but if you wanted to try this:  `Number of bytes read %'10d`.
  - Ideally, you should print a reason you are skipping a region, but you don't have to do that either.
  - You can print the end of the region as the actual end (as I do) or the end that you get from the `maps` file.
  - The columns don't have to line up perfectly

[RDTSC]: https://en.wikipedia.org/wiki/Time_Stamp_Counter
