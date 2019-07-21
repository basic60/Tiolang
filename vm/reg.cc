#include"reg.h"
#include"mem_helper.h"
using namespace std;
namespace tiovm
{
    Register::Register() {
        this->data = new uint8[8];  // A regisger has 8 bytes.
    }

    Register::~Register() {
        delete[] this->data;
    }

    uint8 Register::get8() {
        return MemReader::get8(this->data);
    }

    uint32 Register::get32() {
        return MemReader::get32(this->data);
    }

    uint64 Register::get64() {
        return MemReader::get64(this->data);
    }

    void Register::set8(uint8 val) {
        this->data[0] = val;
    }

    void Register::set32(uint32 val) {
        uint32* ptr = (uint32*)this->data;
        ptr[0] = val;
    }

    void Register::set64(uint64 val) {
        uint64* ptr = (uint64*)this->data;
        ptr[0] = val;
    }

    const unordered_set<string> Register::usable_regs = {"rax", "rbx", "rcx", "rdx", "rbp", "r0", "r1"
            "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"};

    bool Register::valid_reg(string rname) {
        return usable_regs.count(rname);
    }
}   