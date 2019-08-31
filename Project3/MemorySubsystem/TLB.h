/* 
 * TLB - Translation Lookaside Buffer for MMU
 * 
 * The TLB caches recent MMU address translation results. It uses the LRU
 * replacement algorithm. The TLB should be flushed whenever there is a change
 * to the current page table, or when a different page table comes into use.
 * 
 * File:   TLB.h
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#ifndef MEM_TLB_H
#define MEM_TLB_H

#include "Exceptions.h"
#include "PageTable.h"

#include <unordered_map>

namespace mem {

class TLB {
public:
  /**
   * Constructor - create TLB with specified number of entries > 0
   * 
   * @param entry_count number of TLB entries
   */
  TLB(size_t entry_count_);
  
  // Prevent copy/move/assign
  ~TLB() { }
  TLB(const TLB &other) = delete;  // no copy constructor
  TLB(TLB &&other) = delete;       // no move constructor
  TLB operator=(const TLB &other) = delete;  // no copy assign
  TLB operator=(TLB &&other) = delete;       // no move assign
  
  /**
   * Lookup - find mapping for specified virtual address
   * 
   * @param vaddr virtual address to look up
   * @return cached 2nd level page table entry for vaddr, or 0 if not in TLB
   */
  PageTableEntry Lookup(Addr vaddr);
  
  /**
   * Cache - store 2nd level page table entry for virtual address of page 
   *   starting at vaddr
   * 
   * @param vaddr starting virtual address of page
   * @param pt_entry 2nd level page table entry for page
   */
  void Cache(Addr vaddr, PageTableEntry pt_entry);
  
  /**
   * Flush - invalidate all TLB entries
   */
  void Flush();
  
/**
   * TLBStats - statistics on TLB operations
   */
  class TLBStats {
  public:
    // Constructor

    TLBStats()
    : recent_hits(0),
    recent_misses(0),
    recent_max_size(0),
    total_hits(0),
    total_misses(0),
    total_max_size(0) {
    }

    uint64_t recent_hits;     // count of TLB hits since last flush
    uint64_t recent_misses;   // count of TLB misses since last flush
    uint64_t recent_max_size; // max size of TLB since last flush
    uint64_t total_hits;      // count of total TLB hits
    uint64_t total_misses;    // count of total TLB misses
    uint64_t total_max_size;  // max size of TLB
  };
  
  /**
   * get_stats - get TLB statistics
   * 
   * @param stats set to a copy of the current TLB statistics
   */
  void get_stats(TLBStats &stats_) { stats_ = stats; }
  
private:
  /**
   * TLBEntry - contents of an entry in the TLB.  Private class used only
   *   inside of TLB.  TLB entries may be copied and moved.
   */
  class TLBEntry {
  public:
    // Constructors
    TLBEntry() : last_ref_seq(0), pt_entry(0) {}
    TLBEntry(uint64_t lrseq, PageTableEntry pte)
    : last_ref_seq(lrseq), pt_entry(pte) {
    }
    
    uint64_t last_ref_seq;        // last sequence number at which entry used
    PageTableEntry pt_entry;      // copy of 2nd level page table entry
  };
  
  /**
   * RemoveLRUEntry - remove Least Recently Used entry in TLB
   */
  void RemoveLRUEntry();
  
  // Max number of entries in TLB
  size_t entry_count;
  
  // Request sequence number, used by LRU replacement policy as pseudo-time
  uint64_t req_seq;
  
  // Since we can't implement a true associative memory in software, we emulate
  // one using a hash table (unordered_map), where the key is the virtual
  // address of the start of the page, and the value is the 2nd level page table
  // entry.
  std::unordered_map<Addr,TLBEntry> tlb_map;
  
  // TLB statistics
  TLBStats stats;
};

} // namespace mem

#endif /* MEM_TLB_H */

