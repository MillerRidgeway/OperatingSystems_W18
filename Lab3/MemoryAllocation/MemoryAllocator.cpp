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

MemoryAllocator::MemoryAllocator(uint32_t numPages) {
    
    //Allocate memory to all 0's, number of pages by page size
    for(int i = 0; i < numPages*0x10000; i++){ 
        mem.push_back(0);
    }
    
    //Number of pages at 0x10
    memcpy(&mem[0] + page_frames_total, &numPages, sizeof(uint32_t));
    
    //Free frames at 0x100
    uint32_t freeFrames = numPages - 1;
    memcpy(&mem[0] + page_frames_free, &freeFrames, sizeof(uint32_t));
    
    //Free list head at 0x1000
    uint32_t head = 0x10000;
    memcpy(&mem[0] + free_list_head, &head, sizeof(uint32_t));
    
    //First four bytes point to next 'page', except for page 0
    for(int i = 1; i < numPages; i++){ 
        uint32_t temp = (i+1)*0x10000;
        if(i == numPages - 1)
            temp = 0xFFFFFFF;
        memcpy(&mem[0] + i*0x10000, &temp, sizeof(uint32_t));
    }
}

bool MemoryAllocator::AllocatePageFrames(uint32_t count, std::vector<uint32_t> &page_frames){
    if(count > getPageFramesFree()){
        return false;
    }
    
    //Push data in mem onto page_frames vector
    for(int i = 0; i < count; i++){
        uint32_t head = 0;
        memcpy(&head, &mem[0] + free_list_head, sizeof(uint32_t));
        page_frames.push_back(head);
        memcpy(&mem[0] + free_list_head, &mem[0] + head, sizeof(uint32_t));
    }
    
    //Change free frames to the correct value
    uint32_t freeFrames = 0;
    memcpy(&freeFrames, &mem[0] + page_frames_free, sizeof(uint32_t));
    freeFrames = freeFrames - count;
    memcpy(&mem[0] + page_frames_free, &freeFrames, sizeof(uint32_t));
    
    return true;
}

bool MemoryAllocator::FreePageFrames(uint32_t count, std::vector<uint32_t>& page_frames){
    if(count > page_frames.size()){
        return false;
    }
    
    //Take data off of page_frames vector and add back to free list
    for(int i = 0; i < count; i++)
    {
        uint32_t head = 0;
        memcpy(&head, &mem[0] + free_list_head, sizeof(uint32_t));
        memcpy(&mem[0] + free_list_head, &page_frames.back(), sizeof(uint32_t));
        page_frames.pop_back();
    }
    
    //Change free frames to the correct value
    uint32_t freeFrames = 0;
    memcpy(&freeFrames, &mem[0] + page_frames_free, sizeof(uint32_t));
    freeFrames = freeFrames + count;
    memcpy(&mem[0] + page_frames_free, &freeFrames, sizeof(uint32_t));
    
    return true;
}

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
    memcpy(&freeFrames, &mem[0] + page_frames_free, sizeof(uint32_t));
    return freeFrames;
}

MemoryAllocator::~MemoryAllocator() {
    //Blank Destructor 
}

