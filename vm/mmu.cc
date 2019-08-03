#include"mmu.h"
#include"log/log.h"
#include<iostream>
#include<cstdio>
using namespace std;
using namespace logger;
namespace tiovm
{
    MMU::~MMU() {
        for(auto i : page_ent) {
            if(i.second) {
                delete[] i.second;
            }
        }
    }

    uint8 MMU::get8(uint64 addr) {
        int pid = getid(addr);
        if(!page_ent.count(pid)) {
            page_ent[pid] = new uint8[VM_PAGE_SIZE];
            LOG(INFO)<<"Create a new page!"<<endl;
        }
        return page_ent[pid][getoff(addr)];
    }

    uint32 MMU::get32(uint64 addr) {
        int pid = getid(addr);int npid = pid + 1;
        int offset = getoff(addr);
        if(!page_ent.count(pid)) {
            page_ent[pid] = new uint8[VM_PAGE_SIZE];
            LOG(INFO)<<"Create a new page!"<<endl;
            if(offset + 3 >= VM_PAGE_SIZE) {
                if(!page_ent.count(npid)) {
                    page_ent[npid] = new uint8[VM_PAGE_SIZE];
                    LOG(INFO)<<"Create a new page!"<<endl;
                }
            }
        }

        uint32 ret = 0;
        int i = 0;int swt = 0;
        if(offset + 3 >= VM_PAGE_SIZE) {
            while (i < 4) {
                ret |= (uint32)page_ent[swt == 0 ? pid : npid][offset++]<<(i * 8);
                i++;
                if(offset >= VM_PAGE_SIZE) {
                    offset = 0;
                    swt = 1;
                }
            }
            return ret;
        } else {
            return *(uint32*)(page_ent[pid] + getoff(addr));
        }
    }
    
    uint64 MMU::get64(uint64 addr) {
        int pid = getid(addr);int npid = pid + 1;
        int offset = getoff(addr);
        if(!page_ent.count(pid)) {
            page_ent[pid] = new uint8[VM_PAGE_SIZE];
            LOG(INFO)<<"Create a new page!"<<endl;
            if(offset + 7 >= VM_PAGE_SIZE) {
                if(!page_ent.count(npid)) {
                    page_ent[npid] = new uint8[VM_PAGE_SIZE];
                    LOG(INFO)<<"Create a new page!"<<endl;
                }
            }
        }


        uint64 ret = 0;
        int i = 0;int swt = 0;
        if(offset + 7 >= VM_PAGE_SIZE) {
            while (i < 8) {
                ret |= (uint64)(page_ent[swt == 0 ? pid : npid][offset++]) << (i * 8);
                i++;
                if(offset >= VM_PAGE_SIZE) {
                    offset = 0;
                    swt = 1;
                }
            }
            return ret;
        } else {
            return *(uint64*)(page_ent[pid] + getoff(addr));
        }
    }

    uint64 MMU::get_by_size(uint64 addr, int sz) {
        if(sz == 1) {
            return get8(addr);
        } else if(sz == 4) {
            return get32(addr);
        } else if(sz == 8) {
            return get64(addr);
        }
        return 0;
    } 

    void MMU::set8(uint64 addr, uint8 val) {
        int pid = getid(addr);
        if(!page_ent.count(pid)) {
            page_ent[pid] = new uint8[VM_PAGE_SIZE];
            LOG(INFO)<<"Create a new page!"<<endl;
        }
        page_ent[pid][getoff(addr)] = val;   
    }

    void MMU::set32(uint64 addr, uint32 val) {
        int pid = getid(addr);int npid = pid + 1;
        int offset = getoff(addr);
        if(!page_ent.count(pid)) {
            page_ent[pid] = new uint8[VM_PAGE_SIZE];
            LOG(INFO)<<"Create a new page!"<<endl;
            if(offset + 3 >= VM_PAGE_SIZE) {
                if(!page_ent.count(npid)) {
                    page_ent[npid] = new uint8[VM_PAGE_SIZE];
                    LOG(INFO)<<"Create a new page!"<<endl;
                }
            }
        }

        int i = 0;int swt = 0;
        if(offset + 3 >= VM_PAGE_SIZE) {
            while (i < 4) {
                page_ent[swt == 0 ? pid : npid][offset++] = val >> (i * 8);
                i++;
                if(offset >= VM_PAGE_SIZE) {
                    offset = 0;
                    swt = 1;
                }
            }
        } else {
            uint32* ptr = (uint32*)(page_ent[pid] + offset);
            ptr[0] = val;
        }
    }

    void MMU::set64(uint64 addr, uint64 val) {
        int pid = getid(addr);int npid = pid + 1;
        int offset = getoff(addr);
        if(!page_ent.count(pid)) {
            page_ent[pid] = new uint8[VM_PAGE_SIZE];
            LOG(INFO)<<"Create a new page!"<<endl;
            if(offset + 7 >= VM_PAGE_SIZE) {
                if(!page_ent.count(npid)) {
                    page_ent[npid] = new uint8[VM_PAGE_SIZE];
                    LOG(INFO)<<"Create a new page!"<<endl;
                }
            }
        }

        int i = 0;int swt = 0;
        if(offset + 7 >= VM_PAGE_SIZE) {
            while (i < 8) {
                page_ent[swt == 0 ? pid : npid][offset++] = val >> (i * 8);
                i++;
                if(offset >= VM_PAGE_SIZE) {
                    offset = 0;
                    swt = 1;
                }
            }
        } else {
            uint64* ptr = (uint64*)(page_ent[pid] + offset);
            ptr[0] = val;
        }
    }

    void MMU::set_by_size(uint64 addr, uint64 val, int sz) {
        if(sz == 1) {
            set8(addr, val);
        } else if(sz == 4) {
            set32(addr, val);
        } else if(sz == 8) {
            set64(addr, val);
        }
    }
}