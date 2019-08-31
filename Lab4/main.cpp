/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Miller Ridgeway
 *
 * Created on April 19, 2018, 10:50 AM
 */

#include <cstdlib>
#include "Process.h"
#include "MMU.h"
#include <iostream>
using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    Process p(argv[1]);
    p.run();
    
    return 0;
}

