#ifndef __TIOVM_MEMHELPER
#define __TIOVM_MEMHELPER
namespace tiovm
{
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    class MemReader {
    public:
        static uint8 get8(const uint8* addr);
        static uint32 get32(const uint8* addr);
        static uint64 get64(const uint8* addr);
    };
}
#endif