/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Miller Ridgeway
 *
 * Created on May 9, 2018, 10:57 AM
 */

#include <cstdlib>
#include <MMU.h>
#include "Process.h"
#include "MemoryAllocator.h"
#include "PageTableManager.h"
#include <iostream>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    //MMU
    mem::MMU unit(0x40);    
    
    //MemoryAllocator
    MemoryAllocator alloc(unit.get_frame_count(), unit);
  
    //Page Table Manager
    PageTableManager ptm(unit, alloc);
    
    //Run process
    Process p(argv[1]);
    p.run(unit, ptm);
   
    return 0;
}

