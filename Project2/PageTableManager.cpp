/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PageTableManager.cpp
 * Author: Miller Ridgeway
 * 
 * Created on May 13, 2018, 6:49 PM
 * 
 * @param mem - An instance of the MMU class to manage memory
 * @param a - An instance of a memory allocator which tracks physical address space
 * @throws - Exceptions from the MMU library depending on violation
 */

#include "PageTableManager.h"
#include <MMU.h>
#include <PageTable.h>
#include "MemoryAllocator.h"
#include <iostream>

/*
 * BuildKernelPageTable - Build the kernel page table
 * @param vm - MMU instance
 * @param pt_addr - Physical address kernel starts at
 * @throws Exceptions from MMU library based on violation
 */

void BuildKernelPageTable(mem::MMU &vm, mem::Addr pt_addr) {
  mem::PageTable kernel_page_table;  // local copy of page table to build, initialized to 0
  mem::Addr num_pages = vm.get_frame_count();  // size of physical memory

  // Build page table entries
  for (mem::Addr i = 0; i < num_pages; ++i) {
    kernel_page_table.at(i) = 
            (i << mem::kPageSizeBits) | mem::kPTE_PresentMask | mem::kPTE_WritableMask;
  }

  // Write page table to memory
  vm.put_bytes(pt_addr, mem::kPageTableSizeBytes, &kernel_page_table);
}

PageTableManager::PageTableManager(mem::MMU& mem, MemoryAllocator& a){
    this->alloc = &a;
    this->mem = &mem;
    
    //Allocate page for kernel
    alloc->AllocatePageFrames(1,page_frames); 
    
    //Create kernel page table
    BuildKernelPageTable(mem, page_frames[0]);
    kernel = mem::PMCB(page_frames[0]);
    
    //Set PMCB 
    this->mem->set_PMCB(kernel);
    
    //Enter virtual mode
    this->mem->enter_virtual_mode();
}

/*
 * CreateProcessTable - Create page table for processes in memory
 * @param num - Number of frames to create within the table
 * @param vAddr Virtual address of page table
 * @throws MMU exceptions based on violation
 * @returns Base address of process page table
 */

mem::Addr PageTableManager::CreateProcessTable(){
    //Create blank user table
    mem::PageTable user_table;
   
    //Allocate pages
    alloc->AllocatePageFrames(1, page_frames);
    
    //Set PMCB location and copy to physical mem
    user = mem::PMCB(page_frames.back());
    mem->put_bytes(page_frames.back(), mem::kPageTableSizeBytes, &user_table);
    
    //Set creation flag
    created = true;

    return page_frames.back();
}
/*
 * map - Maps page frames from virtual memory into process page table
 * @param num - number of pages to map
 * @param vAddr - Virtual address start of frames to map
 * @throws MMU exceptions based on violation
 */
void PageTableManager::map(int num, mem::Addr vAddr){
    mem->set_PMCB(kernel);
    
    //Check if page table exists. If not, create it & allocate
    //If page table already exists, allocate additional pages
    mem::Addr userAddr;
    mem::PageTable ppt;
    
    if(created == false){
        userAddr = CreateProcessTable();
        mem->get_bytes(&ppt, userAddr, mem::kPageTableSizeBytes);
    }
    else{
        mem->get_bytes(&ppt, userAddr, mem::kPageTableSizeBytes);
    }
    
    //Allocate frames in process page table
    for(int i = 0; i < num; i++){
        mem::Addr phys = (vAddr + (i*4) >> mem::kPageSizeBits) & mem::kPageTableIndexMask;
        ppt.at(phys) = page_frames.back() | mem::kPTE_PresentMask | mem::kPTE_WritableMask;
        
        alloc->AllocatePageFrames(1, page_frames);
    }
     mem->put_bytes(userAddr, mem::kPageTableSizeBytes, &ppt);
     
    //Switch to user table for manipulation
    mem->set_PMCB(user);
}

