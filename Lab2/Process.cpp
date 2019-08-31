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

Process::Process(std::string name) {
   this->name = name;
   ifs =  std::ifstream (name, std::fstream::in);
}

Process::~Process() {
    ifs.close();    
}

void Process::run(){
    std::string line;
    std::vector<uint8_t> mem;
    int lineCount = 0;
    
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
                    if(s.compare("memsize") == 0){
                        iss >> std::hex >> a1;
                        memsize(mem, a1);
                    } //Memsize
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
                    } //Print
                }//word 
            }//line
        }//file
}

void Process::memsize(std::vector<uint8_t> &mem, int size){
    for(int i = 0; i < size; i++){
        mem.push_back(0);
    }
}

void Process::diff(std::vector<uint8_t> &mem, std::vector<int> vals){
    int addr = vals.back();
    for(int i = 0; i < vals.size() - 1; i++){
            if(mem[addr + i] != vals[i]){
                 std::cerr << std::hex << "Address: " << addr + i << ", expected: " << vals[i] << ", actual: " << (int) mem.at(addr + i) << std::endl;
            }
    }
}

void Process::store(std::vector<uint8_t> &mem, std::vector<int> vals){
    int addr = vals.back();
    for(int i = 0; i < vals.size() - 1; i++){
        mem.at(addr + i) = vals[i];
    }
}
void Process::rep(std::vector<uint8_t> &mem, std::vector<int> args){
    int val = args.front();
    int count = args[1];
    int addr = args.back();
    for(int i = 0; i < count; i++){
        mem[addr + i] = val;
    }
}

void Process::duplicate(std::vector<uint8_t>& mem, std::vector<int> args){
    int count = args.front();
    int src  = args[1];
    int dest = args.back();
    
    for(int i = 0; i < count; i++){
        mem[dest + i] = mem[src + i];
    }
}

void Process::print(std::vector<uint8_t>& mem, std::vector<int> args){
    int count = args.front();
    int addr = args.back();
    
    std::cout << std::hex << addr << std::endl;
    for(int i = 0; i < count; i++){
        if(i % 16 == 0 && i != 0)
            std::cout << std::endl;
        std::cout << std::setfill('0') << std::setw(2) << std::hex <<(int) mem[addr + i] << " ";
    }
    std::cout << std::dec << std::endl;
    
}
