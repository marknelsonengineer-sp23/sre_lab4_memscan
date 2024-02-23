Doxygen Notes
=============

[Doxygen] is the de facto standard for generating documentation from annotated
C, C++ and Python sources.  Doxygen's website is:  https://www.doxygen.nl

## Notes
- On Linux, I prefer compile my own version of Doxygen because
  - `-Dbuild_search=ON` -- Enable search
  - `-Duse_libclang=ON` -- Enable clang
  - `-Dbuild_parse=ON`  -- Enable clang-based language parsing
- On Windows, I install it from a Setup file.
- I've done a lot of experimentation with Doxygen, and I've developed a
  baseline configuration that I really like.
  - I have it as both a Doxyfile and a CMake configuration.
- I need to use [DOT](https://graphviz.org ) but
  not [DIA](http://dia-installer.de) which is very old and is not maintained
  anymore.
- Here's a great link on Doxygen's graphics capabilities:  Dig into it:
  https://alesnosek.com/blog/2015/06/28/diagrams-and-images-in-doxygen/
- We are now using ArchLinux's `graphviz` rather than building our own


### Installing prerequisites
As root:

    pacman -S clang llvm xapian-core


### Compile Doxygen from source on Linux
As a mortal user:

     wget https://www.doxygen.nl/files/doxygen-1.10.0.src.tar.gz
     tar -xzvf doxygen-1.10.0.src.tar.gz
     cd doxygen-1.10.0

     mkdir build
     cd build

     export CC=clang
     cmake -L "Unix Makefiles" -Dbuild_search=ON -Duse_libclang=ON -Dbuild_parse=ON ..
     # ... and look over the build.  If you're OK with it, build it with -G

     cmake -G "Unix Makefiles" -Dbuild_search=ON -Duse_libclang=ON -Dbuild_parse=ON ..

     time make -j 3   # Watch your memory and CPU utilization... This needs at least 4G of memory
     time make -j 2   # Worked and took under 5 minutes to build in 2G of memory
     time make -j 1   # Takes 10 minutes to build, but can be done in 2G of memory

     make test   # Test #12 fails, but Doxygen seems fine

     sudo make install


## To integrate (and run) Doxygen with Visual Studio
- TODO... I wire it into Tools
- Click Tools > Doxygen


## To integrate (and run) Doxygen with CLion
- It's automatically integrated into CLion - CMake does all of the work
- Select the Doxygen target and build the project.  Don't run it as there is no run target


## To integrate (and run) Doxygen on Linux
- Install pip with `pacman -S python-pip`
- `pacman -S python-gitpython`


## To Run Doxygen from Bash
- `cd` to the directory and simply run `doxygen`


## To Configure Doxygen
- I clone the configuration from an existing app, then pick through the config
  line-by-line until I'm happy with it.  I have a lot of customizations, but
  it looks good the way I publish it.


[Doxygen]:  https://www.doxygen.nl
[Graphviz]:  https://graphviz.org
