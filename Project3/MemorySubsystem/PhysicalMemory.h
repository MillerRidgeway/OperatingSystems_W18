/* Interface to physical memory
 *  
 * File:   PhysicalMemory.h
 * Author: Mike Goss <mikegoss@cs.du.edu>
 *
 * Created on June 29, 2017, 11:45 AM
 */

#ifndef MEM_PHYSICALMEMORY_H
#define MEM_PHYSICALMEMORY_H

#include "MemoryDefs.h"

#include <cstddef>
#include <vector>

namespace mem {

class PhysicalMemory {
public:
  /**
   * Constructor
   * 
   * @param size number of bytes of memory to allocate (must be a multiple
   *             of 16)
   * @throws std::bad_alloc if insufficient memory
   */
  PhysicalMemory(Addr size) : mem_data(size), byte_count(0) { };
  
  ~PhysicalMemory() { }
  
  PhysicalMemory(const PhysicalMemory &other) = delete;  // no copy constructor
  PhysicalMemory(PhysicalMemory &&other) = delete;       // no move constructor

  /**
   * size - return size of physical memory
   * 
   * @return number of bytes in physical memory 
   */
  Addr size() const { return mem_data.size(); }
  
  /**
   * get_byte - get a single byte from the specified address
   * 
   * @param dest - destination byte pointer
   * @param address - address of data in memory
   */
  void get_byte(uint8_t *dest, Addr address);
  
  /**
   * get_bytes - copy a range of bytes to caller buffer
   * 
   * @param dest where to copy to
   * @param address source address
   * @param count number of bytes to copy
   */
  void get_bytes(uint8_t *dest, Addr address, Addr count);
  
  /**
   * get_32 - get a 32 bit (4 byte) value
   * 
   * @param dest where to copy to
   * @param address source address
   */
  void get_32(uint32_t *dest, Addr address) {
    get_bytes(reinterpret_cast<uint8_t*>(dest), address, 4);
  }

  /**
   * put_byte - store a single byte to the specified address
   * 
   * @param address - address of data in memory
   * @param data - data to store at address
   */
  void put_byte(Addr address, uint8_t *data);

  /**
   * put_bytes - copy a range of bytes into physical memory
   * 
   * @param address destination in physical memory
   * @param count number of bytes to copy
   * @param src source buffer
   */
  void put_bytes(Addr address, Addr count, const uint8_t *src);
  
  /**
   * ValidateAddressRange - check that address range is valid, throw
   *   PhysicalMemoryBoundsException if not.
   * @param address
   * @param count
   */
  void ValidateAddressRange(Addr address, Addr count) const;
  
  /**
   * get_byte_count - return total number of bytes transferred so far 
   *   to/from physical memory.
   * 
   * @return count of bytes transferred.
   */
  uint64_t get_byte_count() const { return byte_count; }
  
private:
  std::vector<uint8_t> mem_data;
  
  // Define counter for number of bytes transferred.  Can be used as
  // pseudo-clock for ordering of cache entries.
  uint64_t byte_count;  // increments by one for every request
};

} // namespace mem

#endif /* MEM_PHYSICALMEMORY_H */

