/* 
 * File:   PhysicalMemoryTests
 * Author: Mike Goss <mikegoss@cs.du.edu>
 *
 * Created on Aug 6, 2017, 4:28:24 PM
 */
#include "../PhysicalMemory.h"
#include "../Exceptions.h"

#include <gtest/gtest.h>
#include <cstring>

using mem::PhysicalMemory;
using mem::PhysicalMemoryBoundsException;
using mem::Addr;

namespace {  // unnamed namespace for local functions

/**
 * RandBuf - initialize a buffer with random data. It doesn't need to be
 *   especially random, just different every time, so we use the (not very good)
 *   rand() function.
 * 
 * @param buf buffer to initialize
 * @param buf_len number of bytes to initialize
 */
void RandBuf(uint8_t *buf, Addr buf_len) {
  for (Addr i = 0; i < buf_len; ++i) {
    buf[i] = 0xFF & rand();
  }
}

}  // namespace

class PhysicalMemoryTests : public testing::Test {
protected:
};

TEST_F(PhysicalMemoryTests, ConstructorAndSize) {
  // Allocate various sizes and check for correct returned size
  {
    PhysicalMemory pm(2048);
    ASSERT_EQ(2048, pm.size());
    ASSERT_EQ(0, pm.get_byte_count());
  }
  {
    PhysicalMemory pm(8);
    ASSERT_EQ(8, pm.size());
    ASSERT_EQ(0, pm.get_byte_count());
  }
  {
    PhysicalMemory pm(1024*1024);
    ASSERT_EQ(1024*1024, pm.size());
    ASSERT_EQ(0, pm.get_byte_count());
  }
}

/**
 * Test put/get of single byte
 */
TEST_F(PhysicalMemoryTests, PutGetByte) {
  // Write then read every byte
  const Addr kSize = 1024;
  PhysicalMemory pm(kSize);
  uint64_t expected_byte_count = 0;
  uint8_t data_byte = 0;
  
  // Initialize every byte to 0xFF
  uint8_t xff = 0xFF;
  for (Addr addr = 0; addr < kSize; ++addr) {
    pm.put_byte(addr, &xff);
    ASSERT_EQ(0xFF, xff);  // should be unchanged
    pm.get_byte(&data_byte, addr);
    ASSERT_EQ(0xFF, data_byte);
    expected_byte_count += 2;
    ASSERT_EQ(expected_byte_count, pm.get_byte_count());
  }
  
  // Write different values to every byte and check value
  uint8_t x55 = 0x55;
  for (Addr addr = 0; addr < kSize; ++addr) {
    pm.put_byte(addr, &x55);
    ASSERT_EQ(0x55, x55);  // should be unchanged
    pm.get_byte(&data_byte, addr);
    ASSERT_EQ(0x55, data_byte);
    uint8_t addr_hash = 0xFF & (addr ^ (addr >> 8));
    pm.put_byte(addr, &addr_hash);
    pm.get_byte(&data_byte, addr);
    ASSERT_EQ(addr_hash, data_byte);
    expected_byte_count += 4;
    ASSERT_EQ(expected_byte_count, pm.get_byte_count());
  }
}

/**
 * Test put/get of block of bytes
 */
TEST_F(PhysicalMemoryTests, PutGetBytes) {
  // Read/write blocks of bytes of all lengths
  const Addr kMaxSize = 1024;
  uint8_t put_buf[kMaxSize+1];
  uint8_t get_buf[kMaxSize+1];
  PhysicalMemory pm(kMaxSize);
  memset(put_buf, 0, kMaxSize+1);  // clear put buffer
  uint64_t expected_byte_count = 0;
  
  // Try all possible sizes
  for (Addr size = 1; size <= kMaxSize; ++size) {
    put_buf[size] = 0x55;  // sentinel at end of put buffer
    // Try all possible addresses
    for (Addr start = 0; start <= (kMaxSize - size); ++start) {
      memset(get_buf, 0, size);  // clear
      get_buf[size] = 0xAA;  // sentinel at end of buffer to check overwrite
      RandBuf(put_buf, size);  // generate random data to write
      pm.put_bytes(start, size, put_buf);  // write data to memory
      pm.get_bytes(get_buf, start, size);  // read back data
      ASSERT_EQ(0, std::memcmp(put_buf, get_buf, size));  // compare
      ASSERT_EQ(0xAA, get_buf[size]);  // check sentinel unchanged
      expected_byte_count += 2 * size;
      ASSERT_EQ(expected_byte_count, pm.get_byte_count());
    }
  }
}

/**
 * Test invalid addresses
 */
TEST_F(PhysicalMemoryTests, AddressCheck) {
  const Addr kSize = 1024;
  PhysicalMemory pm(kSize);
  uint8_t data_byte;
  
  // Test one past end
  {
    bool success = false;
    try {
      pm.get_byte(&data_byte, kSize);
    }
    catch (PhysicalMemoryBoundsException e) {
      ASSERT_STREQ("PhysicalMemoryBoundsException, block starting at 0x400", 
                   e.what());
      success = true;  // success if exception caught
    }
    ASSERT_TRUE(success);
  }
  
  // Test one past end
  {
    bool success = false;
    uint8_t x1 = 1;
    try {
      pm.put_byte(kSize, &x1);
    }
    catch (PhysicalMemoryBoundsException e) {
      ASSERT_STREQ("PhysicalMemoryBoundsException, block starting at 0x400", 
                   e.what());
      success = true;  // success if exception caught
    }
    ASSERT_TRUE(success);
  }
    
  // Test one past end (block)
  {
    bool success = false;
    uint8_t buf[8];
    try {
      pm.get_bytes(buf, kSize-8+1, 8);
    }
    catch (PhysicalMemoryBoundsException e) {
      ASSERT_STREQ("PhysicalMemoryBoundsException, block starting at 0x3f9", 
                   e.what());
      success = true;  // success if exception caught
    }
    ASSERT_TRUE(success);
  }
    
  // Test one past end (block)
  {
    bool success = false;
    uint8_t buf[8];
    try {
      pm.put_bytes(kSize-8+1, 8, buf);
    }
    catch (PhysicalMemoryBoundsException e) {
      ASSERT_STREQ("PhysicalMemoryBoundsException, block starting at 0x3f9", 
                   e.what());
      success = true;  // success if exception caught
    }
    ASSERT_TRUE(success);
  }

  // Test max possible address
  {
    bool success = false;
    Addr addr = 0xFFFFFFFF;  // largest possible
    ASSERT_EQ(4, sizeof(addr));      // test assumes 32 bit addresses
    try {
      pm.get_byte(&data_byte, addr);
    }
    catch (PhysicalMemoryBoundsException e) {
      ASSERT_STREQ("PhysicalMemoryBoundsException, block starting at 0xffffffff", 
                   e.what());
      success = true;  // success if exception caught
    }
    ASSERT_TRUE(success);
  }
    
  // Test wrap-around
  {
    bool success = false;
    Addr addr = 0xFFFFFFFD;  // nearly largest
    ASSERT_EQ(4, sizeof(addr));      // test assumes 64 bit addresses
    uint8_t buf[8];
    try {
      pm.get_bytes(buf, addr, 4);
    }
    catch (PhysicalMemoryBoundsException e) {
      ASSERT_STREQ("PhysicalMemoryBoundsException, block starting at 0xfffffffd", 
                   e.what());
      success = true;  // success if exception caught
    }
    ASSERT_TRUE(success);
  }
  
  ASSERT_EQ(0, pm.get_byte_count());  // no reads/writes should succeed
}