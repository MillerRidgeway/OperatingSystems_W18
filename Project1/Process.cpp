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
#include <algorithm>
#include <math.h>

Process::Process(std::string name, float block_dur, float prediction_weight) {
   this->file = name;
   this->block_duration = block_dur;
   this->pred_weight = prediction_weight;
   this->current_time = 0;
   this->avg = 0;
   this->proc_size = -1;
   ifs =  std::ifstream (name, std::fstream::in);
}

Process::~Process() {
    //Do nothing
}

// Run - Parse the read file and output to console
// @console - Writes to console the scheduling of procs

void Process::run(){
    //Info line
    std::cout << file << " " << block_duration << " " << pred_weight << std::endl;
    
    //Read the file in
    readFile();
    
    proc_size = procs.size();
    while(true){
        for(int i = 0; i < procs.size(); i++){
            //Remove if no time slices
            if(procs[i].times.size() == 0){
                procs.erase(procs.begin() + i);
            }
            
            //Arrival
            if(procs[i].arrival == nextArrival() && current_time >= nextArrival()){
                 procs[i].arrived = true; 
            }

            //Process
            if(procs[i].priority == nextProcess()){
                //Adjust priority
                if(procs[i].priority == 0)
                    procs[i].priority = procs[i].times.back();
                else
                    procs[i].priority = pred_weight*procs[i].priority + (1 - pred_weight)*procs[i].times.back();

                //Is final time slice? If so, add on to throughput and terminate
                if(procs[i].times.size() == 1){
                    std::cout << current_time << "\t" << procs[i].name << "\t" << procs[i].times.back() << "\t" << "T" << "\t" << std::endl;
                    avg += (current_time + procs[i].times.back()) - procs[i].arrival;
                }
                else
                    std::cout << current_time << "\t" << procs[i].name << "\t" << procs[i].times.back() << "\t" << "B" << "\t" << procs[i].priority << std::endl;

                current_time += procs[i].times.back();
                procs[i].blockTime = current_time;
                procs[i].block = true;
                procs[i].times.pop_back();
            }
            
            //Unblock if needed
            unblock();
        }//for
        
        //Idle
        if(idleCheck()){
            if(procs.size() == 1 && procs[0].times.size() == 0){
                 std::cout << current_time << "\t" << "(DONE)" << "\t" << avg/proc_size << "\t" << std::endl;
                 break;
            }
            float idleTime;
            float nextI = (nextIdle() + block_duration) - current_time;
            float nextA = fabs(current_time - nextArrival());
            idleTime = std::min(nextI, nextA);

            std::cout << current_time << "\t" << "(IDLE)" << "\t" << idleTime << "\t" << "I" << std::endl;

            current_time += idleTime;
        }
    }//while
            
}//run

float Process::nextArrival(){
    float ret = 0x7FFFFFFF;
    for(int i = 0; i < procs.size(); i++){
        if(procs[i].arrival < ret && !procs[i].arrived)
            ret = procs[i].arrival;
    }
    
    return ret;
}

float Process::nextProcess(){
    float spn = 0x7FFFFFFF;
    for(int i = 0; i < procs.size(); i++){
        if(procs[i].priority < spn && procs[i].arrived && !procs[i].block)
            spn = procs[i].priority;
    }
    
    return spn;
}

float Process::nextIdle(){
    float next = 0x7FFFFFFF;
    for(int i = 0; i < procs.size(); i++)
    {
        if(procs[i].block){
            if(procs[i].blockTime < next)
                next = procs[i].blockTime;
        }
    }
    
    return next;
}

bool Process::idleCheck(){
    bool isIdle = true;
    for(int i = 0; i < procs.size(); i++){
        if(procs[i].arrived && !procs[i].block)
            isIdle = false;
    }
    
    return isIdle;
}

void Process::unblock(){
    for(int i = 0; i < procs.size(); i++){
        if(procs[i].blockTime + block_duration <= current_time){
            procs[i].block = false;
            //std::cout << "Unblocked: " << procs[i].name << std::endl;
        }
    }
}


// readFile - Reads the input file, populating an array of struct procs (found in header)
// @throws - Runtime error if filename cannot be found

void Process::readFile(){
    std::string line;  
    
    if(!ifs.good())
        throw std::runtime_error("ERROR: File not found: " + file + "\n");

    while(ifs.good()){
        while(getline(ifs,line)){
            std::string s;
            std::istringstream iss (line);
            float temp;

            while(iss >> s){
                std::vector<float> args;
                Proc p;
                
                p.name = s;
                while(iss >> temp){
                    args.push_back(temp);
                }
                p.arrival = args.front();
                std::reverse(args.begin(), args.end());
                args.pop_back();
                p.times = args;
                
                this->procs.push_back(p);
            }//word 
        }//line
    }//file
    
    ifs.close();
}//readFile
