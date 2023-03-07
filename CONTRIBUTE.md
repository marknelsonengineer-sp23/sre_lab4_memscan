Contribute to MemScan
=====================

### Handling errors & warnings
- MemScan does not have a dedicated logger.
- Messages and exceptions should be pre-pended with #getProgramName and sent 
  to `stderr`.
- We need to instrument programs to make them easily to test.  This is easier
  with C++ programs that have `try`/`catch` blocks and exceptions.
- Use `assert()` for things that should never happen and we don't really want 
  to validate or test for (there are probably not many of these).
- Most functions should return `bool`
  - Successful functions should operate silently and return `true`.
  - Warnings, `validation` and errors should have a commented out `printf()` 
    or comment and return `false`.  These can be caught and tested by Boost tests.
    - Using this technique gives me a works/doesn't work test, but we loose
      fidelity in that we don't get a reason code or message.  I'm willing to
      accept that as I'd like to focus on writing a tradational command-line
      progam in C.  Bare-bones tests will tell us if someting works or doesn't
      but not _why_.
- Use #FATAL_ERROR to print a message and `exit( EXIT_FAILURE )`.

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

### Documentation Conventions
- Fully document "published" functions in `.h` files.  Leave internal 
  documentation in the `.c` files.
- If there's a code-block that's longer than a page, I'll usually comment the 
  `}` with an indication of what it's closing.  Ex. `} // validate()` 
