/* 
 * Definitions for page table
 * 
 * MMU uses a single-level page table.  Each entry is 32 bits. The lower 16
 * bits are reserved for flag bits. The upper 16 bits contain the page frame
 * number.
 * 
 * A virtual address is 30 bits. It contains the offset in the page in the lower
 * 16 bits.  The next 14 bits contain the page table offset. The upper 2 bits 
 * are unused and are ignored.
 * 
 * File:   PageTable.h
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#ifndef MEM_PAGETABLE_H
#define MEM_PAGETABLE_H

#include "MemoryDefs.h"

#include <array>

namespace mem {

// Define storage for a single page table entry
typedef Addr PageTableEntry;

// Page table size
const int  kPageTableSizeBits = 14;
const Addr kPageTableEntries = (1 << kPageTableSizeBits);
static_assert(kPageTableEntries == kPageSize / sizeof(PageTableEntry),
              "Page size and page table size mismatch");
const Addr kPageTableSizeBytes = kPageSize;
const Addr kPageTableIndexMask = kPageTableEntries - 1;

// Define bit masks and shifts for fields in page table entry

// The page frame number is stored in the upper 16 bits
const uint32_t kPTE_FrameMask = kPageNumberMask;

// Bit masks for other flags
const uint32_t kPTE_Present = 0;            // page present in memory
const uint32_t kPTE_PresentMask = (1 << kPTE_Present);
const uint32_t kPTE_Writable = 1;           // page is writable
const uint32_t kPTE_WritableMask = (1 << kPTE_Writable);
const uint32_t kPTE_Accessed = 5;           // set when page is accessed
const uint32_t kPTE_AccessedMask = (1 << kPTE_Accessed);
const uint32_t kPTE_Modified = 6;           // set when page is modified
const uint32_t kPTE_ModifiedMask = (1 << kPTE_Modified);

// Define type for a page table as a derived class from std::array.
// The page table is initialized to zero.
class PageTable : public std::array<PageTableEntry, kPageTableEntries> {
public:
  PageTable() {
    fill(0);
  }
};

}  // namespace mem

#endif /* MEM_PAGETABLE_H */

