/* 
 * File:   main.cpp
 * Author: Miller Ridgeway
 *
 * Created on March 29, 2018, 11:02 AM
 */

#include <cstdlib>
#include <iostream>
#include "nbproject/WordCount.h"
#include <string>
using namespace std;


int main(int argc, char** argv) {
    std::string s = argv[1];
   
    WordCount w(s);
    std::cout << w.getWordCount() << " words, " << w.getLineCount() << " lines, " << w.getMaxWordsInLine() << " max words in line";
    return 0;
}

