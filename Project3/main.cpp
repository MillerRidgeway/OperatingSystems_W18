/* COMP3361 Operating Systems Spring 2018
 * Program 2 Sample Solution
 * 
 * File:   main.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#include "MemoryAllocator.h"
#include "PageTableManager.h"
#include "Process.h"

#include <MMU.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <list>

using mem::MMU;

using std::cerr;
using std::cout;
using std::string;
using std::vector;

int main(int argc, char* argv[]) {
  // Create allocator and page table manager (these will be shared among all 
  // processes in programming assignment 2)
  mem::MMU memory(0x80);  // fixed memory size of 64 pages
  MemoryAllocator allocator(memory);
  PageTableManager ptm(memory, allocator);
  
  //Convert from char* to int for timeslice
  int ts = atoi(argv[1]);
 
  //Create process list
  std::list<Process> procs;

  //Append arguments to list
  for(int i = 2; i < argc; i++){
      procs.emplace_back(argv[i], memory, ptm, i - 1, ts);
  }
  
  //Start running processes - stop when they are complete
  std::list<Process>::iterator it = procs.begin();  
  while(procs.size() != 0){
      if(it == procs.end())
          it = std::next(it,1);
      
      it->Run();
      
      //If proces is terminated remove from the list
      if(it->isTerm())
          procs.erase(it++);
      else
          it++;
      
  }
}

