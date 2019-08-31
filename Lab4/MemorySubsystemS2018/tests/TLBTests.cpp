/* 
 * File:   TLBTests.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 *
 * Created on Dec. 3, 2017 04:17:00 PM
 */
#include "../TLB.h"

#include <gtest/gtest.h>

#include <random>

using mem::TLB;
using mem::Addr;
using mem::kPageSizeBits;
using mem::kPTE_PresentMask;
using mem::kPTE_WritableMask;
using mem::PageTableEntry;

using std::vector;
using std::string;

class TLBTests : public testing::Test {
protected:
  // Define class for address/page table entry pairs
  class TLBData {
  public:
    TLBData(Addr vaddr_, PageTableEntry pt_entry_)
    : vaddr(vaddr_), pt_entry(pt_entry_) {
    }
    
    Addr vaddr;
    PageTableEntry pt_entry;
  };
};

TEST_F(TLBTests, Constructor) {
  // Check initial state of TLB
  TLB tlb(8);
  
  // Make sure all stats are initialized to 0
  TLB::TLBStats stats;
  EXPECT_EQ(0, stats.recent_hits);
  EXPECT_EQ(0, stats.recent_max_size);
  EXPECT_EQ(0, stats.recent_misses);
  EXPECT_EQ(0, stats.total_hits);
  EXPECT_EQ(0, stats.total_max_size);
  EXPECT_EQ(0, stats.total_misses);
  
  // Get stats and make sure all come back as 0
  tlb.get_stats(stats);
  EXPECT_EQ(0, stats.recent_hits);
  EXPECT_EQ(0, stats.recent_max_size);
  EXPECT_EQ(0, stats.recent_misses);
  EXPECT_EQ(0, stats.total_hits);
  EXPECT_EQ(0, stats.total_max_size);
  EXPECT_EQ(0, stats.total_misses);
}

TEST_F(TLBTests, SimpleTest) {
  const int tlb_size = 8;
  TLB::TLBStats stats;
  
  // Set up some simple test data
  vector<TLBData> test_data;
  for (int i = 0; i < tlb_size*3; ++i) {
    test_data.push_back(TLBData((i+1) << kPageSizeBits, 
                                (0x27E02 - i) << kPageSizeBits));
  }
  
  // Make a set of queries, make sure they come right back
  TLB tlb(tlb_size);
  for (int i = 0; i < test_data.size(); ++i) {
    EXPECT_EQ(0, tlb.Lookup(test_data[i].vaddr));  // check return code for not in TLB
    tlb.Cache(test_data[i].vaddr, test_data[i].pt_entry);     // add entry to cache
    EXPECT_EQ(test_data[i].pt_entry, tlb.Lookup(test_data[i].vaddr));
    
    // Check removal of LRU entry
    if (i >= tlb_size) {
      EXPECT_EQ(0, tlb.Lookup(test_data[i-tlb_size].vaddr));
    }
    
    // Check statistics gathering
    tlb.get_stats(stats);
    EXPECT_EQ(i+1 + std::max(0, i-tlb_size+1), stats.recent_misses);
    EXPECT_EQ(i+1, stats.recent_hits);
    EXPECT_EQ(std::min(tlb_size, i+1), stats.recent_max_size);
  }
  
  // Make sure that oldest entries were pushed out
  for (int j = 0; j < (test_data.size() - tlb_size); ++j) {
    EXPECT_EQ(0, tlb.Lookup(test_data[j].vaddr));
  }
  
  // Check that newer entries are all in TLB
  for (int k = (test_data.size() - tlb_size); k < test_data.size(); ++k) {
    EXPECT_EQ(test_data[k].pt_entry, tlb.Lookup(test_data[k].vaddr));
  }
  
  // Check TLB stats
  tlb.get_stats(stats);
  EXPECT_EQ(test_data.size() + tlb_size, stats.recent_hits);
  EXPECT_EQ(test_data.size() * 2 - tlb_size + test_data.size() - tlb_size, stats.recent_misses);
  EXPECT_EQ(tlb_size, stats.recent_max_size);
  EXPECT_EQ(stats.recent_hits, stats.total_hits);
  EXPECT_EQ(stats.recent_misses, stats.total_misses);
  EXPECT_EQ(tlb_size, stats.total_max_size);
}

TEST_F(TLBTests, RandomTest) {
  const int tlb_size = 16;
  
  // Set up some simple test data
  vector<TLBData> test_data;
  for (int i = 0; i < tlb_size*4; ++i) {
    test_data.push_back(TLBData((i+1) << kPageSizeBits, 
                                (0x773F2 - i) << kPageSizeBits));
  }
  
  // Set up random number generator
  std::mt19937 gen;  // standard Mersenne Twister engine
  std::uniform_int_distribution<> rand_int(-tlb_size/8, tlb_size/8);
  
  // Make a large set of random queries, caching any values that are not found.
  // To get some locality, we generate a random offset from the last index,
  // positive or negative.
  TLB tlb(tlb_size);
  uint64_t hits1 = 0;
  uint64_t misses1 = 0;
  int t_index = 0;
  for (int j = 0; j < test_data.size()*64; ++j) {
    t_index = (t_index + rand_int(gen)) % test_data.size();
    const TLBData &mem_ref = test_data[t_index];
    
    // Try lookup of address
    PageTableEntry pt_entry = tlb.Lookup(mem_ref.vaddr);
    
    // If TLB hit, check return value
    if (pt_entry != 0) {
      ++hits1;
      EXPECT_EQ(mem_ref.pt_entry, pt_entry);
    } else {
      // Miss - put entry in cache
      ++misses1;
      tlb.Cache(mem_ref.vaddr, mem_ref.pt_entry);
    }
  }
  
  // Check stats
  TLB::TLBStats stats;
  tlb.get_stats(stats);
  EXPECT_EQ(hits1, stats.recent_hits);
  EXPECT_EQ(misses1, stats.recent_misses);
  EXPECT_EQ(tlb_size, stats.recent_max_size);
  EXPECT_EQ(hits1, stats.total_hits);
  EXPECT_EQ(misses1, stats.total_misses);
  EXPECT_EQ(tlb_size, stats.total_max_size);

  // Flush TLB and check stats
  tlb.Flush();
  tlb.get_stats(stats);
  EXPECT_EQ(0, stats.recent_hits);
  EXPECT_EQ(0, stats.recent_misses);
  EXPECT_EQ(0, stats.recent_max_size);
  EXPECT_EQ(hits1, stats.total_hits);
  EXPECT_EQ(misses1, stats.total_misses);
  EXPECT_EQ(tlb_size, stats.total_max_size);
  
  std::cout << "hits1 = " << hits1 << ", misses1 = " << misses1 << "\n";
  
  // Re-run and then re-check stats
  uint64_t hits2 = 0;
  uint64_t misses2 = 0;
  t_index = 0;
  for (int j = 0; j < test_data.size()*64; ++j) {
    t_index = (t_index + rand_int(gen)) % test_data.size();
    const TLBData &mem_ref = test_data[t_index];
    
    // Try lookup of address
    PageTableEntry pt_entry = tlb.Lookup(mem_ref.vaddr);
    
    // If TLB hit, check return value
    if (pt_entry != 0) {
      ++hits2;
      EXPECT_EQ(mem_ref.pt_entry, pt_entry);
    } else {
      // Miss - put entry in cache
      ++misses2;
      tlb.Cache(mem_ref.vaddr, mem_ref.pt_entry);
    }
  }
  
  // Check stats
  tlb.get_stats(stats);
  EXPECT_EQ(hits2, stats.recent_hits);
  EXPECT_EQ(misses2, stats.recent_misses);
  EXPECT_EQ(tlb_size, stats.recent_max_size);
  EXPECT_EQ(hits1+hits2, stats.total_hits);
  EXPECT_EQ(misses1+misses2, stats.total_misses);
  EXPECT_EQ(tlb_size, stats.total_max_size);

  std::cout << "hits2 = " << hits2 << ", misses2 = " << misses2 << "\n";  
}
