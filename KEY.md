Key to Flags
============

@brief Documents the key to the flags memscan outputs with the
       `--pfn` and `--phys` command line options

````
memscan --phys will output physical pages like this:

0x55a5be863000 - 0x55a5be864fff page not present
0x55a5be865000 - 0x55a5be8abfff Flags: *X           \      IO: U LRU:L    MA B    System RAM
0x55a5be8ac000 - 0x55a5be8adfff Flags: *X           \      IO: U LRU: A   MA B    System RAM


memscan --pfn will output physical pages like this:

0x7ffca078e000 - pfn: 0x0279672 #:  1 Flags: *X           \      IO: U LRU:L    MA B    System RAM H: 0.564 Very low entropy
0x7ffca078f000 - pfn: 0x0214113 #:  1 Flags: *X           \      IO: U LRU:L    MA B    System RAM H: 3.082 English text in UNICODE
0x7ffca0790000 - pfn: 0x0118352 #:  1 Flags: *X           \      IO: U LRU:L    MA B    System RAM H: 1.190 Unknown


Each line starts with the virtual address region of the physical page.
It will report high-level messages like "page not present" or "page swapped".

pfn: Page Frame Number.  Essentially, the physical page.
#:   the number of times each page is mapped

Flags:
*:   page is soft-dirty (it's been written to recently)
X:   page exclusively mapped
F:   page is file mapped and not anonymously mapped
KSM: kernel samepage merging: Identical memory pages are dynamically shared
     between processes
0:   zero page for pfn_zero or huge_zero page
L:   locked for exclusive access, e.g. by undergoing read/write IO
S:   page is managed by the SLAB/SLOB/SLUB/SLQB kernel memory allocator
W:   writeback: page is being synced to disk
B:   a free memory block managed by the buddy system allocator
I:   idle: The page has not been accessed since it was marked idle
P:   the page is in use as a page table

\    the end of regular flags and the start of huge page flags
H:   huge TLB page
T:   transparent huge page:  Contiguous pages that form a huge allocation
<:   the head of a contiguous block of pages
>:   the tail of a contiguous block of pages
B:   page is tagged for balloon compaction

IO:  I/O flags
!:   IO error occurred
U:   page has up-to-date data for file backed page
D:   page page has been written to and contains new data

LRU: least recently used flags
L:   page is in one of the LRU lists
A:   active: page is in the active LRU list
U:   unevictable: It is pinned and not a candidate for reclamation
R:   referenced: page has been referenced since last LRU list enqueue/requeue
R:   reclaim: page will be reclaimed soon after its pageout IO completed
M:   memory mapped page
A:   anonymous: memory mapped page that is not part of a file
C:   swapCache: page is mapped to swap space, i.e. has an associated swap entry
B:   swapBacked: page is backed by swap/RAM

Rarely seen flags
N:   no page frame exists at the requested address
!:   hardware detected memory corruption: Don't touch this page

````
