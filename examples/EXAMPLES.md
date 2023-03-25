Examples
========

## Swap
One of the goals of memscan is to observe how and when systems swap pages to memory.

This experiment was run on a current ArchLinux VM provisioned with 2G of memory
and the default swapfile.

To induce swapping, we need a little help...I use [stressapptest](https://github.com/stressapptest/stressapptest) 
which is available on ArchLinux [here](https://aur.archlinux.org/packages/stressapptest).  Other
stress testing tools are listed [here](https://wiki.archlinux.org/title/Stress_testing).

#### Install stressapptest
`#` runs as `root`.  `$` runs as a mortal user.

    # pacman -S libaio

    $ git clone https://aur.archlinux.org/stressapptest.git
    $ cd stressapptest
    $ makepkg

    # pacman -U stressapptest-1.0.9-1-x86_64.pkg.tar.zst
                 
#### Running memscan and looking at swap
I run the stress test in 3 windows:
  - `htop`
  - `stressapptest`
  - `clear && ./memscan --map_mem=1G --fill --sleep=15 --path --phys`

Start `htop` and watch the `Mem` and `Swp` lines.  Run `stressapptest` then immediately start `memscan`

#### Notes
  - `stressapptest` only runs for 20 seconds
  - None of the options to `stressapptest` seemed to be working, but it consumed all the memory for 20 seconds and that's all we need.
  - This virtual machine was provisioned with 2G of memory

#### Output
Here's a screenshot of the setup:

<img src="examples/images/swap_screenshot_1.png" style="width:95%;" alt="Swap Screenshot 1"/>

Here's the full output:

    Unable to read[/proc/self/pagemap] for [0xffffffffff600000]
     0: 0x555bf828e000 - 0x555bf828ffff      8,192 r--p/media/sf_Src/src-clion/sre_lab4_memscan/memscan
        0x555bf828e000 - 0x555bf828ffff      8,192 Flags: *XF          \      IO: U LRU:L    M       System RAM 
     1: 0x555bf8290000 - 0x555bf8297fff     32,768 r-xp/media/sf_Src/src-clion/sre_lab4_memscan/memscan
        0x555bf8290000 - 0x555bf8290fff      4,096 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x555bf8291000 - 0x555bf8292fff      8,192 page not present
        0x555bf8293000 - 0x555bf8295fff     12,288 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x555bf8296000 - 0x555bf8297fff      8,192 page not present
     2: 0x555bf8298000 - 0x555bf829bfff     16,384 r--p/media/sf_Src/src-clion/sre_lab4_memscan/memscan
        0x555bf8298000 - 0x555bf8298fff      4,096 page not present
        0x555bf8299000 - 0x555bf8299fff      4,096 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x555bf829a000 - 0x555bf829bfff      8,192 page not present
     3: 0x555bf829c000 - 0x555bf829cfff      4,096 r--p/media/sf_Src/src-clion/sre_lab4_memscan/memscan
        0x555bf829c000 - 0x555bf829cfff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
     4: 0x555bf829d000 - 0x555bf829dfff      4,096 rw-p/media/sf_Src/src-clion/sre_lab4_memscan/memscan
        0x555bf829d000 - 0x555bf829dfff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
     5: 0x555bf829e000 - 0x555bf82e8fff    307,200 rw-p
        0x555bf829e000 - 0x555bf829ffff      8,192 page not present
        0x555bf82a0000 - 0x555bf82e8fff    299,008 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
     6: 0x555bf9f87000 - 0x555bf9fa7fff    135,168 rw-p[heap]
        0x555bf9f87000 - 0x555bf9f89fff     12,288 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x555bf9f8a000 - 0x555bf9f8cfff     12,288 Flags: *X           \      IO: U LRU: A   MA B    System RAM 
        0x555bf9f8d000 - 0x555bf9fa7fff    110,592 page not present
     7: 0x7fa75b200000 - 0x7fa79b1fffff 1,073,741,824 rw-p
        0x7fa75b200000 - 0x7fa7661fffff 184,549,376 swapped: type: 0  
        0x7fa766200000 - 0x7fa766392fff  1,650,688 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa766393000 - 0x7fa77a3fffff 335,990,784 swapped: type: 0  
        0x7fa77a400000 - 0x7fa77a556fff  1,404,928 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa77a557000 - 0x7fa77a562fff     49,152 swapped: type: 0  
        0x7fa77a563000 - 0x7fa77a5d6fff    475,136 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa77a5d7000 - 0x7fa77a5e2fff     49,152 swapped: type: 0  
        0x7fa77a5e3000 - 0x7fa77a5fffff    118,784 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa77a600000 - 0x7fa77a9fffff  4,194,304 swapped: type: 0  
        0x7fa77aa00000 - 0x7fa77aa00fff      4,096 Flags: *X           \ T<   IO: U LRU:L    MA B    System RAM 
        0x7fa77aa01000 - 0x7fa77abfffff  2,093,056 Flags: *X           \ T >  IO:   LRU:     MA      System RAM 
        0x7fa77ac00000 - 0x7fa77ac00fff      4,096 Flags: *X           \ T<   IO: U LRU:L    MA B    System RAM 
        0x7fa77ac01000 - 0x7fa77adfffff  2,093,056 Flags: *X           \ T >  IO:   LRU:     MA      System RAM 
        0x7fa77ae00000 - 0x7fa788e3ffff 235,143,168 swapped: type: 0  
        0x7fa788e40000 - 0x7fa788f13fff    868,352 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa788f14000 - 0x7fa78a91ffff 27,312,128 swapped: type: 0  
        0x7fa78a920000 - 0x7fa78ac96fff  3,633,152 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78ac97000 - 0x7fa78acd6fff    262,144 swapped: type: 0  
        0x7fa78acd7000 - 0x7fa78ad40fff    434,176 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78ad41000 - 0x7fa78ad80fff    262,144 swapped: type: 0  
        0x7fa78ad81000 - 0x7fa78adfffff    520,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78ae00000 - 0x7fa78c9acfff 29,020,160 swapped: type: 0  
        0x7fa78c9ad000 - 0x7fa78c9b8fff     49,152 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78c9b9000 - 0x7fa78c9f8fff    262,144 swapped: type: 0  
        0x7fa78c9f9000 - 0x7fa78c9fffff     28,672 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78ca00000 - 0x7fa78d1bbfff  8,110,080 swapped: type: 0  
        0x7fa78d1bc000 - 0x7fa78d1d5fff    106,496 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78d1d6000 - 0x7fa78d679fff  4,866,048 swapped: type: 0  
        0x7fa78d67a000 - 0x7fa78d68bfff     73,728 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78d68c000 - 0x7fa78e63ffff 16,465,920 swapped: type: 0  
        0x7fa78e640000 - 0x7fa78e675fff    221,184 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78e676000 - 0x7fa78ea3ffff  3,973,120 swapped: type: 0  
        0x7fa78ea40000 - 0x7fa78ea9cfff    380,928 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa78ea9d000 - 0x7fa78fbfffff 18,231,296 swapped: type: 0  
        0x7fa78fc00000 - 0x7fa7900e8fff  5,148,672 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7900e9000 - 0x7fa795094fff 83,542,016 swapped: type: 0  
        0x7fa795095000 - 0x7fa79518afff  1,007,616 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79518b000 - 0x7fa795190fff     24,576 swapped: type: 0  
        0x7fa795191000 - 0x7fa7951e3fff    339,968 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7951e4000 - 0x7fa7951f0fff     53,248 swapped: type: 0  
        0x7fa7951f1000 - 0x7fa7952b9fff    823,296 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7952ba000 - 0x7fa7952bcfff     12,288 swapped: type: 0  
        0x7fa7952bd000 - 0x7fa7952f3fff    225,280 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7952f4000 - 0x7fa7952fcfff     36,864 swapped: type: 0  
        0x7fa7952fd000 - 0x7fa7955e0fff  3,031,040 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7955e1000 - 0x7fa7955e7fff     28,672 swapped: type: 0  
        0x7fa7955e8000 - 0x7fa7956fdfff  1,138,688 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7956fe000 - 0x7fa7956fffff      8,192 swapped: type: 0  
        0x7fa795700000 - 0x7fa7957eafff    962,560 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7957eb000 - 0x7fa7957f5fff     45,056 swapped: type: 0  
        0x7fa7957f6000 - 0x7fa795a5afff  2,510,848 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795a5b000 - 0x7fa795a5ffff     20,480 swapped: type: 0  
        0x7fa795a60000 - 0x7fa795adafff    503,808 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795adb000 - 0x7fa795adffff     20,480 swapped: type: 0  
        0x7fa795ae0000 - 0x7fa795b5bfff    507,904 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795b5c000 - 0x7fa795b5ffff     16,384 swapped: type: 0  
        0x7fa795b60000 - 0x7fa795b9bfff    245,760 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795b9c000 - 0x7fa795b9ffff     16,384 swapped: type: 0  
        0x7fa795ba0000 - 0x7fa795bd8fff    233,472 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795bd9000 - 0x7fa795bdffff     28,672 swapped: type: 0  
        0x7fa795be0000 - 0x7fa795c75fff    614,400 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795c76000 - 0x7fa795c76fff      4,096 swapped: type: 0  
        0x7fa795c77000 - 0x7fa795c9dfff    159,744 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795c9e000 - 0x7fa795c9ffff      8,192 swapped: type: 0  
        0x7fa795ca0000 - 0x7fa795cddfff    253,952 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795cde000 - 0x7fa795cdffff      8,192 swapped: type: 0  
        0x7fa795ce0000 - 0x7fa795fbbfff  2,998,272 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa795fbc000 - 0x7fa795fbefff     12,288 swapped: type: 0  
        0x7fa795fbf000 - 0x7fa7967d4fff  8,478,720 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7967d5000 - 0x7fa7967e2fff     57,344 swapped: type: 0  
        0x7fa7967e3000 - 0x7fa7968b2fff    851,968 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7968b3000 - 0x7fa7968bffff     53,248 swapped: type: 0  
        0x7fa7968c0000 - 0x7fa7968d3fff     81,920 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7968d4000 - 0x7fa7968dffff     49,152 swapped: type: 0  
        0x7fa7968e0000 - 0x7fa796970fff    593,920 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa796971000 - 0x7fa79697afff     40,960 swapped: type: 0  
        0x7fa79697b000 - 0x7fa7969cffff    348,160 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7969d0000 - 0x7fa7969dafff     45,056 swapped: type: 0  
        0x7fa7969db000 - 0x7fa796c79fff  2,748,416 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa796c7a000 - 0x7fa796c7ffff     24,576 swapped: type: 0  
        0x7fa796c80000 - 0x7fa796cfefff    520,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa796cff000 - 0x7fa796d04fff     24,576 swapped: type: 0  
        0x7fa796d05000 - 0x7fa796d5cfff    360,448 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa796d5d000 - 0x7fa796d64fff     32,768 swapped: type: 0  
        0x7fa796d65000 - 0x7fa796f3dfff  1,937,408 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa796f3e000 - 0x7fa796f46fff     36,864 swapped: type: 0  
        0x7fa796f47000 - 0x7fa7974c4fff  5,758,976 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7974c5000 - 0x7fa7974d0fff     49,152 swapped: type: 0  
        0x7fa7974d1000 - 0x7fa797c59fff  7,901,184 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa797c5a000 - 0x7fa797c5ffff     24,576 swapped: type: 0  
        0x7fa797c60000 - 0x7fa797d19fff    761,856 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa797d1a000 - 0x7fa797d1ffff     24,576 swapped: type: 0  
        0x7fa797d20000 - 0x7fa797db3fff    606,208 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa797db4000 - 0x7fa797db5fff      8,192 swapped: type: 0  
        0x7fa797db6000 - 0x7fa797dfafff    282,624 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa797dfb000 - 0x7fa797dfffff     20,480 swapped: type: 0  
        0x7fa797e00000 - 0x7fa797e35fff    221,184 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa797e36000 - 0x7fa797e3ffff     40,960 swapped: type: 0  
        0x7fa797e40000 - 0x7fa7983b7fff  5,734,400 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7983b8000 - 0x7fa7983bffff     32,768 swapped: type: 0  
        0x7fa7983c0000 - 0x7fa7983d6fff     94,208 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa7983d7000 - 0x7fa7983dffff     36,864 swapped: type: 0  
        0x7fa7983e0000 - 0x7fa799076fff 13,201,408 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa799077000 - 0x7fa79907bfff     20,480 swapped: type: 0  
        0x7fa79907c000 - 0x7fa799986fff  9,482,240 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa799987000 - 0x7fa799987fff      4,096 swapped: type: 0  
        0x7fa799988000 - 0x7fa799ff6fff  6,746,112 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa799ff7000 - 0x7fa799ffffff     36,864 swapped: type: 0  
        0x7fa79a000000 - 0x7fa79a0ddfff    909,312 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a0de000 - 0x7fa79a0ebfff     57,344 swapped: type: 0  
        0x7fa79a0ec000 - 0x7fa79a4f6fff  4,239,360 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a4f7000 - 0x7fa79a4f8fff      8,192 swapped: type: 0  
        0x7fa79a4f9000 - 0x7fa79a536fff    253,952 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a537000 - 0x7fa79a538fff      8,192 swapped: type: 0  
        0x7fa79a539000 - 0x7fa79a58cfff    344,064 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a58d000 - 0x7fa79a598fff     49,152 swapped: type: 0  
        0x7fa79a599000 - 0x7fa79a5f5fff    380,928 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a5f6000 - 0x7fa79a5f8fff     12,288 swapped: type: 0  
        0x7fa79a5f9000 - 0x7fa79a854fff  2,473,984 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a855000 - 0x7fa79a85ffff     45,056 swapped: type: 0  
        0x7fa79a860000 - 0x7fa79a9aafff  1,355,776 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79a9ab000 - 0x7fa79a9b7fff     53,248 swapped: type: 0  
        0x7fa79a9b8000 - 0x7fa79b1fffff  8,683,520 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
     8: 0x7fa79b200000 - 0x7fa79b4e8fff  3,051,520 r--p/usr/lib/locale/locale-archive excluded
        0x7fa79b200000 - 0x7fa79b218fff    102,400 page not present
        0x7fa79b219000 - 0x7fa79b21bfff     12,288 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b21c000 - 0x7fa79b21ffff     16,384 page not present
        0x7fa79b220000 - 0x7fa79b220fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b221000 - 0x7fa79b222fff      8,192 page not present
        0x7fa79b223000 - 0x7fa79b223fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b224000 - 0x7fa79b4dffff  2,867,200 page not present
        0x7fa79b4e0000 - 0x7fa79b4e0fff      4,096 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x7fa79b4e1000 - 0x7fa79b4e1fff      4,096 page not present
        0x7fa79b4e2000 - 0x7fa79b4e8fff     28,672 Flags: *XF          \      IO: U LRU:L    M       System RAM 
     9: 0x7fa79b65b000 - 0x7fa79b65dfff     12,288 rw-p
        0x7fa79b65b000 - 0x7fa79b65cfff      8,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79b65d000 - 0x7fa79b65dfff      4,096 page not present
    10: 0x7fa79b65e000 - 0x7fa79b67ffff    139,264 r--p/usr/lib/libc.so.6
        0x7fa79b65e000 - 0x7fa79b66dfff     65,536 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x7fa79b66e000 - 0x7fa79b66ffff      8,192 page not present
        0x7fa79b670000 - 0x7fa79b67dfff     57,344 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x7fa79b67e000 - 0x7fa79b67efff      4,096 page not present
        0x7fa79b67f000 - 0x7fa79b67ffff      4,096 Flags: *XF          \      IO: U LRU:L    M       System RAM 
    11: 0x7fa79b680000 - 0x7fa79b7dafff  1,421,312 r-xp/usr/lib/libc.so.6
        0x7fa79b680000 - 0x7fa79b681fff      8,192 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b682000 - 0x7fa79b687fff     24,576 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b688000 - 0x7fa79b688fff      4,096 page not present
        0x7fa79b689000 - 0x7fa79b68efff     24,576 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b68f000 - 0x7fa79b68ffff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b690000 - 0x7fa79b69ffff     65,536 page not present
        0x7fa79b6a0000 - 0x7fa79b6a0fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6a1000 - 0x7fa79b6a2fff      8,192 page not present
        0x7fa79b6a3000 - 0x7fa79b6a3fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6a4000 - 0x7fa79b6a5fff      8,192 page not present
        0x7fa79b6a6000 - 0x7fa79b6a9fff     16,384 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6aa000 - 0x7fa79b6abfff      8,192 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6ac000 - 0x7fa79b6b0fff     20,480 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6b1000 - 0x7fa79b6b3fff     12,288 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6b4000 - 0x7fa79b6b4fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6b5000 - 0x7fa79b6b6fff      8,192 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6b7000 - 0x7fa79b6befff     32,768 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6bf000 - 0x7fa79b6bffff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6c0000 - 0x7fa79b6cffff     65,536 page not present
        0x7fa79b6d0000 - 0x7fa79b6d6fff     28,672 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6d7000 - 0x7fa79b6d7fff      4,096 page not present
        0x7fa79b6d8000 - 0x7fa79b6d8fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6d9000 - 0x7fa79b6d9fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6da000 - 0x7fa79b6ddfff     16,384 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6de000 - 0x7fa79b6eafff     53,248 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b6eb000 - 0x7fa79b6effff     20,480 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b6f0000 - 0x7fa79b6fffff     65,536 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b700000 - 0x7fa79b72afff    176,128 page not present
        0x7fa79b72b000 - 0x7fa79b72bfff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b72c000 - 0x7fa79b72cfff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b72d000 - 0x7fa79b72ffff     12,288 page not present
        0x7fa79b730000 - 0x7fa79b730fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b731000 - 0x7fa79b734fff     16,384 Flags: *XF          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b735000 - 0x7fa79b73ffff     45,056 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x7fa79b740000 - 0x7fa79b749fff     40,960 page not present
        0x7fa79b74a000 - 0x7fa79b74afff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b74b000 - 0x7fa79b74ffff     20,480 page not present
        0x7fa79b750000 - 0x7fa79b753fff     16,384 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x7fa79b754000 - 0x7fa79b755fff      8,192 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b756000 - 0x7fa79b758fff     12,288 page not present
        0x7fa79b759000 - 0x7fa79b75afff      8,192 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b75b000 - 0x7fa79b75bfff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b75c000 - 0x7fa79b75cfff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b75d000 - 0x7fa79b75dfff      4,096 page not present
        0x7fa79b75e000 - 0x7fa79b75efff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b75f000 - 0x7fa79b75ffff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b760000 - 0x7fa79b765fff     24,576 page not present
        0x7fa79b766000 - 0x7fa79b766fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b767000 - 0x7fa79b767fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b768000 - 0x7fa79b7affff    294,912 page not present
        0x7fa79b7b0000 - 0x7fa79b7b0fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7b1000 - 0x7fa79b7b2fff      8,192 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7b3000 - 0x7fa79b7b3fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7b4000 - 0x7fa79b7b5fff      8,192 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7b6000 - 0x7fa79b7bffff     40,960 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7c0000 - 0x7fa79b7cffff     65,536 page not present
        0x7fa79b7d0000 - 0x7fa79b7dafff     45,056 Flags: *XF          \      IO: U LRU:L    M       System RAM 
    12: 0x7fa79b7db000 - 0x7fa79b831fff    356,352 r--p/usr/lib/libc.so.6
        0x7fa79b7db000 - 0x7fa79b7dbfff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7dc000 - 0x7fa79b7ddfff      8,192 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7de000 - 0x7fa79b7e3fff     24,576 Flags: *XF          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7e4000 - 0x7fa79b7e8fff     20,480 page not present
        0x7fa79b7e9000 - 0x7fa79b7e9fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7ea000 - 0x7fa79b7ebfff      8,192 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7ec000 - 0x7fa79b7edfff      8,192 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7ee000 - 0x7fa79b7f1fff     16,384 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7f2000 - 0x7fa79b7f4fff     12,288 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7f5000 - 0x7fa79b7f5fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7f6000 - 0x7fa79b7f6fff      4,096 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7f7000 - 0x7fa79b7f7fff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7f8000 - 0x7fa79b7f9fff      8,192 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7fa000 - 0x7fa79b7fafff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b7fb000 - 0x7fa79b7fefff     16,384 Flags: * F          \      IO: U LRU:L    M       System RAM 
        0x7fa79b7ff000 - 0x7fa79b7fffff      4,096 Flags: * F          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b800000 - 0x7fa79b831fff    204,800 page not present
    13: 0x7fa79b832000 - 0x7fa79b835fff     16,384 r--p/usr/lib/libc.so.6
        0x7fa79b832000 - 0x7fa79b832fff      4,096 swapped: type: 0  
        0x7fa79b833000 - 0x7fa79b833fff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79b834000 - 0x7fa79b834fff      4,096 swapped: type: 0  
        0x7fa79b835000 - 0x7fa79b835fff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    14: 0x7fa79b836000 - 0x7fa79b837fff      8,192 rw-p/usr/lib/libc.so.6
        0x7fa79b836000 - 0x7fa79b837fff      8,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    15: 0x7fa79b838000 - 0x7fa79b844fff     53,248 rw-p
        0x7fa79b838000 - 0x7fa79b838fff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79b839000 - 0x7fa79b83bfff     12,288 page not present
        0x7fa79b83c000 - 0x7fa79b83cfff      4,096 swapped: type: 0  
        0x7fa79b83d000 - 0x7fa79b83efff      8,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fa79b83f000 - 0x7fa79b843fff     20,480 page not present
        0x7fa79b844000 - 0x7fa79b844fff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    16: 0x7fa79b845000 - 0x7fa79b847fff     12,288 r--p/usr/lib/libcap.so.2.66
        0x7fa79b845000 - 0x7fa79b847fff     12,288 Flags: *XF          \      IO: U LRU:L    M       System RAM 
    17: 0x7fa79b848000 - 0x7fa79b84cfff     20,480 r-xp/usr/lib/libcap.so.2.66
        0x7fa79b848000 - 0x7fa79b84cfff     20,480 page not present
    18: 0x7fa79b84d000 - 0x7fa79b84efff      8,192 r--p/usr/lib/libcap.so.2.66
        0x7fa79b84d000 - 0x7fa79b84efff      8,192 page not present
    19: 0x7fa79b84f000 - 0x7fa79b84ffff      4,096 r--p/usr/lib/libcap.so.2.66
        0x7fa79b84f000 - 0x7fa79b84ffff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    20: 0x7fa79b850000 - 0x7fa79b850fff      4,096 rw-p/usr/lib/libcap.so.2.66
        0x7fa79b850000 - 0x7fa79b850fff      4,096 swapped: type: 0  
    21: 0x7fa79b851000 - 0x7fa79b85efff     57,344 r--p/usr/lib/libm.so.6
        0x7fa79b851000 - 0x7fa79b85efff     57,344 Flags: *XF          \      IO: U LRU:L    M       System RAM 
    22: 0x7fa79b85f000 - 0x7fa79b8d8fff    499,712 r-xp/usr/lib/libm.so.6
        0x7fa79b85f000 - 0x7fa79b8d8fff    499,712 page not present
    23: 0x7fa79b8d9000 - 0x7fa79b936fff    385,024 r--p/usr/lib/libm.so.6
        0x7fa79b8d9000 - 0x7fa79b936fff    385,024 page not present
    24: 0x7fa79b937000 - 0x7fa79b937fff      4,096 r--p/usr/lib/libm.so.6
        0x7fa79b937000 - 0x7fa79b937fff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    25: 0x7fa79b938000 - 0x7fa79b938fff      4,096 rw-p/usr/lib/libm.so.6
        0x7fa79b938000 - 0x7fa79b938fff      4,096 swapped: type: 0  
    26: 0x7fa79b939000 - 0x7fa79b93afff      8,192 rw-p
        0x7fa79b939000 - 0x7fa79b93afff      8,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    27: 0x7fa79b942000 - 0x7fa79b942fff      4,096 r--p/usr/lib/ld-linux-x86-64.so.2
        0x7fa79b942000 - 0x7fa79b942fff      4,096 page not present
    28: 0x7fa79b943000 - 0x7fa79b968fff    155,648 r-xp/usr/lib/ld-linux-x86-64.so.2
        0x7fa79b943000 - 0x7fa79b95afff     98,304 Flags: *XF          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b95b000 - 0x7fa79b95bfff      4,096 page not present
        0x7fa79b95c000 - 0x7fa79b968fff     53,248 Flags: *XF          \      IO: U LRU:L  R M       System RAM 
    29: 0x7fa79b969000 - 0x7fa79b972fff     40,960 r--p/usr/lib/ld-linux-x86-64.so.2
        0x7fa79b969000 - 0x7fa79b96efff     24,576 Flags: *XF          \      IO: U LRU:L  R M       System RAM 
        0x7fa79b96f000 - 0x7fa79b96ffff      4,096 page not present
        0x7fa79b970000 - 0x7fa79b972fff     12,288 Flags: *XF          \      IO: U LRU:L  R M       System RAM 
    30: 0x7fa79b973000 - 0x7fa79b974fff      8,192 r--p/usr/lib/ld-linux-x86-64.so.2
        0x7fa79b973000 - 0x7fa79b973fff      4,096 swapped: type: 0  
        0x7fa79b974000 - 0x7fa79b974fff      4,096 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    31: 0x7fa79b975000 - 0x7fa79b976fff      8,192 rw-p/usr/lib/ld-linux-x86-64.so.2
        0x7fa79b975000 - 0x7fa79b976fff      8,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
    32: 0x7fff4d18e000 - 0x7fff4d1aefff    135,168 rw-p[stack]
        0x7fff4d18e000 - 0x7fff4d1aafff    118,784 page not present
        0x7fff4d1ab000 - 0x7fff4d1acfff      8,192 Flags: *X           \      IO: U LRU:L    MA B    System RAM 
        0x7fff4d1ad000 - 0x7fff4d1adfff      4,096 page not present
        0x7fff4d1ae000 - 0x7fff4d1aefff      4,096 swapped: type: 0  
    33: 0x7fff4d1bc000 - 0x7fff4d1bffff     16,384 r--p[vvar] excluded
        0x7fff4d1bc000 - 0x7fff4d1bffff     16,384 page not present
    34: 0x7fff4d1c0000 - 0x7fff4d1c1fff      8,192 r-xp[vdso]
        0x7fff4d1c0000 - 0x7fff4d1c0fff      4,096 Flags: * F          \      IO:   LRU:   R M       System RAM 
        0x7fff4d1c1000 - 0x7fff4d1c1fff      4,096 page not present
    35: 0xffffffffff600000 - 0xffffffffff600fff      4,096 --xpread permission not set
        0xffffffffff600000 - 0xffffffffff600fff      4,096 virtual address was not read by pagemap

#### Observations
When a system is under memory pressure, idle processes will:
  - Read-only pages and zero-pages, it doesn't swap them, it simply evicts them and reports them as `page not present`
  - The 1G region `memscan` allocated was unevenly swapped... 
  - Even `libc` and `ld-linux` pages would be swapped out


## Debian
Here's a simple memscan on a Debian system running under WSL2

#### Output
Here's a screenshot from this command:  `# ./memscan --path --shannon`

<img src="examples/images/debian_screenshot_1.png" style="width:95%;" alt="Debian Screenshot 1"/>

Here's the full output from this command:  `# ./memscan --path --shannon --phys`

     0: 0x55b0cf5b9000 - 0x55b0cf5bafff      8,192 r--p H: 1.877 Unknown                 /home/mark/src/src-clion/sre_lab4_memscan/memscan
        0x55b0cf5b9000 - 0x55b0cf5bafff      8,192 Flags:  XF          \      IO: U LRU:L    M       ACPI Tables H: 0.572 Very low entropy
     1: 0x55b0cf5bb000 - 0x55b0cf5c1fff     28,672 r-xp H: 5.579 ARM code                /home/mark/src/src-clion/sre_lab4_memscan/memscan
        0x55b0cf5bb000 - 0x55b0cf5c1fff     28,672 Flags:  XF          \      IO: U LRU:L    M       ACPI Tables H: 5.210 ARM code
     2: 0x55b0cf5c2000 - 0x55b0cf5c5fff     16,384 r--p H: 5.116 ARM code                /home/mark/src/src-clion/sre_lab4_memscan/memscan
        0x55b0cf5c2000 - 0x55b0cf5c5fff     16,384 Flags:  XF          \      IO: U LRU:L    M       ACPI Tables H: 2.860 Unknown
     3: 0x55b0cf5c6000 - 0x55b0cf5c6fff      4,096 r--p H: 2.994 Unknown                 /home/mark/src/src-clion/sre_lab4_memscan/memscan
        0x55b0cf5c6000 - 0x55b0cf5c6fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 2.994 Unknown
     4: 0x55b0cf5c7000 - 0x55b0cf5c7fff      4,096 rw-p H: 1.991 Unknown                 /home/mark/src/src-clion/sre_lab4_memscan/memscan
        0x55b0cf5c7000 - 0x55b0cf5c7fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 1.982 Unknown
     5: 0x55b0cf5c8000 - 0x55b0cf612fff    307,200 rw-p H: 0.252 Very low entropy
        0x55b0cf5c8000 - 0x55b0cf5c9fff      8,192 Flags:       0      \      IO:   LRU:             System RAM H: 0.000 No entropy
        0x55b0cf5ca000 - 0x55b0cf60efff    282,624 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 0.000 No entropy
        0x55b0cf60f000 - 0x55b0cf612fff     16,384 Flags:  X           \      IO: U LRU:     MA B    ACPI Tables H: 0.003 Very low entropy
     6: 0x55b0cf6ed000 - 0x55b0cf70dfff    135,168 rw-p H: 0.439 Very low entropy        [heap]
        0x55b0cf6ed000 - 0x55b0cf6effff     12,288 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 2.281 Unknown
        0x55b0cf6f0000 - 0x55b0cf6f2fff     12,288 Flags:  X           \      IO: U LRU:     MA B    ACPI Tables H: 0.010 Very low entropy
        0x55b0cf6f3000 - 0x55b0cf70dfff    110,592 Flags:       0      \      IO:   LRU:             System RAM H: 0.000 No entropy
     7: 0x7faf2eecf000 - 0x7faf2f1b5fff  3,043,328 r--p /usr/lib/locale/locale-archive excluded
        0x7faf2eecf000 - 0x7faf2eedefff     65,536 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2eedf000 - 0x7faf2eedffff      4,096 page not present
        0x7faf2eee0000 - 0x7faf2eee6fff     28,672 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2eee7000 - 0x7faf2eee7fff      4,096 page not present
        0x7faf2eee8000 - 0x7faf2eefafff     77,824 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2eefb000 - 0x7faf2eefbfff      4,096 page not present
        0x7faf2eefc000 - 0x7faf2eefffff     16,384 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2ef00000 - 0x7faf2ef0ffff     65,536 page not present
        0x7faf2ef10000 - 0x7faf2ef11fff      8,192 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2ef12000 - 0x7faf2ef1ffff     57,344 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2ef20000 - 0x7faf2ef2ffff     65,536 page not present
        0x7faf2ef30000 - 0x7faf2ef3ffff     65,536 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2ef40000 - 0x7faf2f15ffff  2,228,224 page not present
        0x7faf2f160000 - 0x7faf2f161fff      8,192 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2f162000 - 0x7faf2f16ffff     57,344 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
        0x7faf2f170000 - 0x7faf2f1affff    262,144 page not present
        0x7faf2f1b0000 - 0x7faf2f1b5fff     24,576 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.000 No entropy
     8: 0x7faf2f1b6000 - 0x7faf2f1b7fff      8,192 rw-p H: 0.231 Very low entropy
        0x7faf2f1b6000 - 0x7faf2f1b7fff      8,192 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 0.083 Very low entropy
     9: 0x7faf2f1b8000 - 0x7faf2f1d9fff    139,264 r--p H: 4.364 English text in ASCII   /usr/lib/x86_64-linux-gnu/libc-2.31.so
        0x7faf2f1b8000 - 0x7faf2f1d9fff    139,264 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.774 Very low entropy
    10: 0x7faf2f1da000 - 0x7faf2f333fff  1,417,216 r-xp H: 6.406 Unknown                 /usr/lib/x86_64-linux-gnu/libc-2.31.so
        0x7faf2f1da000 - 0x7faf2f1defff     20,480 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 6.260 x86 code
        0x7faf2f1df000 - 0x7faf2f1dffff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 6.198 x86 code
        0x7faf2f1e0000 - 0x7faf2f214fff    217,088 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 5.667 x86 code
        0x7faf2f215000 - 0x7faf2f215fff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.913 x86 code
        0x7faf2f216000 - 0x7faf2f24bfff    221,184 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 5.413 ARM code
        0x7faf2f24c000 - 0x7faf2f24cfff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.192 ARM code
        0x7faf2f24d000 - 0x7faf2f264fff     98,304 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 6.307 x86 code
        0x7faf2f265000 - 0x7faf2f265fff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 6.225 x86 code
        0x7faf2f266000 - 0x7faf2f28cfff    159,744 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 6.295 x86 code
        0x7faf2f28d000 - 0x7faf2f299fff     53,248 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 6.210 x86 code
        0x7faf2f29a000 - 0x7faf2f2c9fff    196,608 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 6.254 x86 code
        0x7faf2f2ca000 - 0x7faf2f2cafff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 6.014 x86 code
        0x7faf2f2cb000 - 0x7faf2f2f4fff    172,032 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 5.621 x86 code
        0x7faf2f2f5000 - 0x7faf2f2f5fff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.679 x86 code
        0x7faf2f2f6000 - 0x7faf2f2fcfff     28,672 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 5.544 ARM code
        0x7faf2f2fd000 - 0x7faf2f305fff     36,864 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.335 ARM code
        0x7faf2f306000 - 0x7faf2f31dfff     98,304 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 5.925 x86 code
        0x7faf2f31e000 - 0x7faf2f31efff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.380 ARM code
        0x7faf2f31f000 - 0x7faf2f322fff     16,384 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 6.081 x86 code
        0x7faf2f323000 - 0x7faf2f333fff     69,632 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.752 x86 code
    11: 0x7faf2f334000 - 0x7faf2f382fff    323,584 r--p H: 5.460 ARM code                /usr/lib/x86_64-linux-gnu/libc-2.31.so
        0x7faf2f334000 - 0x7faf2f33bfff     32,768 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.900 Very low entropy
        0x7faf2f33c000 - 0x7faf2f33cfff      4,096 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 3.930 Unknown
        0x7faf2f33d000 - 0x7faf2f35efff    139,264 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 5.810 x86 code
        0x7faf2f35f000 - 0x7faf2f382fff    147,456 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 2.263 Unknown
    12: 0x7faf2f383000 - 0x7faf2f386fff     16,384 r--p H: 3.597 Unknown                 /usr/lib/x86_64-linux-gnu/libc-2.31.so
        0x7faf2f383000 - 0x7faf2f386fff     16,384 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 3.646 Unknown
    13: 0x7faf2f387000 - 0x7faf2f388fff      8,192 rw-p H: 2.652 Unknown                 /usr/lib/x86_64-linux-gnu/libc-2.31.so
        0x7faf2f387000 - 0x7faf2f388fff      8,192 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 2.130 Unknown
    14: 0x7faf2f389000 - 0x7faf2f38cfff     16,384 rw-p H: 0.266 Very low entropy
        0x7faf2f389000 - 0x7faf2f38afff      8,192 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 0.077 Very low entropy
        0x7faf2f38b000 - 0x7faf2f38cfff      8,192 Flags:       0      \      IO:   LRU:             System RAM H: 0.000 No entropy
    15: 0x7faf2f38d000 - 0x7faf2f392fff     24,576 r--p H: 3.769 Unknown                 /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
        0x7faf2f38d000 - 0x7faf2f392fff     24,576 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 1.230 Unknown
    16: 0x7faf2f393000 - 0x7faf2f3a2fff     65,536 r-xp H: 5.882 x86 code                /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
        0x7faf2f393000 - 0x7faf2f3a2fff     65,536 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 0.399 Very low entropy
    17: 0x7faf2f3a3000 - 0x7faf2f3a8fff     24,576 r--p H: 4.534 English text in ASCII   /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
        0x7faf2f3a3000 - 0x7faf2f3a8fff     24,576 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 1.037 Unknown
    18: 0x7faf2f3a9000 - 0x7faf2f3a9fff      4,096 r--p H: 1.506 Unknown                 /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
        0x7faf2f3a9000 - 0x7faf2f3a9fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 1.506 Unknown
    19: 0x7faf2f3aa000 - 0x7faf2f3aafff      4,096 rw-p H: 1.167 Unknown                 /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
        0x7faf2f3aa000 - 0x7faf2f3aafff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 1.167 Unknown
    20: 0x7faf2f3ab000 - 0x7faf2f3aefff     16,384 rw-p H: 0.013 Very low entropy
        0x7faf2f3ab000 - 0x7faf2f3adfff     12,288 Flags:       0      \      IO:   LRU:             System RAM H: 0.000 No entropy
        0x7faf2f3ae000 - 0x7faf2f3aefff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 0.046 Very low entropy
    21: 0x7faf2f3af000 - 0x7faf2f3b1fff     12,288 r--p H: 2.410 Unknown                 /usr/lib/x86_64-linux-gnu/libcap.so.2.44
        0x7faf2f3af000 - 0x7faf2f3b1fff     12,288 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 0.249 Very low entropy
    22: 0x7faf2f3b2000 - 0x7faf2f3b5fff     16,384 r-xp H: 5.520 ARM code                /usr/lib/x86_64-linux-gnu/libcap.so.2.44
        0x7faf2f3b2000 - 0x7faf2f3b5fff     16,384 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 2.381 Unknown
    23: 0x7faf2f3b6000 - 0x7faf2f3b7fff      8,192 r--p H: 4.398 English text in ASCII   /usr/lib/x86_64-linux-gnu/libcap.so.2.44
        0x7faf2f3b6000 - 0x7faf2f3b7fff      8,192 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 2.493 Unknown
    24: 0x7faf2f3b8000 - 0x7faf2f3b8fff      4,096 r--p H: 3.370 English text in UNICODE /usr/lib/x86_64-linux-gnu/libcap.so.2.44
        0x7faf2f3b8000 - 0x7faf2f3b8fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 3.370 English text in UNICODE
    25: 0x7faf2f3b9000 - 0x7faf2f3b9fff      4,096 rw-p H: 1.347 Unknown                 /usr/lib/x86_64-linux-gnu/libcap.so.2.44
        0x7faf2f3b9000 - 0x7faf2f3b9fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 1.347 Unknown
    26: 0x7faf2f3ba000 - 0x7faf2f3c6fff     53,248 r--p H: 4.357 English text in ASCII   /usr/lib/x86_64-linux-gnu/libm-2.31.so
        0x7faf2f3ba000 - 0x7faf2f3c6fff     53,248 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 0.424 Very low entropy
    27: 0x7faf2f3c7000 - 0x7faf2f460fff    630,784 r-xp H: 6.558 Unknown                 /usr/lib/x86_64-linux-gnu/libm-2.31.so
        0x7faf2f3c7000 - 0x7faf2f460fff    630,784 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 5.016 ARM code
    28: 0x7faf2f461000 - 0x7faf2f4fbfff    634,880 r--p H: 7.347 Unknown                 /usr/lib/x86_64-linux-gnu/libm-2.31.so
        0x7faf2f461000 - 0x7faf2f4fbfff    634,880 Flags:  XF          \      IO: U LRU:LA R M       ACPI Tables H: 0.639 Very low entropy
    29: 0x7faf2f4fc000 - 0x7faf2f4fcfff      4,096 r--p H: 0.832 Very low entropy        /usr/lib/x86_64-linux-gnu/libm-2.31.so
        0x7faf2f4fc000 - 0x7faf2f4fcfff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 0.832 Very low entropy
    30: 0x7faf2f4fd000 - 0x7faf2f4fdfff      4,096 rw-p H: 1.892 Unknown                 /usr/lib/x86_64-linux-gnu/libm-2.31.so
        0x7faf2f4fd000 - 0x7faf2f4fdfff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 1.892 Unknown
    31: 0x7faf2f4fe000 - 0x7faf2f4fffff      8,192 rw-p H: 2.937 Unknown
        0x7faf2f4fe000 - 0x7faf2f4fffff      8,192 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 2.940 Unknown
    32: 0x7faf2f50a000 - 0x7faf2f50afff      4,096 r--p H: 3.471 Unknown                 /usr/lib/x86_64-linux-gnu/ld-2.31.so
        0x7faf2f50a000 - 0x7faf2f50afff      4,096 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 3.471 Unknown
    33: 0x7faf2f50b000 - 0x7faf2f52afff    131,072 r-xp H: 6.236 x86 code                /usr/lib/x86_64-linux-gnu/ld-2.31.so
        0x7faf2f50b000 - 0x7faf2f52afff    131,072 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 1.537 Unknown
    34: 0x7faf2f52b000 - 0x7faf2f532fff     32,768 r--p H: 5.830 x86 code                /usr/lib/x86_64-linux-gnu/ld-2.31.so
        0x7faf2f52b000 - 0x7faf2f532fff     32,768 Flags:   F          \      IO: U LRU:LA R M       ACPI Tables H: 4.017 Unknown
    35: 0x7faf2f534000 - 0x7faf2f534fff      4,096 r--p H: 1.813 Unknown                 /usr/lib/x86_64-linux-gnu/ld-2.31.so
        0x7faf2f534000 - 0x7faf2f534fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 1.813 Unknown
    36: 0x7faf2f535000 - 0x7faf2f535fff      4,096 rw-p H: 0.822 Very low entropy        /usr/lib/x86_64-linux-gnu/ld-2.31.so
        0x7faf2f535000 - 0x7faf2f535fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 0.822 Very low entropy
    37: 0x7faf2f536000 - 0x7faf2f536fff      4,096 rw-p H: 2.041 Unknown
        0x7faf2f536000 - 0x7faf2f536fff      4,096 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 2.041 Unknown
    38: 0x7ffee93d3000 - 0x7ffee93f3fff    135,168 rw-p H: 0.352 Very low entropy        [stack]
        0x7ffee93d3000 - 0x7ffee93f0fff    122,880 Flags:       0      \      IO:   LRU:             System RAM H: 0.000 No entropy
        0x7ffee93f1000 - 0x7ffee93f2fff      8,192 Flags:  X           \      IO: U LRU:L    MA B    ACPI Tables H: 3.079 English text in UNICODE
        0x7ffee93f3000 - 0x7ffee93f3fff      4,096 Flags:  X           \      IO: U LRU:LA R MA B    ACPI Tables H: 3.741 Unknown
    39: 0x7ffee93f9000 - 0x7ffee93fcfff     16,384 r--p [vvar] excluded
        0x7ffee93f9000 - 0x7ffee93fcfff     16,384 page not present
    40: 0x7ffee93fd000 - 0x7ffee93fefff      8,192 r-xp H: 3.035 English text in UNICODE [vdso]
        0x7ffee93fd000 - 0x7ffee93fdfff      4,096 Flags:   F          \      IO:   LRU:     M       System RAM H: 5.118 ARM code
        0x7ffee93fe000 - 0x7ffee93fefff      4,096 Flags:  XF          \      IO:   LRU:     M       System RAM H: 0.201 Very low entropy

#### Observations
- Debian required `libpthread`, which ArchLinux did not.
  - On both systems `-lpthread` is specified at compile time.  On Debian, 
    `libpthread` is loaded into memory, but on Arch it's not.
- Shannon Entropy reports a lot of the code as `ARM code`.  It's not, it's all x86.
- The Debian system seems to load physical pages in memory marked `ACPI Tables`.  I 
  don't believe the data and I suspect it's because the Linux instance is running in
  a container (WSL2) on Windows.  `/proc/iomem` was never designed to faithfully report
  memory regions in such a convoluted setup.


## ARM system
Memscan running on a Raspberry Pi running Rasberian

TBD
