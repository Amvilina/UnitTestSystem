#pragma once
#include <cstdint>
#include <stdlib.h>

namespace UnitTestSystem
{

class MemoryAllocator {
  private:
    static uint64_t _used_bytes;
    MemoryAllocator() {}
  public:
    static void AddUsedBytes(uint64_t bytes) {
        _used_bytes += bytes;
    }
    
    static void RemoveUsedBytes(uint64_t bytes) {
        _used_bytes -= bytes;
    }
    
    static void ResetUsedBytes() {
        _used_bytes = 0;
    }
    
    static uint64_t GetUsedBytes() {
        return _used_bytes;
    }
};
uint64_t MemoryAllocator::_used_bytes = 0;

} // namespace UnitTestSystem

void * operator new(size_t n)
{
    void* ptr = malloc(n + sizeof(n));
    size_t* dataPtr = (size_t*)ptr;
    dataPtr[0] = n;
    ptr = (void*)(++dataPtr);
    UnitTestSystem::MemoryAllocator::AddUsedBytes(n);
    return ptr;
}

void operator delete(void * ptr) throw()
{
    size_t* dataPtr = (size_t*)ptr;
    --dataPtr;
    size_t n = *dataPtr;
    ptr = (void*)(dataPtr);
    UnitTestSystem::MemoryAllocator::RemoveUsedBytes(n);
    free(ptr);
}
