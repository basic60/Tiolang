#include"memory_manager.h"
#include"log/log.h"
#include<iostream>
using namespace std;
using namespace logger;
namespace tio
{
    long long MemoryManager::static_mem_start = 0;
    long long MemoryManager::static_mem_top = 0;
    long long MemoryManager::stack_top = 0x100000000l; // 4GB

    long long MemoryManager::alloc_static(int size) {
        long long ret = static_mem_top;
        if(ret + size > stack_top) {
            LOG(ERROR)<<"Bad alloc!"<<endl;
            throw -1;
        }
        static_mem_top += size;
        return ret;
    }
}