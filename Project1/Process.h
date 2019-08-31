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
    Process(std::string, float, float);
    
    //Rule of Five  
    ~Process();
    Process(const Process &w) = delete;
    Process& operator=(const Process &w) = delete;
    Process(Process &&w) = delete;
    Process& operator=(Process &&w) = delete;
    
    void run();
    
private:
     std::ifstream ifs; 
     std::string file;
     
     float block_duration;
     float pred_weight;
     float current_time;
     float avg;
     float proc_size;
     
     struct Proc{
        float priority = 0;
        float arrival;
        float blockTime;
        bool block;
        bool arrived;
        std::vector<float> times;
        std::string name;
     };
     std::vector<Proc> procs;  
     
     //Run assist functions 
     void readFile();
     float nextArrival();
     float nextProcess();
     float nextIdle();
     bool idleCheck();
     void unblock();
};

#endif /* PROCESS_H */

