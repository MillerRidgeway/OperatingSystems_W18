/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Process.h
 * Author: Miller Ridgeway
 *
 * Created on April 5, 2018, 11:13 AM
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <fstream>
#include <vector>

class Process {
public:
    Process(std::string);
    
    //Rule of Five  
    ~Process();
    Process(const Process &w) = delete;
    Process& operator=(const Process &w) = delete;
    Process(Process &&w) = delete;
    Process& operator=(Process &&w) = delete;
    
    void run();
    
private:
     std::ifstream ifs;
     std::string name;
     
     void memsize(std::vector<uint8_t> &mem, int size);
     void diff(std::vector<uint8_t> &mem, std::vector<int> vals); //Final value of vals is addr
     void store(std::vector<uint8_t> &mem, std::vector<int> vals); //The same is true here
     void rep(std::vector<uint8_t> &mem,  std::vector<int> args);
     void duplicate(std::vector<uint8_t> &mem, std::vector<int> args);
     void print(std::vector<uint8_t> &mem, std::vector<int> args);
};

#endif /* PROCESS_H */

