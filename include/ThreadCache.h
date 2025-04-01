#pragma once
#include "Common.h"

namespace memoryPool {

class ThreadCache {
private:
    ThreadCache() = default;
    void* fetchFromCentralCache(size_t index);
    void returnToCentralCache(void* start, size_t size);
    size_t getBatchNum(size_t size);
    bool shouldReturnToCentralCache(size_t index);
    std::array<void*, FREE_LIST_SIZE> freeList_;    
    std::array<size_t, FREE_LIST_SIZE> freeListSize_; 
public:
    static ThreadCache* getInstance() {
        static thread_local ThreadCache instance;
        return &instance;
    }
    void *allocate(size_t size);
    void  deallocate(void* ptr, size_t size);

};

}