Contribute to MemScan
=====================

### Setup the development environment
- THIS SECTION IS STILL INCOMPLETE...
  - ...but it worked on Rasberian!
  - ...and it worked on Debian under WSL2
- Install Boost:  `# apt-get install libboost-all-dev`
- Install clang-tidy:  `# apt-get install clang-tidy`
- Install doxygen:  `# apt-get install doxygen`
- Install python3-pip ( # pacman -S python-pip ) `# apt-get install python3-pip`
- Install GitPython:  `# pip install GitPython`
- Install GraphViz (for DOT):  `# apt-get install graphviz`
- Install Capabilities:  `# apt-get install libcap-dev`


### Checkout memscan

Try out the different [Makefile targets](#MakeTargets), but `$ make memscan` 
will allow you to do the following:

- Usage:  `# ./memscan --version`, `# ./memscan --key`, `# ./memscan --help`
- The basics:  `# ./memscan`, `# ./memscan --path` 
- Checkout iomem:  `# cat /proc/iomem`, `# ./memscan --iomem`
- Look at physical memory:  `# ./memscan --phys`, `# ./memscan --pfn`
- Carve out some memory at a specific location, but don't access it:
  - `# ./memscan --path --map_addr=0x100000000000 --map_mem=64K --pfn | head -20`
  - Notice how most of them are `page not present`?  It's because even though
    the page has been allocated, it hasn't been accessed, so Linux, which is 
    demand paged, hasn't brought it in yet.
- Carve out some memory, and just read it:
  - `./memscan --path --map_addr=0x100000000000 --map_mem=64K --pfn --scan_byte=00 | head -20`
  - Notice how the pages have been allocated now, but the PFNs are all the same...
  - ...and the `0` flag has been set.  The demand pager brought in a bunch of 
    physical pages, but pointed them to one page (with all '0's) and set it to 
    copy-on-write.
  - Notice how it found 65,536 bytes of `0x00`.
- Run the same command with Shannon Entropy
  - `./memscan --path --map_addr=0x100000000000 --map_mem=64K --pfn --shannon | head -20`
  - Notice how all pages have `No entropy` -- all the values are the same
- Carve out some memory and write to it:
  - `# ./memscan --path --map_addr=0x100000000000 --map_mem=64K --pfn --shannon --fill | head -20`
  - The pages have been filled with #SHANNON_CONSTANT_FOR_ALLOCATIONS
  - This yields an exact Shannon entropy of `3.000`.
  - Notice how each of the PFNs are now different... and several new
    [flags](KEY.md) may be set:
    - `X`:   page exclusively mapped
    - `U`:   page has up-to-date data for file backed page
    - `L`:   page is in one of the LRU lists
    - `M`:   memory mapped page 
    - `A`:   anonymous: memory mapped page that is not part of a file
    - `B`:   swapBacked: page is backed by swap/RAM
- The same thing happens with memory mapped files...
  - `# ./memscan --path --map_file=/bin/cat --pfn cat`
  - Notice how the page never gets mapped into memory... but if you run:
  - `# ./memscan --path --map_file=/bin/cat --pfn --read cat`
  - ...the pages get mapped to memory that's backed by an actual file:
    - `X`:   page exclusively mapped
    - `F`:   page is file mapped and not anonymously mapped
    - `U`:   page has up-to-date data for file backed page
    - `L`:   page is in one of the LRU lists 
    - `A`:   active: page is in the active LRU list
    - `R`:   reclaim: page will be reclaimed soon after its pageout IO completed 
    - `M`:   memory mapped page
- To stress test memscan and do a bit of everything, try this:
  - `clear && ./memscan --block=/etc/passwd --stream=/etc/passwd --map_file=/etc/passwd --read --local=16384 --numLocal=500 --malloc=1M --numMalloc=4 --map_mem=13M --map_addr=0x555000000000 --fill --threads=10 --sleep=1 --scan_byte=0xc3 --shannon --path --phys`
  

### Handling errors & warnings in a testing framework
- MemScan does not have a dedicated logger.
- Messages and exceptions should be pre-pended with #getProgramName and sent 
  to `stderr`.
- Instrument programs to make them easily to test.  This is easier with C++ 
  programs that have `try`/`catch` blocks and exceptions.  As a C program, 
  we have created three tools for our instrumentation:
  - `PRINT( outStream, ... )`:  Print a message
  - `WARNING( msg, ... )`:  Print a formatted warning message
  - `ASSERT( condition )`:  Assert a condition.  Call `exit()` when the 
     condition is not `true`
  - `FATAL_ERROR( msg, ... )`:  Print a message and call `exit()`
    
- Use ASSERT() for things that should never happen.
- Use FATAL_ERROR() for things that may happen and we need to tell the user
  why the program is ending.
- Functions may return `bool` if the local logic needs to check the results.  In
  most cases, however, functions should call FATAL_ERROR().
- Successful functions should operate silently.

Memscan uses FATAL_ERROR() and ASSERT() for error checking.  Normally,
these functions will call `exit()`, however when `TESTING` is defined,
they will call throwException() so [BOOST_CHECK_THROW] can catch it.

Furthermore, when an unexpected call to FATAL_ERROR() or ASSERT() happens,
the [Boost Test] framework can catch the exception rather than immediately
`exit()`.

This utility function is a unique hybrid of C and C++ -- as C does not support
exceptions.

Using this technique gives me a works/doesn't work test, but we loose
fidelity in that we don't get a reason code or message.  I'm willing to
accept that as I'd like to focus on writing a traditional command-line
program in C.  Bare-bones tests will tell us if something works or doesn't
but not _why_.


### Toolchain
- MemScan is written in C.
  - It's compiled with [gcc](https://gcc.gnu.org).
  - Use `make tidy` to lint the program.  [clang-tidy] is the linter.
  - Use `make doc` to build documentation:  [Doxygen] is used to produce it.
  - Use `make publish` to publish the documentation to [UHUnix].
  - It uses [Makefile] for its build.
  - It uses [GitHub] for version control.
  - All of the above should run clean and without warnings.
- MemScan uses [Boost Test] written in C++.  Use `make clean` and 
  `make test` to test the application.
- MemScan is a Linux-based, user-mode command-line program that needs to run
  with root-level privileges.
  - To get the most out of MemScan, it should be run as `root` or with the 
    `CAP_SYS_ADMIN` capability.  See https://www.kernel.org/doc/Documentation/vm/pagemap.txt
- MemScan was written in CLion, using `Makefile` based build.  Note:  CLion 
  usually uses a CMake based build.

  
### Coding Conventions
- Put 2 blank lines between functions
- Put a space before the `;`.  Ex. `return true ;`


### Naming Conventions
- I use CaMel case and snake_case interchangeably, depending on my mood.
- File names and the functions that are in them represent the names of the
  files they are processing.  Ie. `iomem`, `maps`, `pagemap`, et. al..
- Function names start with lowercase letters.
- I'm not using [Hungarian notation]
  in this project.
- Structures start with uppercase.


### Documentation Conventions
- Fully document "published" functions in `.h` files.  Leave internal 
  documentation in the `.c` files.
- If there's a code-block that's longer than a page, I'll usually comment the 
  `}` with an indication of what it's closing.  Ex. `} // validate()`
- Markdown files should use [references] and avoid embedding URLs in the
  narrative.


### Testing Notes
I need to work on this section as well.  Memscan is severely test deficient.  I'd
like to sweep through the code and make a bunch of unit and system tests
for it.

`make test` needs to run as `root`

One cool thing about memscan is that it's a C program that is tested by a C++
unit test framework [Boost Test](https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html).  
It's cool to have this working.

#### Test Results
|              | Archlinux | Debian                                       | System X | System Y |
|--------------|-----------|----------------------------------------------|----------|----------|
| Architecture | x86-64    | x86-64                                       |          |          |
| Date tested  | Ongoing   | 27 Mar 2023                                  |          |          |
| Build tested | Ongoing   | 2.1.0+10332                                  |          |          |
| make memscan | Clean     | Clean                                        |          |          |
| make doc     | Clean     | A few warnings, as Doxygen is out of date    |          |          |
| make test    | Clean     | Clean                                        |          |          |
| make lint    | Clean     | Some warnings as `clang-tidy` is out of date |          |          |


### Release Procedures
- Scrub all `@todo`s
- Review code for:
  - `const` correctness
  - Extraneous `#include`s or ill-documented `#include`s
  - Read the Doxygen content for each source file and Markdown file
  - Check for a space before `;`
- Run `make lint`
- Run `make clean` and then `make test`
- Run `make doc` and then `make publish`
- Create `scratch.c`, then `#include` each `.h` file and ensure they are 
  self-sufficient:
  - Ensure they have the `#include`s to stand alone.  
  - Ensure the functions they import are documented.
  - Remove any unnecessary `#include`s.
- Scrub GitHub issues
- Tag the release in Git

[Hungarian notation]: https://en.wikipedia.org/wiki/Hungarian_notation
[clang-tidy]: https://releases.llvm.org/13.0.0/tools/clang/tools/extra/docs/clang-tidy/
[Doxygen]: https://www.doxygen.nl
[UHUnix]: https://www2.hawaii.edu/~marknels/sre/memscan/index.html
[Makefile]: https://www.gnu.org/software/make/manual/make.html
[GitHub]: https://github.com/marknelsonengineer-sp23/sre_lab4_memscan
[Boost Test]: https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html
[BOOST_CHECK_THROW]: https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/boost_test/utf_reference/testing_tool_ref/assertion_boost_level_throw.html
[references]: https://www.doxygen.nl/manual/markdown.html#md_reflinks
