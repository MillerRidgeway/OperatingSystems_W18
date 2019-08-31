/* 
 * Interface to Virtual Memory (Memory Management Unit)
 * 
 * File:   MMU.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#include "MMU.h"

#include "Exceptions.h"

namespace mem {

void MMU::InitMemoryOperation(PMCB::PMCB_op op, 
                              Addr vaddress, 
                              Addr count, 
                              uint8_t* user_buffer) {
  pmcb.operation_state = op;
  pmcb.next_vaddress = vaddress;
  pmcb.remaining_count = count;
  pmcb.user_buffer = user_buffer;
}

void MMU::ToPhysical(Addr vaddress, Addr& paddress, bool write_op) {
  // If not in virtual memory mode, physical == virtual
  if (!virtual_mode) {
    paddress = vaddress;
    return;
  }
  
  // If address translation cached in TLB, use it
  PageTableEntry pt_entry = 0;  // page table entry contents
  Addr pt_entry_addr = 0xFFFFFFFF;  // page table entry address
  bool from_tlb = false;  // true if translation from TLB
  
  if (tlb) {
    pt_entry = tlb->Lookup(vaddress);
    // Use TLB entry if page present. If this is a write and the modified
    // bit is not set in the cache, force use of page table so that 
    // the modified bit will be updated in the page table.
    from_tlb = (pt_entry & kPTE_PresentMask) != 0
            && (!write_op || ((pt_entry & kPTE_ModifiedMask) != 0));
  }

  if (!from_tlb) {
    // Get entry from page table
    Addr pt_index = (vaddress >> kPageSizeBits) & kPageTableIndexMask;
    pt_entry_addr = pmcb.page_table_base + pt_index * sizeof(PageTableEntry);
    phys_mem.get_32(&pt_entry, pt_entry_addr);
  }

    // Check for page present
  if ((pt_entry & kPTE_PresentMask) == 0) {
    throw PageFaultException();
  }
  
  // If write operation and page not writable, throw exception
  if (write_op && (pt_entry & kPTE_WritableMask) == 0) {
    throw WritePermissionFaultException();
  }
  
  // If address not from TLB, set accessed and (optionally) modified flags 
  // in 2nd level table, then update the TLB
  if (!from_tlb) {
    PageTableEntry new_pt_entry = pt_entry | kPTE_AccessedMask
            | (write_op ? kPTE_ModifiedMask : 0);

    // If changed, write back to page table
    if (new_pt_entry != pt_entry) {
      pt_entry = new_pt_entry;
      phys_mem.put_bytes(pt_entry_addr, sizeof(PageTableEntry),
                        reinterpret_cast<uint8_t*>(&pt_entry));
    }
    
    // Update TLB
    if (tlb) {
      tlb->Cache(vaddress, pt_entry);
    }
  }
  
  // Page is mapped, return physical
  paddress = (pt_entry & kPTE_FrameMask) | (vaddress & kPageOffsetMask);
}

void MMU::Execute() {
  if (pmcb.operation_state == PMCB::NONE) return;
  
  if (pmcb.operation_state != PMCB::READ_OP && pmcb.operation_state != PMCB::WRITE_OP) {
    throw InvalidMMUOperationException("PMCB Error: operation is invalid");
  }
  
  while (pmcb.remaining_count > 0) {
    // Check if next page is mapped and has correct write permission
    Addr next_paddress;
    ToPhysical(pmcb.next_vaddress, next_paddress, 
               pmcb.operation_state == PMCB::WRITE_OP);
    
    // Determine remaining count within current page
    Addr count_in_page = std::min(pmcb.remaining_count,
                                  kPageSize - (pmcb.next_vaddress & kPageOffsetMask));
    
    // Transfer bytes
    if (pmcb.operation_state == PMCB::READ_OP) {
      phys_mem.get_bytes(pmcb.user_buffer, next_paddress, count_in_page);
    } else {  // write
      phys_mem.put_bytes(next_paddress, count_in_page, pmcb.user_buffer);
    }
    
    // Advance state of transfer
    pmcb.next_vaddress += count_in_page;
    pmcb.user_buffer += count_in_page;
    pmcb.remaining_count -= count_in_page;
  }
}

void MMU::get_byte(void *dest, Addr vaddress) {
  InitMemoryOperation(PMCB::READ_OP, vaddress, 1, 
          reinterpret_cast<uint8_t*>(dest));
  Execute();
}

void MMU::get_bytes(void *dest, Addr vaddress, Addr count) {
  InitMemoryOperation(PMCB::READ_OP, vaddress, count, 
          reinterpret_cast<uint8_t*>(dest));
  Execute();
}

void MMU::put_byte(Addr vaddress, void *src) {
  InitMemoryOperation(PMCB::WRITE_OP, vaddress, 1, 
          reinterpret_cast<uint8_t*>(src));
  Execute();  
}

void MMU::put_bytes(Addr vaddress, Addr count, void *src) {
  InitMemoryOperation(PMCB::WRITE_OP, vaddress, count, 
          reinterpret_cast<uint8_t*>(src));
  Execute();
}

void MMU::enter_virtual_mode() {
  if (!pmcb_initialized) {
    throw InvalidMMUOperationException(
            "PMCB not initialized before enter_virtual_mode") ;
  } else if (virtual_mode) {
    throw InvalidMMUOperationException(
            "enter_virtual_mode called when already in virtual mode");
  } else {
    virtual_mode = true;
  }
}

void MMU::set_PMCB(const PMCB &new_pmcb) {
  if ((new_pmcb.page_table_base & kPageOffsetMask) != 0) {
    throw InvalidMMUOperationException("page_table_base must be on a page boundary");
  }
  pmcb = new_pmcb;
  pmcb_initialized = true;
  if (pmcb.operation_state != PMCB::NONE && pmcb.remaining_count > 0) {
    Execute();
  }
}

void MMU::get_TLBStats(TLB::TLBStats& stats) {
  if (tlb.get() != nullptr) {
    tlb->get_stats(stats);
  } else {
    throw InvalidMMUOperationException("TLB is not enabled, stats not available");
  }
}

}  // namespace mem