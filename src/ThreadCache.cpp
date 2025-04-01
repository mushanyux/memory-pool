#include "../include/ThreadCache.h"
#include "../include/CentralCache.h"

namespace memoryPool {

void* ThreadCache::allocate(size_t size) {
    if (size == 0) {
        size = ALIGNMENT;
    }
    
    if (size > MAX_BYTES) {
        return malloc(size);
    }

    size_t index = SizeClass::getIndex(size);

    freeListSize_[index]--;
    if (void* ptr = freeList_[index]) {
        freeList_[index] = *reinterpret_cast<void**>(ptr); 
        return ptr;
    }

    return fetchFromCentralCache(index);
}

void ThreadCache::deallocate(void* ptr, size_t size) {
    if (size > MAX_BYTES) {
        free(ptr);
        return;
    }

    size_t index = SizeClass::getIndex(size);

    *reinterpret_cast<void**>(ptr) = freeList_[index];
    freeList_[index] = ptr;

    freeListSize_[index]++; 

    if (shouldReturnToCentralCache(index)) {
        returnToCentralCache(freeList_[index], size);
    }
}

bool ThreadCache::shouldReturnToCentralCache(size_t index) {
    size_t threshold = 64; 
    return (freeListSize_[index] > threshold);
}

void* ThreadCache::fetchFromCentralCache(size_t index) {
    size_t size = (index + 1) * ALIGNMENT;
    size_t batchNum = getBatchNum(size);
    void* start = CentralCache::getInstance().fetchRange(index, batchNum);
    if (!start) return nullptr;

    freeListSize_[index] += batchNum; 

    void* result = start;
    if (batchNum > 1) {
        freeList_[index] = *reinterpret_cast<void**>(start);
    }
    
    return result;
}

void ThreadCache::returnToCentralCache(void* start, size_t size) {
    size_t index = SizeClass::getIndex(size);

    size_t alignedSize = SizeClass::roundUp(size);

    size_t batchNum = freeListSize_[index];
    if (batchNum <= 1) return; 

    size_t keepNum = std::max(batchNum / 4, size_t(1));
    size_t returnNum = batchNum - keepNum;

    char* current = static_cast<char*>(start);
    char* splitNode = current;
    for (size_t i = 0; i < keepNum - 1; ++i) {
        splitNode = reinterpret_cast<char*>(*reinterpret_cast<void**>(splitNode));
        if (splitNode == nullptr) {
            returnNum = batchNum - (i + 1);
            break;
        }
    }

    if (splitNode != nullptr) {
        void* nextNode = *reinterpret_cast<void**>(splitNode);
        *reinterpret_cast<void**>(splitNode) = nullptr; 

        freeList_[index] = start;

        freeListSize_[index] = keepNum;

        if (returnNum > 0 && nextNode != nullptr) {
            CentralCache::getInstance().returnRange(nextNode, returnNum * alignedSize, index);
        }
    }
}

size_t ThreadCache::getBatchNum(size_t size) {
    constexpr size_t MAX_BATCH_SIZE = 4 * 1024; 

    size_t baseNum;
    if (size <= 32) baseNum = 64;   
    else if (size <= 64) baseNum = 32; 
    else if (size <= 128) baseNum = 16; 
    else if (size <= 256) baseNum = 8; 
    else if (size <= 512) baseNum = 4;  
    else if (size <= 1024) baseNum = 2; 
    else baseNum = 1;

    size_t maxNum = std::max(size_t(1), MAX_BATCH_SIZE / size);
    return std::max(sizeof(1), std::min(maxNum, baseNum));
}

} 