#include"mem_helper.h"
namespace tiovm
{
    int8 MemReader::get8(const uint8* addr) {
        return addr[0];
    }

    int32 MemReader::get32(const uint8* addr) {
        int32* ptr = (int32*)addr;
        return ptr[0];
    }

    int64 MemReader::get64(const uint8* addr) {
        int64* ptr = (int64*)addr;
        return ptr[0];
    }
}