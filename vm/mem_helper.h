#ifndef __TIOVM_MEMHELPER
#define __TIOVM_MEMHELPER
namespace tiovm
{
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    typedef char int8;
    typedef int int32;
    typedef long long int64;

    class MemReader {
    public:
        static int8 get8(const uint8* addr);
        static int32 get32(const uint8* addr);
        static int64 get64(const uint8* addr);
    };
}
#endif