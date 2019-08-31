/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Miller Ridgeway
 *
 * Created on April 12, 2018, 11:09 AM
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include "MemoryAllocator.h"
#include <vector>

using namespace std;

int main(int argc, char** argv) {
    
    //File from first cmd line argument
    std::string name = argv[1];
    std::ifstream ifs (name, std::fstream::in);
    
    std::string line;
    int lineCount = 0;
    std::vector<uint32_t> page_frames;
    
    if(!ifs.good())
            throw std::runtime_error("ERROR: File not found: " + name + "\n");
    
     //Read in first line for allocation
    int pageCount;
    if(getline(ifs,line)){
        std::istringstream firstRead (line);
        firstRead >> std::hex >> pageCount;
        std::cout << "#" << firstRead.str() << std::endl;
    }
    MemoryAllocator mem(pageCount);
    
    
    //Read the rest of the file
    while(ifs.good()){
        while(getline(ifs,line)){
            std::string s;
            std::istringstream iss (line);
            int temp;

            //Print # before output
            std::cout << "#" << iss.str() << std::endl;

                while(iss >> s){
                    //Allocate
                    if(s.compare("1") == 0){
                        iss >> std::hex >> temp;
                        bool check = mem.AllocatePageFrames(temp,page_frames);
                        std::cout << std::boolalpha << " " <<  check << " " << std::hex << mem.getPageFramesFree() << std::endl;
                    }
                    //Printing
                    else if(s.compare("2") == 0){
                        mem.PrintFreeList();
                    }
                    //Deallocate
                    else if(s.compare("0") == 0){
                        iss >> std::hex >> temp;
                        bool check = mem.FreePageFrames(temp,page_frames);
                        std::cout << std::boolalpha << " " <<  check << " " << std::hex << mem.getPageFramesFree() << std::endl;
                    }
                }//word
            }//line 
        }//file
    ifs.close();
}//main


