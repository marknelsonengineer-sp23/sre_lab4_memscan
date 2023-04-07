Contribute to Memscan
=====================

@brief Documents the development environment, development philosophy, major
       design decisions, code conventions, et al.

Memscan is a Makefile-based C Linux usermode console program

## Setup the development environment
To setup your development environment on a fresh system, you'll likely need 
the following:
- This section is incomplete, but...
  - ...but it worked on Rasberian!
  - ...and it worked on Debian under WSL2

| Package            	    | Debian / apt-get                     	 | ArchLinux / pacman       	 | RedHat / rpm 	 |
|-------------------------|----------------------------------------|----------------------------|----------------|
| [Boost]              	  | `# apt-get install libboost-all-dev` 	 | 	                          | 	              |
| [clang-tidy]         	  | `# apt-get install clang-tidy`       	 | 	                          | 	              |
| [Doxygen]            	  | `# apt-get install doxygen`          	 | 	                          | 	              |
| [Python pip]            | `# apt-get install python3-pip`      	 | `# pacman -S python-pip` 	 | 	              |
| [GitPython]          	  | `# pip install GitPython`            	 | 	                          | 	              |
| [Graphviz] for DOT   	  | `# apt-get install graphviz`         	 | 	                          | 	              |
| [Capabilities]       	  | `# apt-get install libcap-dev`       	 | 	                          | 	              |



## Handling errors & warnings in a testing framework
- Memscan does not have a dedicated logger.
- Messages and exceptions should be pre-pended with #getProgramName and sent 
  to `stderr`.
- Instrument programs to make them easily to test.  This is easier with C++ 
  programs that have `try`/`catch` blocks and exceptions.  As a C program, 
  we have created some tools for test instrumentation:
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
these functions will call `exit( EXIT_FAILURE )`, however when `TESTING` is 
defined, they will call throwException() so [BOOST_CHECK_THROW] can catch it.

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


## Local variables
There are many ways to do local variables.  Local variables are fundamentally 
different from `malloc` and `mmap`.  When the thread of execution exits
the scope,  the local data evaporates.  The other challenge with local
variables is how to dynamically size them.  Locals, like globals, are fixed
at compile-time.  So how do we allocate `--local` bytes of local data when 
`--local` is only known at run time?

Consider the [stack] and how most programs find their parameters and local
variables relative to the base of their [stack frames], and it becomes clear 
that programs cannot allocate a block of local memory whose size is only 
known at runtime.

There are a number of ways we could solve these problems: 
  - Create some local variables on the `main()` thread and immediately 
    release them to continue the program.
  - Create some local variables on the `main()` thread and then complete
    the rest of the program from that local function, thereby keeping the 
    local variables in scope.
  - Spin up a new [thread], with a new [stack] and then create some local 
    variables on that new stack.

Memscan creates a new [thread] on a new stack.

The next problem is how to allocate an arbitrary amount of local memory.  The
only way to do this in C is using [recursion].  Having a subroutine allocate
local memory repeatedly.  We do use [recursion] in memscan, but we've found a 
novel way to allocate an arbitrary amount of local memory with [Inline Assembly].

It ends up that with two relatively simple [Inline Assembly] functions, we can
create arbitrarily sized local variables using `--local`.  The functions are:  
  - getBaseOfStack() `GET_BASE_OF_STACK` is used to create a pointer to 
    the start of a local memory region.  This is kept in an array of pointers
    #localAllocations which is #numLocals long.
  - allocateLocalStorage() `ALLOCATE_LOCAL_STORAGE` subtracts an arbitrary value
    from the [Stack Pointer], creating the local storage.

Then, we recursively call the above function `--numLocal` times.  This gives
us the ability to create many small local allocations or a few big ones.  It 
also allows us to explore how Linux will expand a stack should it become 
necessary.

Finally, remember that the total amount of local data will be a bit larger
than `--local` x `--numLocal` bytes.  This is because each call to a function
incurs some overhead on the stack frame.  Not a lot, but not 0.

Have fun exploring local variables.  Use `--shannon` or `--fill` and `--scan_byte` to 
find the new stack.

[stack]:  https://en.wikipedia.org/wiki/Stack-based_memory_allocation
[stack frames]:  https://en.wikipedia.org/wiki/Call_stack
[thread]:  https://en.wikipedia.org/wiki/Thread_(computing)
[recursion]:  https://en.wikipedia.org/wiki/Recursion_(computer_science)
[Inline Assembly]:  https://en.wikipedia.org/wiki/Inline_assembler
[Stack Pointer]: https://en.wikipedia.org/wiki/Call_stack#STACK-POINTER


## Toolchain
- Memscan is written in C.
  - It's compiled with [gcc](https://gcc.gnu.org).
  - Use `make tidy` to lint the program.  [clang-tidy] is the linter.
  - Use `make doc` to build documentation:  [Doxygen] is used to produce it.
  - Use `make publish` to publish the documentation to [UHUnix].
  - It uses [Makefile] for its build.
  - It uses [GitHub] for version control.
  - All of the above should run clean and without warnings.
- Memscan uses [Boost Test] written in C++.  Use `make clean` and 
  `make test` to test the application.
- Memscan is a Linux-based, user-mode command-line program that needs to run
  with root-level privileges.
  - To get the most out of Memscan, it should be run as `root` or with the 
    `CAP_SYS_ADMIN` capability.  See https://www.kernel.org/doc/Documentation/vm/pagemap.txt
- Memscan was written in CLion, using `Makefile` based build.  Note:  CLion 
  usually uses a CMake based build.

  
## Coding Conventions
- Put 2 blank lines between functions
- Put a space before the `;`.  Ex. `return true ;`
- Memscan uses `printf` but we will be converting to TBD  @todo fix this


## Naming Conventions
- I use [CaMel case] and [snake_case] interchangeably, depending on my mood.
- File names and the functions that are in them represent the names of the
  files they are processing.  Ie. `iomem`, `maps`, `pagemap`, et. al..
- Function names start with a lowercase letter.
- Structures, Enums, et al. start with an uppercase letter.
- Constants and macros are in all UPPERCASE.
- I'm not consistently using [Hungarian notation], but you might see it from
  time to time.

[CaMel case]: https://en.wikipedia.org/wiki/Camel_case
[snake_case]: https://en.wikipedia.org/wiki/Snake_case


## Documentation Conventions
- Fully document "published" functions in `.h` files.  Leave internal 
  documentation in the `.c` files.
- If there's a code-block that's longer than a page, I'll usually comment the 
  `}` with an indication of what it's closing.  Ex. `} // validate()`
- Markdown files should use [references] and avoid embedding URLs in the
  narrative.


## Testing Notes
I need to work on this section as well.  Memscan is severely test deficient.  I'd
like to sweep through the code and make a bunch of unit and system tests
for it.

`make test` needs to run as `root`

One cool thing about memscan is that it's a C program that is tested by a C++
unit test framework [Boost Test](https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html).  
It's cool to have this working.


### Test Results
|              | Archlinux | Debian on WSL                                | ARM 32-bit |
|--------------|-----------|----------------------------------------------|------------|
| Architecture | x86-64    | x86-64                                       |            |
| Date tested  | Ongoing   | 27 Mar 2023                                  |            |
| Build tested | Ongoing   | 2.1.0+10332                                  |            |
| make memscan | Clean     | Clean                                        |            |
| make doc     | Clean     | A few warnings, as Doxygen is out of date    |            |
| make test    | Clean     | Clean                                        |            |
| make lint    | Clean     | Some warnings as `clang-tidy` is out of date |            |


## Release Procedures
- For each source file:
  - Cleanup `#include` files
    - Create `scratch.c`, then `#include` each `.h` file and ensure they are
      self-sufficient:
      - Ensure they have the `#include`s to stand alone.
      - Remove any unnecessary `#include`s.
      - Ensure the functions `#include`d are documented. 
  - `const` correctness
    - Look for `const`-able parameters
  - Look for any API calls and document them with `@API{}`
    - Man pages come from:  https://man.archlinux.org
    - Some calls have multiple man pages; reference system calls (2) first, 
      then the Standard C API (3) and lastly the POSIX API (3p)
  - Read the Doxygen content for each source file and Markdown file
  - Check for a space before `;`
  - Ensure all parameters are validated
  - `assert( validateThing() )` before-and-after anything that modifies it
  - Look for CLion problems
- Run `make lint`
- Run `make clean` and then `make test`
- Run `make doc` and then `make publish`
- Scrub GitHub issues
- Scrub all `@todo`s
- Create a tagged release in Git


## Source Code Status

| Module               	             | const correctness 	 | include correctness 	 | Review Doxygen 	 | 32-bit 	  | Boost tests 	 | API documented |
|------------------------------------|---------------------|-----------------------|------------------|-----------|---------------|:---------------|
| `allocate.h` <br> `allocate.c`  	  | 	                   | 	                     | 	                | 	         | 	             |                |
| `assembly.h` <br> `assembly.c`  	  | 	                   | 	                     | 	                | 	         | 	             |                |
| `colors.h`           	             | 	                   | 	                     | 	                | 	         | 	             |                |
| `config.h` <br> `config.c`    	    | 	                   | 	                     | 	                | 	         | 	             |                |
| `convert.h` <br> `convert.c`   	   | 	                   | 	                     | 	                | 	         | 	             |                |
| `files.h` <br> `files.c`     	     | 4 Apr 23	           | 	                     | 	4 Apr 23        | 4 Apr 23	 | 	             | 4 Apr 23       |
| `iomem.h` <br> `iomem.c`     	     | 	                   | 	                     | 	                | 	         | 	             |                |
| `maps.h` <br> `maps.c`      	      | 	                   | 	                     | 	                | 	         | 	             |                |
| `memscan.h` <br> `memscan.c`   	   | 	                   | 	                     | 	                | 	         | 	             |                |
| `pagecount.h` <br> `pagecount.c` 	 | 6 Apr 23	           | 6 Apr 23	             | 6 Apr 23	        | 	         | (1) 6 Apr 23	 | 6 Apr 23       |
| `pageflags.h` <br> `pageflags.c` 	 | 	                   | 	                     | 	                | 	         | 	             |                |
| `pagemap.h` <br> `pagemap.c`   	   | 	                   | 	                     | 	                | 	         | 	             |                |
| `shannon.h` <br> `shannon.c`   	   | 	                   | 	                     | 	                | 	         | 	             |                |
| `threads.h` <br> `threads.c`   	   | 	                   | 	                     | 	                | 	         | 	             |                |
| `trim.h` <br> `trim.c`      	      | 6 Apr 23	           | 6 Apr 23	             | 6 Apr 23	        | N/A	      | 6 Apr 23	     | 6 Apr 23       |
| `version.h`          	             | 	4 Apr 23           | None	                 | 4 Apr 23	        | 4 Apr 23	 | 	             | 4 Apr 23       |

The `main()` for memscan is in `memscan.c`.

(1):  Not worth the effort.

[Hungarian notation]: https://en.wikipedia.org/wiki/Hungarian_notation
[clang-tidy]: https://releases.llvm.org/13.0.0/tools/clang/tools/extra/docs/clang-tidy/
[Doxygen]: https://www.doxygen.nl
[UHUnix]: https://www2.hawaii.edu/~marknels/sre/memscan/index.html
[Makefile]: https://www.gnu.org/software/make/manual/make.html
[GitHub]: https://github.com/marknelsonengineer-sp23/sre_lab4_memscan
[Boost Test]: https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html
[Boost]: https://www.boost.org/
[BOOST_CHECK_THROW]: https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/boost_test/utf_reference/testing_tool_ref/assertion_boost_level_throw.html
[references]: https://www.doxygen.nl/manual/markdown.html#md_reflinks
[Capabilities]: https://man7.org/linux/man-pages/man7/capabilities.7.html
[Python pip]:  https://pypi.org/project/pip/
[GitPython]:  https://gitpython.readthedocs.io/en/stable/
[Graphviz]:  https://graphviz.org
