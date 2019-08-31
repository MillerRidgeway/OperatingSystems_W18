/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Process.cpp
 * Author: Miller Ridgeway
 * 
 * Created on April 5, 2018, 11:13 AM
 */

#include "Process.h"
#include <string>
#include <fstream>
#include <vector>
#include "stdint.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "MMU.h"
#include "PageTableManager.h"

Process::Process(std::string name) {
   this->name = name; 
   ifs =  std::ifstream (name, std::fstream::in);
}

Process::~Process() {
    ifs.close();    
}

//@param none
//@throws Runtime error if file not found
void Process::run(mem::MMU &mem, PageTableManager &ptm){
    std::string line;
    int lineCount = 0;
  
    if(!ifs.good())
            throw std::runtime_error("ERROR: File not found: " + name + "\n");
        while(ifs.good()){
            while(getline(ifs,line)){
                std::string s;
                std::istringstream iss (line);
                int a1, a2;
                
                //Print line number before output
                lineCount++;
                std::cout << lineCount << ":" << iss.str() << std::endl;
                  
                while(iss >> s){
                    if(s.compare("#") == 0){
                        while(iss >> std::hex >> a1){
                            //Read until end of line w/o action
                        }
                    } //Comment
                    else if(s.compare("map") == 0){
                        std::vector<int> args;
                        while(iss >> std::hex >> a1){
                            args.push_back(a1);
                        }
                        map(mem, args, ptm);
                    }//map
                    else if(s.compare("diff") == 0){
                        std::vector<int> expected;
                        while(iss >> std::hex >> a1){
                            expected.push_back(a1);
                        }
                        diff(mem, expected);
                    } //Diff
                    else if(s.compare("store") == 0){
                        std::vector<int> vals;
                        while(iss >> std::hex >> a1){
                            vals.push_back(a1);
                        }
                        store(mem, vals);
                    } //Store    
                    else if(s.compare("replicate") == 0){
                        std::vector<int> args;
                        while(iss >> std::hex >> a1){
                            args.push_back(a1);
                        }
                        rep(mem, args);
                    } //Replicate
                    else if(s.compare("duplicate") == 0){
                        std::vector<int> args;
                        while(iss >> std::hex >> a1){
                            args.push_back(a1);
                        }
                        duplicate(mem, args);
                    } //Duplicate
                      else if(s.compare("print") == 0){
                        std::vector<int> args;
                        while(iss >> std::hex >> a1){
                            args.push_back(a1);
                        }
                        print(mem, args);
                    }//Print
                }//word 
            }//line
        }//file
}//run 

void Process::map(mem::MMU &mem, std::vector<int> vals, PageTableManager &ptm){
    int numPages = vals[0];
    mem::Addr vAddr = vals[1];
    ptm.map(numPages, vAddr);
}

void Process::diff(mem::MMU &mem, std::vector<int> vals){
    //Get address to check from arguments
    mem::Addr addr = vals.back();
    
    //Copy data in memory into buffer for comparison
    uint8_t buff[vals.size() - 1];
    try{
        mem.get_bytes(buff,addr,sizeof(buff));
    }
    catch(mem::PageFaultException e){
            std::cout << "PageFaultException at virtual address " <<
                    std::hex << "0x" << addr << " " << e.what() << std::endl;
            mem.set_PMCB(mem::PMCB::NONE);
     }
    catch(mem::PhysicalMemoryBoundsException e){
        std::cout << "PhysicalMemoryBoundsException at virtual address " <<
                std::hex << "0x" << addr << " " << e.what() << std::endl;
        mem.set_PMCB(mem::PMCB::NONE);
    }
    
    //Compare what is in the buffer and the argument list
    for(int i = 0; i < vals.size() - 1; i++){
            if(buff[i] != vals[i]){
                 std::cerr << std::hex << "Address: " << addr + i << ", expected: " << vals[i] << ", actual: " << (int) buff[i] << std::endl;
            }
    }
}

void Process::store(mem::MMU &mem, std::vector<int> vals){
    mem::Addr addr = vals.back();
    uint8_t data[vals.size() - 1];
    for(int i = 0; i < vals.size() - 1; i++){
        data[i] = vals[i];
    }
    
    mem.put_bytes(addr, sizeof(data), data);
}

void Process::rep(mem::MMU &mem, std::vector<int> args){
    int val = args.front();
    int count = args[1];
    
    mem::Addr addr = args.back();
    
    uint8_t* data = new uint8_t[count];
    for(int i = 0; i < count; i++){
        data[i] = val;
    }
    try{
        mem.put_bytes(addr, count, data);
    }
    catch(mem::PageFaultException e){
        std::cout << "PageFaultException at virtual address " <<
                std::hex << "0x" << addr << " " << e.what() << std::endl;
        mem.set_PMCB(mem::PMCB::NONE);
    }
    catch(mem::PhysicalMemoryBoundsException e){
        std::cout << "PhysicalMemoryBoundsException at virtual address " <<
                std::hex << "0x" << addr << " " << e.what() << std::endl;
        mem.set_PMCB(mem::PMCB::NONE);
    }
    
    delete [] data;
}

void Process::duplicate(mem::MMU &mem, std::vector<int> args){
    int count = args.front();
    mem::Addr src  = args[1];
    mem::Addr dest = args.back();
    
    uint8_t buff[count];
    
    mem.get_bytes(buff,src,sizeof(buff));
    mem.put_bytes(dest,sizeof(buff),buff);
}

void Process::print(mem::MMU &mem, std::vector<int> args){
    int count = args.front();
    mem::Addr addr = args.back();
    
    std::cout << std::hex << addr << std::endl;
    
    uint8_t buff[count];
    mem.get_bytes(buff, addr, sizeof(buff));
    
    for(int i = 0; i < count; i++){
        if(i % 16 == 0 && i != 0)
            std::cout << std::endl;
        std::cout << std::setfill('0') << std::setw(2) << std::hex <<(int) buff[i] << " ";
    }
    std::cout << std::dec << std::endl;
    
}
