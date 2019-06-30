#include"mem_helper.h"
namespace tiovm
{
    uint8 MemReader::get8(const uint8* addr) {
        return addr[0];
    }

    uint32 MemReader::get32(const uint8* addr) {
        uint32* ptr = (uint32*)addr;
        return ptr[0];
    }

    uint64 MemReader::get64(const uint8* addr) {
        uint64* ptr = (uint64*)addr;
        return ptr[0];
    }
}