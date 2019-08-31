/* Interface to physical memory
 * 
 * File:   PhysicalMemory.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 * 
 * Created on June 29, 2017, 11:45 AM
 */

#include "PhysicalMemory.h"

#include "Exceptions.h"
#include <cstring>

namespace mem {

void PhysicalMemory::ValidateAddressRange(Addr address, Addr count) const {
  // Invalid if either end address is past end of memory, or if wrap-around
  // or 0 size block
  Addr end = address + count;
  if (end > mem_data.size() || end <= address) {
    throw PhysicalMemoryBoundsException(address);
  }
}

void PhysicalMemory::get_byte(uint8_t *dest, Addr address) {
  ValidateAddressRange(address, 1);
  ++byte_count;
  *dest = mem_data[address];
}

void PhysicalMemory::get_bytes(uint8_t *dest, Addr address, Addr count) {
  ValidateAddressRange(address, count);
  byte_count += count;
  memcpy(dest, &mem_data[address], count);
}

void PhysicalMemory::put_byte(Addr address, uint8_t *data) {
  ValidateAddressRange(address, 1);
  ++byte_count;
  mem_data[address] = *data;
}

void PhysicalMemory::put_bytes(Addr address, Addr count, const uint8_t *src) {
  ValidateAddressRange(address, count);
  byte_count += count;
  memcpy(&mem_data[address], src, count);
}

} // namespace mem