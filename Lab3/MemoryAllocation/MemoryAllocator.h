/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MemoryAllocater.h
 * Author: Miller Ridgeway
 *
 * Created on April 13, 2018, 1:03 PM
 */

#ifndef MEMORYALLOCATOR_H
#define MEMORYALLOCATOR_H

#include <vector>
#include <stdint.h>
class MemoryAllocator {
public:
    MemoryAllocator(uint32_t numPages);
    
    //Rule of Five  
    ~MemoryAllocator();
    MemoryAllocator(const MemoryAllocator &w) = delete;
    MemoryAllocator& operator=(const MemoryAllocator &w) = delete;
    MemoryAllocator(MemoryAllocator &&w) = delete;
    MemoryAllocator& operator=(MemoryAllocator &&w) = delete;
    
    bool AllocatePageFrames(uint32_t count, std::vector<uint32_t> &page_frames);
    bool FreePageFrames(uint32_t count, std::vector<uint32_t> &page_frames);
    void PrintFreeList();
    uint32_t getPageFramesFree();
    
private:
    std::vector<uint8_t> mem;
    
    //'Pointers' to values within mem
    const int page_frames_total = 0x10;
    const int page_frames_free = 0x100;
    const int free_list_head = 0x1000;
    
};

#endif /* MEMORYALLOCATER_H */

