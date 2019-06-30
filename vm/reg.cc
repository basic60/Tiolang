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
}   