Contribute to MemScan
=====================

### Setup the development environment
- THIS SECTION IS INCOMPLETE AND NOT TESTED
  - ...but it worked on Rasberian!
- Install Boost ( # apt-get install libboost-all-dev )
- Install clang-tidy ( # apt-get install clang-tidy )
- Install doxygen ( # apt-get install doxygen )
- Install python3-pip ( # pacman -S python-pip ) ( # apt-get install python3-pip )
- Install GitPython ( # pip install GitPython )
- Install GraphViz (for DOT)  ( # apt-get install graphviz )

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
- Use FATAL_ERROR() for things that may happen and we want to tell the user
  why the program is ending.
- Functions may return `bool` if local logic needs to check the results.  In
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

[Boost Test]:  https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html
[BOOST_CHECK_THROW]: https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/boost_test/utf_reference/testing_tool_ref/assertion_boost_level_throw.html

Using this technique gives me a works/doesn't work test, but we loose
fidelity in that we don't get a reason code or message.  I'm willing to
accept that as I'd like to focus on writing a traditional command-line
program in C.  Bare-bones tests will tell us if something works or doesn't
but not _why_.

### Toolchain
- MemScan is written in C.
  - It's compiled with [gcc](https://gcc.gnu.org).
  - Use `make tidy` to lint the program: [clang-tidy](https://releases.llvm.org/13.0.0/tools/clang/tools/extra/docs/clang-tidy/) 
    as a linter.
  - Use `make doc` to build documentation:  [Doxygen](https://www.doxygen.nl) 
    for documentation.
  - Use `make publish` to publish the documentation to [UHUnix](https://www2.hawaii.edu/~marknels/sre/memscan/index.html).
  - It uses [Makefile](https://www.gnu.org/software/make/manual/make.html) 
    for its build.
  - It uses [GitHub](https://github.com/marknelsonengineer-sp23/sre_lab4_memscan) 
    for version control.
  - All of the above should run clean and without warnings.
- MemScan uses [Boost unit tests](https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html) which are written in C++.
- MemScan is a Linux-based, user-mode command-line program.
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
- I'm not using [Hungarian notation](https://en.wikipedia.org/wiki/Hungarian_notation)
  in this project.
- Structures start with uppercase.

### Documentation Conventions
- Fully document "published" functions in `.h` files.  Leave internal 
  documentation in the `.c` files.
- If there's a code-block that's longer than a page, I'll usually comment the 
  `}` with an indication of what it's closing.  Ex. `} // validate()` 

### Release Procedures
- Run `make lint`
- Run `make clean` and then `make test`
- Run `make doc` and then `make publish`
- Scrub all `@todo`s
- Scrub GitHub issues
- Tag the release in Git
