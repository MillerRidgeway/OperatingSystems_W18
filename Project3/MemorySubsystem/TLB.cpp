/* 
 * TLB - Translation Look-aside Buffer for MMU
 * 
 * The TLB caches recent MMU address translation results. It uses the LRU
 * replacement algorithm. The TLB should be flushed whenever there is a change
 * to the current page table, or when a different page table comes into use.
 * 
 * File:   TLB.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#include "TLB.h"

using std::next;

namespace mem {

TLB::TLB(size_t entry_count_)
: entry_count(entry_count_), req_seq(0), tlb_map(entry_count_) {
  if(entry_count == 0) {
    throw InvalidMMUOperationException("TLB size specified as 0");
  }
}

PageTableEntry TLB::Lookup(Addr vaddr) {
  // Clear offset bits in vaddr
  Addr vaddr_page = vaddr & kPageNumberMask;
  
  // Try to find address in TLB
  auto tlb_loc = tlb_map.find(vaddr_page);
  
  // If found in TLB
  if (tlb_loc != tlb_map.end()) {
    ++stats.recent_hits;
    ++stats.total_hits;
    tlb_loc->second.last_ref_seq = ++req_seq; // update last reference
    return tlb_loc->second.pt_entry;          // return cached page table entry
  } else {
    // Not found in TLB
    ++stats.recent_misses;
    ++stats.total_misses;
    return static_cast<PageTableEntry>(0);
  }
}

void TLB::Cache(Addr vaddr, PageTableEntry pt_entry) {
  // Clear offset bits in vaddr
  Addr vaddr_page = vaddr & kPageNumberMask;

  // If entry already in TLB, update mapping and exit
  auto tlb_loc = tlb_map.find(vaddr_page);
  if (tlb_loc != tlb_map.end()) {  // if already in TLB
    tlb_loc->second.last_ref_seq = ++req_seq; // update last reference
    tlb_loc->second.pt_entry = pt_entry;      // update cached page table entry
    return;
  }
  
  // If TLB is full, remove an entry
  if (tlb_map.size() >= entry_count) {
    RemoveLRUEntry();
  }
  
  // Add new entry to TLB
  tlb_map[vaddr_page] = TLBEntry(++req_seq, pt_entry);
  
  // Update TLB size stats
  if (tlb_map.size() > stats.recent_max_size)
    stats.recent_max_size = tlb_map.size();
  if (stats.recent_max_size > stats.total_max_size)
    stats.total_max_size = stats.recent_max_size;
}

void TLB::Flush() {
  stats.recent_hits = stats.recent_misses = stats.recent_max_size = 0;
  tlb_map.clear();
}

void TLB::RemoveLRUEntry() {
  if (!tlb_map.empty()) {
    // Search sequentially through the TLB entries for the least recent access
    // time.
    //
    // TODO: see if another data structure such as a priority queue would be
    //       faster (normally the TLB is fairly small, so it may not make a
    //       difference).
    auto victim = tlb_map.begin();
    for (auto cur = next(tlb_map.begin()); cur != tlb_map.end(); ++cur) {
      if (cur->second.last_ref_seq < victim->second.last_ref_seq) {
        victim = cur;
      }
    }
    
    // Delete the victim (entry with oldest reference time)
    tlb_map.erase(victim);
  }
}

} // namespace mem