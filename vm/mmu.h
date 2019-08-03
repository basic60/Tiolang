#ifndef __TIOVM_MMU
#define __TIOVM_MMU
#include<map>
#include"mem_helper.h"
namespace tiovm
{
    #define VM_PAGE_SIZE 4194304    // Page size is 4KB
    class MMU {
    private:
        std::map<int, uint8*> page_ent;
        
        // Get the page id of the address.
        inline int getid(uint64 addr) {
            return addr / VM_PAGE_SIZE;
        }

        inline int getoff(uint64 addr) {
            return addr % VM_PAGE_SIZE;            
        }
    public:
        ~MMU();

        uint8 get8(uint64 addr);
        uint32 get32(uint64 addr);
        uint64 get64(uint64 addr);
        uint64 get_by_size(uint64 addr, int sz);
        void set8(uint64 addr, uint8 val);
        void set32(uint64 addr, uint32 val);
        void set64(uint64 addr,uint64 val);
        void set_by_size(uint64 addr, uint64 val, int sz);
    };
}
#endif