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

Process::Process(std::string name) {
   this->name = name; 
   ifs =  std::ifstream (name, std::fstream::in);
}

Process::~Process() {
    ifs.close();    
}

//@param none
//@throws Runtime error if file not found
void Process::run(){
    std::string line;
    int lineCount = 0;
  
    mem::MMU mem(getMemsize() / mem::kPageSize);
    
    if(!ifs.good())
            throw std::runtime_error("ERROR: File not found: " + name + "\n");
        while(ifs.good()){
            while(getline(ifs,line)){
                std::string s;
                std::istringstream iss (line);
                int a1;
                
                //Print line number before output
                lineCount++;
                std::cout << lineCount << ":" << iss.str() << std::endl;
    
                while(iss >> s){
                    if(s.compare("#") == 0){
                        while(iss >> std::hex >> a1){
                            //Read until end of line w/o action
                        }
                    } //Comment
                    else if(s.compare("memsize") == 0){
                        iss >> std::hex >> a1;
                        int rounded_num = ((a1 + 0x10000/2) / 0x10000) * 0x10000;
                    }//memsize
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

int Process::getMemsize(){
    if(!ifs.good()){
        throw std::runtime_error("ERROR: File not found: " + name + "\n");
    }
    std::string line;
    while(ifs.good()){
        while(getline(ifs,line)){
            std::string s;
            std::istringstream iss (line);
            int a1;
            
            while(iss >> s){
                if(s.compare("memsize") == 0){
                    iss >> std::hex >> a1;
                    int rounded_num = ((a1 + 0x10000/2) / 0x10000) * 0x10000;
                    return rounded_num;
                }
            }   
        }
    }
}

void Process::diff(mem::MMU &mem, std::vector<int> vals){
    //Get address to check from arguments
    mem::Addr addr = vals.back();
    
    //Copy data in memory into buffer for comparison
    uint8_t buff[vals.size() - 1];
    mem.get_bytes(buff,addr,sizeof(buff));
    
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
    
    mem.put_bytes(addr, count, data);
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
