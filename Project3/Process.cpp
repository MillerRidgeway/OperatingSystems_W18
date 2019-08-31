    /*
 * Process implementation 
 * 
 * File:   Process.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 * 
 */

#include "Process.h"
#include "PageTableManager.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

using mem::Addr;
using mem::kPageSize;
using std::cin;
using std::cout;
using std::cerr;
using std::getline;
using std::istringstream;
using std::string;
using std::vector;

Process::Process(const string &file_name_, mem::MMU &memory_, PageTableManager &ptm_, int procNum, int ts) 
: file_name(file_name_), line_number(0), memory(memory_), ptm(ptm_) {
  // Open the trace file.  Abort program if can't open.
  trace.open(file_name, std::ios_base::in);
  if (!trace.is_open()) {
    cerr << "ERROR: failed to open trace file: " << file_name << "\n";
    exit(2);
  }
  
  // Set up empty process page table and load process PMCB
  proc_pmcb.page_table_base = ptm.CreateProcessPageTable();
  memory.set_PMCB(proc_pmcb);
  
  pageCount = 0;
  this->procNum = procNum;
  this->timeslice = ts;
  terminated = false;
  line_number = 0;
}

Process::~Process() {
  trace.close();
}

void Process::Run() {
  // Read and process commands
  string line;                // text line read
  string cmd;                 // command from line
  vector<uint32_t> cmdArgs;   // arguments from line
  int count = 0;
    //Select the command to execute
    while (count < timeslice && !terminated && ParseCommand(line, cmd, cmdArgs)) {
      if (cmd == "pagelimit" ) {
        count++;  
        CmdPageLimit(line, cmd, cmdArgs);         // allocate and map pages
      } else if (cmd == "diff") {
        count++;
        CmdDiff(line, cmd, cmdArgs);        // get and compare multiple bytes
      } else if (cmd == "store") {
        count++;  
        CmdStore(line, cmd, cmdArgs);       // put bytes
      } else if (cmd == "replicate") {
        count++;  
        CmdRepl(line, cmd, cmdArgs);        // fill bytes with value
      } else if (cmd == "duplicate") {
        count++;  
        CmdDupl(line, cmd, cmdArgs);        // copy bytes to dest from source
      } else if (cmd == "print") {
        count++;  
        CmdPrint(line, cmd, cmdArgs);       // dump byte values to output
      } else if (cmd == "permission") {
        count++;  
        CmdPermission(line, cmd, cmdArgs);  // change page permissions
      } else if (cmd != "#") {
        cout << "ERROR: invalid command\n";
        exit(2);
      }
    }
    //Check if terminated flag is set
   if(terminated){
       //Termination message
       //ptm.FreeProcessPages(proc_pmcb.page_table_base, pageCount - 1);
       cout << std::dec << line_number << ":" << procNum << ":" << "TERMINATED, free page frames = " << memory.get_frame_count() - ptm.getAllocatedFrames() <<  "\n";
   }
  
}

bool Process::ParseCommand(
  string &line, string &cmd, vector<uint32_t> &cmdArgs) {
  cmdArgs.clear();
  line.clear();
    // Read next line
    if (std::getline(trace, line)) {
      ++line_number;
      cout << std::dec << line_number << ":" << procNum << ":" << line << "\n";
      // No further processing if comment or empty line
      if (line.size() == 0 || line[0] == '#') {
        cmd = "#";
        return true;
      }

      // Make a string stream from command line
      istringstream lineStream(line);

      // Get command
      if (!(lineStream >> cmd)) {
        // Blank line, treat as comment
        cmd = "#";
        return true;
      }

      // Get arguments
      uint32_t arg;
      while (lineStream >> std::hex >> arg) {
        cmdArgs.push_back(arg);
      }
      return true;
    } else if (trace.eof()) {
        terminated = true;
        return false;
    } else {
      cerr << "ERROR: getline failed on trace file: " << file_name 
              << "at line " << line_number << "\n";
      exit(2);
    }
}

void Process::CmdPageLimit(const std::string& line, 
                           const std::string& cmd, 
                           std::vector<uint32_t>& cmdArgs){
    //Set max number of pages
    maxPages = cmdArgs.back();
}

void Process::CmdDiff(const string &line,
                              const string &cmd,
                              vector<uint32_t> &cmdArgs) {
  Addr addr = cmdArgs.back();

  try {
    // Compare specified byte values
    int count = cmdArgs.size() - 1;
    for (int i = 0; i < count; ++i) {
      uint8_t b;
      memory.get_byte(&b, addr);
      if(b != cmdArgs.at(i)) {
        cout << "diff error at address " << std::hex << addr
                << ", expected " << static_cast<uint32_t>(cmdArgs.at(i))
                << ", actual is " << static_cast<uint32_t>(b) << "\n";
      }
      ++addr;
    }
  }
  catch(mem::PageFaultException e) {
    memory.get_PMCB(proc_pmcb);
    cout << "PageFaultException at address " << std::hex 
            <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
    proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
    memory.set_PMCB(proc_pmcb);
  }
}

void Process::CmdStore(const string &line,
                       const string &cmd,
                       vector<uint32_t> &cmdArgs) {
  // Store multiple bytes starting at specified address
  Addr addr = cmdArgs.back();
  cmdArgs.pop_back();
  
  // Build buffer
  uint8_t buffer[cmdArgs.size()];
  for (size_t i = 0; i < cmdArgs.size(); ++i) {
    buffer[i] = cmdArgs[i];
  }
  
  // Write to MMU
  try {
    memory.put_bytes(addr, cmdArgs.size(), buffer);
  }
  
  catch(mem::WritePermissionFaultException e) {
    memory.get_PMCB(proc_pmcb);
    cout << "WritePermissionFaultException at address " << std::hex 
            <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
    proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
    memory.set_PMCB(proc_pmcb);
  }
  
  catch(mem::PageFaultException e) {
    memory.get_PMCB(proc_pmcb);

    //Check if current count exceeds max PT count
    //Allocate new page for process
    pageCount++;
    if(pageCount > maxPages){
        memory.get_PMCB(proc_pmcb);
        cout << "Page limit exceeded, process terminated, free page frames = " << memory.get_frame_count() << std::endl;
        proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
        terminated = true;
    }
    
    ptm.MapProcessPages(proc_pmcb.next_vaddress & mem::kPageNumberMask, 1);
    
    //Resume process
    memory.set_PMCB(proc_pmcb);
    proc_pmcb.operation_state = mem::PMCB::NONE;
  }
} 

void Process::CmdDupl(const string &line,
                      const string &cmd,
                      vector<uint32_t> &cmdArgs) {
  // Duplicate specified number of bytes to destination from source
  Addr dst = cmdArgs.at(2);
  Addr src = cmdArgs.at(1);
  uint32_t count = cmdArgs.at(0);
  uint8_t b;
  try {
    while (count-- > 0) {
      memory.get_byte(&b, src++);
      memory.put_byte(dst++, &b);
    }
  }
  catch(mem::PageFaultException e) {
    memory.get_PMCB(proc_pmcb);
    
    if(proc_pmcb.next_vaddress + 1 == src){
        cout << "PageFaultException at address " << std::hex 
             <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
        proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
        memory.set_PMCB(proc_pmcb);
    }
    else{
        //Check if current count exceeds max PT count
        //Allocate new page for process
        pageCount++;
        if(pageCount > maxPages){
            memory.get_PMCB(proc_pmcb);
            cout << "Page limit exceeded, process terminated, free page frames = " << memory.get_frame_count()<< std::endl;
            proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
            terminated = true;
        }

        ptm.MapProcessPages(proc_pmcb.next_vaddress & mem::kPageNumberMask, 1);

        //Resume process
        memory.set_PMCB(proc_pmcb);

        CmdDupl(line,cmd,cmdArgs);
        
        proc_pmcb.operation_state = mem::PMCB::NONE;
    }
  }
  
  catch(mem::WritePermissionFaultException e) {
    memory.get_PMCB(proc_pmcb);
    cout << "WritePermissionFaultException at address " << std::hex 
            <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
    proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
    memory.set_PMCB(proc_pmcb);
    
  }
}

void Process::CmdRepl(const string &line,
                      const string &cmd,
                      vector<uint32_t> &cmdArgs) {
  // Replicate specified value in destination range
  uint8_t value = cmdArgs.at(0);
  uint32_t count = cmdArgs.at(1);
  uint32_t addr = cmdArgs.at(2);
  
  int iter = 0;
  
  try {
    for (int i = 0; i < count; ++i) {
      memory.put_byte(addr++, &value);
      iter++;
    }
  }
  catch(mem::WritePermissionFaultException e) {
    memory.get_PMCB(proc_pmcb);
    cout << "WritePermissionFaultException at address " << std::hex 
            <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
    proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
    memory.set_PMCB(proc_pmcb);
  }
  
  catch(mem::PageFaultException &e) {
    memory.get_PMCB(proc_pmcb);
    
    //Check if current count exceeds max PT count
    //Allocate new page for process
    pageCount++;
    if(pageCount > maxPages){
        memory.get_PMCB(proc_pmcb);
        cout << "Page limit exceeded, process terminated, free page frames = " << memory.get_frame_count()<< std::endl;
        proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
        terminated = true;
    }
    
    ptm.MapProcessPages(proc_pmcb.next_vaddress & mem::kPageNumberMask, 1);
        
    memory.set_PMCB(proc_pmcb);
    
        
    proc_pmcb.operation_state = mem::PMCB::NONE;
    CmdRepl(line,cmd,cmdArgs);

  }
}

void Process::CmdPrint(const string &line,
                       const string &cmd,
                       vector<uint32_t> &cmdArgs) {
  uint32_t addr = cmdArgs.at(1);
  uint32_t count = cmdArgs.at(0);

  // Output the address
  cout << std::hex << addr;

  try {
    // Output the specified number of bytes starting at the address
    uint8_t b;
    for(int i = 0; i < count; ++i) {
      if((i % 16) == 0) {  // line break every 16 bytes
        cout << "\n";
      }
      memory.get_byte(&b, addr++);
      cout << " " << std::setfill('0') << std::setw(2)
              << static_cast<uint32_t> (b);
    }
  }
  catch(mem::PageFaultException e) {
    memory.get_PMCB(proc_pmcb);
    cout << "PageFaultException at address " << std::hex 
            <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
    proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
    memory.set_PMCB(proc_pmcb);
  }
  catch(mem::WritePermissionFaultException e) {
    memory.get_PMCB(proc_pmcb);
    cout << "WritePermissionFaultException at address " << std::hex 
            <<  proc_pmcb.next_vaddress << ": " << e.what() << "\n";
    proc_pmcb.operation_state = mem::PMCB::NONE;  // clear fault
    memory.set_PMCB(proc_pmcb);
  }

  cout << "\n";
}

void Process::CmdPermission(const string &line, 
                            const string &cmd, 
                            vector<uint32_t> &cmdArgs) {
  // Change the permissions of the specified pages
  ptm.SetPageWritePermission(cmdArgs.at(2), cmdArgs.at(0), cmdArgs.at(1));
}

bool Process::isTerm(){
    return terminated;
}

