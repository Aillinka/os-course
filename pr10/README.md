# Practical Work 10 — Paging and Virtual Memory

## Goal
The goal is to understand paging, logical-to-physical address translation,
Translation Lookaside Buffer (TLB), and virtual memory in Linux.

## Tools
- Python 3
- C
- Matplotlib
- Linux Ubuntu
- mmap
- /proc/self/maps
- /proc/self/pagemap

## Variant A — Address Translation

Page size: 1024 bytes.
Logical address space: 16 pages.

The program calculates:
- Page number
- Offset
- Frame number
- Physical address

Formula:

page = logical_address // page_size
offset = logical_address % page_size
physical_address = frame * page_size + offset

If a page is not present in the page table, translation fails.

## Variant B — TLB and LRU

The program simulates a TLB with different capacities.
LRU removes the least recently used entry.

Metrics:
- TLB hits
- TLB misses
- Hit ratio
- Effective Access Time

The graph is saved as tlb_results.png.

Effective Access Time:

EAT = h * (T_TLB + T_M) + (1-h) * (T_TLB + 2*T_M)

The program also demonstrates a two-level page table.

## Variant C — Linux Virtual Memory

The program inspects /proc/self/maps and /proc/self/pagemap.
It also demonstrates file mapping with mmap.

Linux may hide physical frame numbers (PFN) for security reasons.

## TLB Experiment in C

The C program compares sequential memory access with page-stride access.
It measures execution time using clock().

Compile:

gcc -std=c11 -Wall -Wextra -O2 tlb_experiment.c -o tlb_experiment

Run:

./tlb_experiment

## Conclusion

Paging divides virtual memory into fixed-size pages and physical memory
into frames. The page table maps pages to frames.

TLB improves address translation by caching recent page table entries.
LRU replaces the least recently used entry.

Two-level page tables organize large address spaces more efficiently.
Linux exposes virtual memory mappings through /proc and supports
file-backed memory mapping through mmap.

## Variant C — Linux Memory Inspection

The program memory_inspect_safe.py examines virtual memory regions
using the /proc/self/maps interface.

The shared_memory.c program demonstrates shared memory between
a parent process and a child process using fork() and mmap().

Both processes access the same MAP_SHARED memory region.
The child modifies the data, and the parent reads the modification.

The tlb_experiment.c program compares memory access patterns:
- Sequential array traversal.
- Page-stride traversal.

The program measures execution time for both patterns.

Physical frame numbers in /proc/self/pagemap may be restricted
by Linux kernel permissions. Therefore, the experiment does not
depend on access to physical frame numbers.

## Conclusion

This practical work demonstrates paging, logical-to-physical
address translation, page table entries, TLB caching, LRU
replacement, effective access time, and shared memory.

The experiments show how TLB caching can reduce address
translation overhead and how Linux exposes process memory
regions through the /proc filesystem.
