/* Define exceptions used in MemorySubsystem project.
 * 
 * File:   Exceptions.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 * 
 * Created on June 29, 2017, 10:41 AM
 * 
 */

#include "Exceptions.h"

#include <sstream>
#include <ios>

namespace mem {

/**
 * Copy constructor
 */
MemorySubsystemException::MemorySubsystemException(const MemorySubsystemException &other) {
  description = other.description;
}

/**
 * Move constructor
 */
MemorySubsystemException::MemorySubsystemException(MemorySubsystemException &&other) {
  description = std::move(other.description);
}

/**
 * Constructor for PhysicalMemoryBoundsException
 */
PhysicalMemoryBoundsException::PhysicalMemoryBoundsException(Addr address) {
  std::stringstream description_stream;
  description_stream << "PhysicalMemoryBoundsException, block starting at 0x" 
          << std::hex << address;
  SetDescription(description_stream.str());
}

} // namespace mem