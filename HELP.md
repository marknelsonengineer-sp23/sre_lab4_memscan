How to use Memscan
==================

@brief Documents various ways to use memscan with links to 
       detailed [examples](examples/EXAMPLES.md)

- Usage:  
  - `# ./memscan --version`
  - `# ./memscan --key`
  - `# ./memscan --help`
- The basics:  
  - `# ./memscan`
  - `# ./memscan --path`
- Checkout iomem:  
  - `# cat /proc/iomem`
  - `# ./memscan --iomem`
- Look at physical memory:  
  - `# ./memscan --phys`
  - `# ./memscan --pfn`
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
- Explore filters...
    - Print region at index 0 (the first region)
        - `# ./memscan --path 0`
    - Print regions at index 0 and 2
        - `# ./memscan --path 0 2`
    - Map a region to a specific address, then look at it
        - `# ./memscan --map_mem=200K --map_addr=0x2000 0x2000`
        - Consistently finding regions at a specific address is hard because of
          [ASLR] Address Space Layout Randomization.
    - Look for regions that have `libc` in their `--path` (you may need to change the search)
        - `# ./memscan --path libc`
    - Look at the default heap and stack
        - `# ./memscan --path heap stack`
            - One of things you'll learn when you work with memscan is that there can
              be many heaps (if you allocate a large block) and many stacks (in multi-threaded
              programs).
    - Look at just the executable regions (or just the writable regions) (or both!)
        - `# ./memscan --path +rx`
        - `# ./memscan --path +rw`
        - `# ./memscan --path +rx +rw`
        - You may see a region that's executable, but not readable:
            - `# ./memscan --path +x`
- You can use memscan to look at other processes...
  - `# ./memscan --path --pid=1`
  - `# ./memscan --path --pid=1 libz --phys`
  - `# ./memscan --path --pid=1 --pfn +rw`
- To stress test memscan and do a bit of everything, try this:
    - `clear && ./memscan --block=/etc/passwd --stream=/etc/passwd --map_file=/etc/passwd --read --local=16384 --numLocal=500 --malloc=1M --numMalloc=4 --map_mem=13M --map_addr=0x555000000000 --fill --threads=10 --sleep=1 --scan_byte=0xc3 --shannon --path --phys`
- To look at memscan that was compiled statically, try this:
  - `# ./memscan-static --path`

[ASLR]: https://en.wikipedia.org/wiki/Address_space_layout_randomization
