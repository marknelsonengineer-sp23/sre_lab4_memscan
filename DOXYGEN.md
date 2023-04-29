Doxygen Notes
=============

[Doxygen] is the de facto standard for generating documentation from annotated
C++ sources.  Doxygen's website is:  https://www.doxygen.nl

## Notes
- On Linux, I prefer compile my own version of Doxygen.
- On Windows, I install it from a Setup file.
- I've done a lot of experimentation with Doxygen, and I've developed a
  baseline configuration that I really like.
    - I have it as both a Doxyfile and a CMake configuration.
- I need to use [DOT](https://graphviz.org ) but
  not [DIA](http://dia-installer.de) which is very old and is not maintained
  anymore.
- Here's a great link on Doxygen's graphics capabilities:  Dig into it:  https://alesnosek.com/blog/2015/06/28/diagrams-and-images-in-doxygen/


### Installing Graphviz from source
DOT is a tool from [Graphviz] that Doxygen uses for its drawings.

As a mortal user:

    wget https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/8.0.1/graphviz-8.0.1.tar.gz
    gunzip graphviz-8.0.1.tar.gz
    tar -xvf graphviz-8.0.1.tar
    cd graphviz-8.0.1
    ./configure
    make
    sudo make install


### Installing Clang
As root, run one of the following:

    pacman -S clang
    dnf install clang

    wget https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-16.0.0.tar.gz
    gunzip -c llvmorg-16.0.0.tar.gz | tar -xvf -
    cd llvm-project-llvmorg-16.0.0
    cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release



### Compile Doxygen from source on Linux
You may need some of these tools...  As root:

    pacman -S llvm    # This is 368MB, but it's only needed to compile DOxygen.  We will remove it after.
    pacman -S xapian-core

    dnf install cmake

As a mortal user:

     wget https://www.doxygen.nl/files/doxygen-1.9.6.src.tar.gz
     gunzip doxygen-1.9.6.src.tar.gz
     tar -xvf doxygen-1.9.6.src.tar
     cd doxygen-1.9.6

     mkdir build
     cd build

     cmake -L "Unix Makefiles" -Dbuild_search=ON -Duse_libclang=ON -Dbuild_parse=ON ..
     # ... and look over the build.  If you're OK with it, build it with -G
     
     cmake -G "Unix Makefiles" -Dbuild_search=ON -Duse_libclang=ON -Dbuild_parse=ON ..

     time make -j 3   # Watch your memory and CPU utilization... This needs at least 4G of memory
     time make -j 2   # Worked and took under 5 minutes to build in 2G of memory
     time make -j 1   # Takes 10 minutes to build, but can be done in 2G of memory

     make test   # One test failed, but it's OK

     sudo make install

As root:

     sudo pacman -R xapian-core
     sudo pacman -R llvm    # This is 368MB, but it's only needed to compile DOxygen.  We will remove it after
     sudo pacman -Scc       # Clean the cache
     sudo pacman -Qtd       # List orphan packages
     sudo rm -rf ~/.cache/*   
     sudo fstrim -av 

## To integrate (and run) Doxygen with Visual Studio
- TODO... I wire it into Tools
- Click Tools > Doxygen

## To integrate (and run) Doxygen with CLion
- It's automatically integrated into CLion - CMake does all of the work
- Select the Doxygen target and build the project.  Don't run it as there is no run target

## To Run Doxygen from Bash
- `cd` to the directory and simply run `doxygen`

## To Configure Doxygen
- I clone the configuration from an existing app, then pick through the config
  line-by-line until I'm happy with it.  I have a lot of customizations, but
  it looks good the way I publish it.

[Doxygen]:  https://www.doxygen.nl
[Graphviz]:  https://graphviz.org
