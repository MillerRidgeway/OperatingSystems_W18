/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MemoryAllocater.cpp
 * Author: Miller Ridgeway
 * 
 * Created on April 13, 2018, 1:03 PM
 */

#include <vector>
#include "MemoryAllocator.h"
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <MMU.h>

/*
 * MemoryAllocator - Tracks free and occupied pages in memory
 * @param numPages - Number of frames to observe in memory
 * @param m - Memory manager of type MMU
 */
MemoryAllocator::MemoryAllocator(uint32_t numPages, mem::MMU &m) {
    this->m = &m;
    
    //Allocate memory to all 0's, number of pages by page size
    for(int i = 0; i < numPages*0x10000; i++){ 
        mem.push_back(0);
    }
    m.put_bytes(0,sizeof(mem),&mem);
    
    //Number of pages at 0x10
    m.put_bytes(0 + page_frames_total, sizeof(uint32_t), &numPages);
    
    //Free frames at 0x100
    uint32_t freeFrames = numPages - 1;
    m.put_bytes(0 + page_frames_free, sizeof(uint32_t), &freeFrames);
    
    //Free list head at 0x1000
    uint32_t head = 0x10000;
    m.put_bytes(0 + free_list_head, sizeof(uint32_t), &head);
    
    //First four bytes point to next 'page', except for page 0
    for(int i = 1; i < numPages; i++){ 
        uint32_t temp = (i+1)*0x10000;
        if(i == numPages - 1)
            temp = 0xFFFFFFF;
        m.put_bytes(0 + i*0x10000, sizeof(uint32_t), &temp);
    }
}

/*
 * AllocatePageFrames - Allocates frames from physical space
 * @param count - number of frames to allocate
 * @param page_frames - Page frames array to track free & occupied frames
 */
bool MemoryAllocator::AllocatePageFrames(uint32_t count, std::vector<uint32_t> &page_frames){
    if(count > getPageFramesFree()){
        return false;
    }
    
    //Push data in mem onto page_frames vector
    for(int i = 0; i < count; i++){
        uint32_t head = 0;
        m->get_bytes(&head, 0 + free_list_head, sizeof(uint32_t));
        page_frames.push_back(head);
        
        uint32_t temp = 0;
        m->get_bytes(&temp, 0 + head, sizeof(uint32_t));
        m->put_bytes(0 + free_list_head, sizeof(uint32_t), &temp);
    }
    
    //Change free frames to the correct value
    uint32_t freeFrames = 0;
    m->get_bytes(&freeFrames, 0 + page_frames_free, sizeof(uint32_t));
    freeFrames = freeFrames - count;
    m->put_bytes(0 + page_frames_free, sizeof(uint32_t), &freeFrames);
    
    return true;
}
/*
 * FreePageFrames - Free page frames from physical memory space
 * @params - Same as AllocatePageFrames
 */
bool MemoryAllocator::FreePageFrames(uint32_t count, std::vector<uint32_t>& page_frames){
    if(count > page_frames.size()){
        return false;
    }
    
    //Take data off of page_frames vector and add back to free list
    for(int i = 0; i < count; i++)
    {
        uint32_t head = 0;
        m->get_bytes(&head, 0 + free_list_head, sizeof(uint32_t));
        m->put_bytes(0 + free_list_head, sizeof(uint32_t), &page_frames.back());
        page_frames.pop_back();
    }
    
    //Change free frames to the correct value
    uint32_t freeFrames = 0;
    m->get_bytes(&freeFrames, 0 + page_frames_free, sizeof(uint32_t));
    freeFrames = freeFrames + count;
    m->put_bytes(0 + page_frames_free, sizeof(uint32_t), &freeFrames);
    
    return true;
}
/*
 * PrintFreeList - Print free frames
 */
void MemoryAllocator::PrintFreeList(){
    uint32_t head = 0;
    uint32_t hold = 0;
    
    memcpy(&head, &mem[0] + free_list_head, sizeof(uint32_t));
    memcpy(&hold, &mem[0] + free_list_head, sizeof(uint32_t));
    
    while(head != 0xFFFFFFF)
    {
      std::cout << " " << std::hex << head;
      memcpy(&mem[0] + free_list_head, &mem[0] + head, sizeof(uint32_t));
      memcpy(&head, &mem[0] + free_list_head, sizeof(uint32_t));
    }
    
    memcpy(&mem[0] + free_list_head, &hold, sizeof(uint32_t));
    std::cout << std::endl;
}

uint32_t MemoryAllocator::getPageFramesFree(){
    uint32_t freeFrames = 0;
    m->get_bytes(&freeFrames, 0 + page_frames_free, sizeof(uint32_t));
    return freeFrames;
}

MemoryAllocator::~MemoryAllocator() {
    //Blank Destructor 
}

