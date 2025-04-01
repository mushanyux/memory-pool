#pragma once

#include "Common.h"
#include <map>
#include <mutex>

namespace memoryPool {
    class PageCache
    {
    public:
        static const size_t PAGE_SIZE = 4096;
    
        static PageCache& getInstance() {
            static PageCache instance;
            return instance;
        }
    
        void* allocateSpan(size_t numPages);
    
        void deallocateSpan(void* ptr, size_t numPages);
    
    private:
        PageCache() = default;
    
        void* systemAlloc(size_t numPages);

        struct Span {
            void*  pageAddr; // 页起始地址
            size_t numPages; // 页数
            Span*  next;     // 链表指针
        };
    
        std::map<size_t, Span*> freeSpans_;
        std::map<void*, Span*> spanMap_;
        std::mutex mutex_;
    };


}
