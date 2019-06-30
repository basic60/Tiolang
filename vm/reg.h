#ifndef __TIOVM_REG
#define __TIOVM_REG
#include"mem_helper.h"
namespace tiovm
{
    class Register {
    private:
        uint8* data;
    public:
        Register();
        ~Register();
        uint8 get8();
        uint32 get32();
        uint64 get64();
        void set8(uint8 val);
        void set32(uint32 val);
        void set64(uint64 val);
    };
}
#endif