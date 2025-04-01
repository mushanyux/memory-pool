#pragma once
#include <cstddef>
#include <atomic>
#include <array>

namespace memoryPool {
constexpr size_t ALIGNMENT = 8;
constexpr size_t MAX_BYTES = 256 *1024;
constexpr size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT;

//内存块头部信息
struct BlockHeader {
    size_t size;
    bool   inuse;
    BlockHeader* next;
};

//大小类
class SizeClass {
public:
    static size_t roundUp(size_t bytes) {
        return (bytes + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    static size_t getIndex(size_t bytes) {
        bytes = std::max(bytes, ALIGNMENT);
        return (bytes + ALIGNMENT - 1) / ALIGNMENT - 1;
    }
};

}