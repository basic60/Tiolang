#ifndef __TIO_MEMORY_MANAGER
#define __TIO_MEMORY_MANAGER
namespace tio
{
    class MemoryManager {
    private:
        static long long static_mem_start;
        static long long static_mem_top;
    public:
        static long long alloc_static(int size);
    };
}
#endif