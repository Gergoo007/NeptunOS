PAT, or 'Page Attribute Table' 
	If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0.
G, or 'Global' 
	tells the processor not to invalidate the TLB entry corresponding to the page upon a MOV to CR3 instruction. Bit 7 (PGE) in CR4 must be set to enable global pages.
PS, or 'Page Size' 
	stores the page size for that specific entry. If the bit is set, then the PDE maps to a page that is 4 MiB in size. Otherwise, it maps to a 4 KiB page table. Please note that 4-MiB pages require PSE to be enabled.
D, or 'Dirty' 
	is used to determine whether a page has been written to.
A, or 'Accessed' 
	is used to discover whether a PDE or PTE was read during virtual address translation. If it has, then the bit is set, otherwise, it is not. Note that, this bit will not be cleared by the CPU, so that burden falls on the OS (if it needs this bit at all).
PCD, is the 'Cache Disable' 
	bit. If the bit is set, the page will not be cached. Otherwise, it will be.
PWT, controls Write-Through' 
	abilities of the page. If the bit is set, write-through caching is enabled. If not, then write-back is enabled instead.
U/S, the 'User/Supervisor' 
	bit, controls access to the page based on privilege level. If the bit is set, then the page may be accessed by all; if the bit is not set, however, only the supervisor can access it. For a page directory entry, the user bit controls access to all the pages referenced by the page directory entry. Therefore if you wish to make a page a user page, you must set the user bit in the relevant page directory entry as well as the page table entry.
R/W, the 'Read/Write' 
	permissions flag. If the bit is set, the page is read/write. Otherwise when it is not set, the page is read-only. The WP bit in CR0 determines if this is only applied to userland, always giving the kernel write access (the default) or both userland and the kernel (see Intel Manuals 3A 2-20).
P, or 'Present'. 
	If the bit is set, the page is actually in physical memory at the moment. For example, when a page is swapped out, it is not in physical memory and therefore not 'Present'. If a page is called, but not present, a page fault will occur, and the OS should handle it. (See below.)