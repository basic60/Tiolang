#ifndef __TIOVM_REG
#define __TIOVM_REG
#include"mem_helper.h"
#include<string>
#include<unordered_set>
namespace tiovm
{
    class Register {
    private:
        uint8* data;
    public:
        Register();
        Register(const Register& reg);
        Register(Register&& reg);
        Register& operator=(const Register& reg);
        Register& operator=(Register&& reg);
        ~Register();
        int8 get8();
        int32 get32();
        int64 get64();
        int64 get_by_size(int sz);
        void set8(int8 val);
        void set32(int32 val);
        void set64(int64 val);
        void set_by_size(int64 val, int sz);
        static bool valid_reg(std::string name);
        const static std::unordered_set<std::string> usable_regs; 
    };
}
#endif