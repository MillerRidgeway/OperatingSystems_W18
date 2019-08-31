/* 
 * Processor Memory Control Block definition
 * 
 * The PMCB controls the current state of the MMU interface. It contains the
 * fields described below, which control how the processor interfaces with
 * memory.  In an actual hardware implementation, the contents of the PMCB
 * would be loaded into hardware registers.
 *
 * File:   PMCB.h
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#ifndef MEM_PMCB_H
#define MEM_PMCB_H

#include "PhysicalMemory.h"
#include "PageTable.h"

namespace mem {

class PMCB {
public:
  // Constructors (note that default copy and move constructors are allowed)

  PMCB()
  : page_table_base(0),
    operation_state(NONE),
    next_vaddress(0),
    remaining_count(0),
    user_buffer(nullptr) {
  };

  PMCB(Addr page_table_base_)
  : page_table_base(page_table_base_),
    operation_state(NONE),
    next_vaddress(0),
    remaining_count(0),
    user_buffer(nullptr) {
  };
  
  // Address in physical memory of page table.
  // Must point to the start of a page frame (multiple of 0x1000).
  // Page table always has 0x400 (1024) entries (exactly one page frame).
  Addr page_table_base;
  
  // Partial operation state.  This is set when an operation is unable
  // to complete due to a virtual memory fault (page fault, write permission 
  // fault, etc.). The address is the next virtual address to process, the count 
  // is the remaining byte count, and the user buffer is a pointer to the buffer 
  // supplied by the user.
  typedef enum { NONE, READ_OP, WRITE_OP } PMCB_op;
  PMCB_op operation_state;
  Addr next_vaddress;    // virtual address at which to resume
  Addr remaining_count;  // number of bytes left to process
  uint8_t *user_buffer;  // caller buffer virtual address
};

}  // namespace mem

#endif /* MEM_PMCB_H */

