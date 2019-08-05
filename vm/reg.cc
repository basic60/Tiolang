#include"reg.h"
#include"mem_helper.h"
#include<iostream>
using namespace std;
namespace tiovm
{
    Register::Register() {
        this->data = new uint8[8];  // A regisger has 8 bytes.
    }

    Register::Register(const Register& reg) {
        this->data = new uint8[8];
        for(int i = 0; i != 8; i++) {
            this->data[i] = reg.data[i];
        }
    }

    Register::Register(Register&& reg) {
        this->data = reg.data;
        reg.data = nullptr;
    }

    Register& Register::operator=(const Register& reg) {
        if(this->data) {
            delete[] this->data;
        }
        this->data = new uint8[8];
        for(int i = 0; i != 8; i++) {
            this->data[i] = reg.data[i];
        }
        return *this;
    }

    Register& Register::operator=(Register&& reg) {
        if(this->data) {
            delete[] this->data;
        }
        this->data = reg.data;
        reg.data = nullptr;
        return *this;
    }

    Register::~Register() {
        if(this->data) {
            delete[] this->data;
        }
    }

    int8 Register::get8() {
        return MemReader::get8(this->data);
    }

    int32 Register::get32() {
        return MemReader::get32(this->data);
    }

    int64 Register::get64() {
        return MemReader::get64(this->data);
    }

    int64 Register::get_by_size(int sz) {
        if(sz == 1) {
            return get8();
        } else if(sz == 4) {
            return get32();
        } else if(sz == 8) {
            return get64();
        }
        return 0;
    }

    void Register::set8(int8 val) {
        this->data[0] = val;
    }

    void Register::set32(int32 val) {
        int32* ptr = (int32*)this->data;
        ptr[0] = val;
    }

    void Register::set64(int64 val) {
        int64* ptr = (int64*)this->data;
        ptr[0] = val;
    }

    void Register::set_by_size(int64 val, int sz) {
        if(sz == 1) {
            set8(val);
        } else if(sz == 4) {
            set32(val);
        } else if(sz == 8) {
            set64(val);
        }
    }

    const unordered_set<string> Register::usable_regs = {"rax", "rbx", "rcx", "rdx", "rbp", "r0", "r1"
            "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "rsp", "sf"};

    bool Register::valid_reg(string rname) {
        return usable_regs.count(rname);
    }
}   