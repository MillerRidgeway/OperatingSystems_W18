/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Miller Ridgeway
 *
 * Created on April 5, 2018, 11:09 AM
 */

#include <cstdlib>
#include <string>
#include "Process.h"

int main(int argc, char** argv) {
    std::string s = argv[1];
   
    Process p1(s);
    p1.run();
    
    return 0;
}

