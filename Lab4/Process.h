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
#include <MMU.h>

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
     
     int getMemsize();
     void diff(mem::MMU &mem, std::vector<int> vals); //Final value of vals is addr
     void store(mem::MMU &mem, std::vector<int> vals); //The same is true here
     void rep(mem::MMU &mem,  std::vector<int> args);
     void duplicate(mem::MMU &mem, std::vector<int> args);
     void print(mem::MMU &mem, std::vector<int> args);
};

#endif /* PROCESS_H */

