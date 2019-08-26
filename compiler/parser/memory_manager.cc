#include"memory_manager.h"
#include"log/log.h"
#include<iostream>
using namespace std;
using namespace logger;
namespace tio
{
    long long MemoryManager::static_mem_start = 0;
    long long MemoryManager::static_mem_top = 0;

    long long MemoryManager::alloc_static(int size) {
        long long ret = static_mem_start;
        static_mem_start += size;
        return ret;
    }
}