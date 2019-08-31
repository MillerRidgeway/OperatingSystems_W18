/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Miller Ridgeway
 *
 * Created on April 18, 2018, 10:19 AM
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include "Process.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    //Arguments
    std::string fname = argv[1];
    float block_dur = strtof(argv[2], NULL);
    float pred_weight = strtof(argv[3], NULL);
    
    Process p(fname, block_dur, pred_weight);
    p.run();
    
    return 0;
}

