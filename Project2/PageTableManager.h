/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PageTableManager.h
 * Author: Miller Ridgeway
 *
 * Created on May 13, 2018, 6:49 PM
 */

#ifndef PAGETABLEMANAGER_H
#define PAGETABLEMANAGER_H


#include "MemoryAllocator.h"
#include <MMU.h>

class PageTableManager {
public:
    PageTableManager(mem::MMU &mem, MemoryAllocator &a);
    mem::Addr CreateProcessTable();
    void map(int num, mem::Addr vAddr);
    void swapPerm();
    
private:
    MemoryAllocator *alloc;
    mem::MMU *mem;
    std::vector<uint32_t> page_frames;
    
    mem::PMCB kernel;
    mem::PMCB user;
    
    bool created;

};

#endif /* PAGETABLEMANAGER_H */

