#include"lrparser.h"
#include"symbol_table.h"
#include"log/log.h"
#include"memory_manager.h"
#include<iostream>
#include"parser_aux.h"
#include<memory>
using namespace std;
namespace tio
{
    SymbolTable::var_info::var_info(std::string tn, int tsz, int psz, int asz): type_name(tn), \
                                    type_size(tsz), ptr_size(psz), arr_size(asz), addr(0) {}
    
    SymbolTable::var_info::var_info() {
        addr = 0;
    }

    // SymbolTable::var_info& SymbolTable::var_info::operator=(const var_info& other) {
        
    // }

    void SymbolTable::add_to_table(const VarUnit& vu) {
        if(table.count(vu.var_name)) {
            LOG(ERROR)<<"Duplicate declaration of: "<<vu.var_name<<endl;
            throw -1;
        } 
        int tsize = 0;
        if(vu.ptr_cnt) {
            tsize = POINTER_SIZE;
        } else if(vu.type_name == "int") {
            tsize = INT_SIZE;
        } else if(vu.type_name == "long") {
            tsize = LONG_SIZE;
        } else if(vu.type_name == "char") {
            tsize = CHAR_SIZE;
        }
        this->table[vu.var_name] = var_info(vu.type_name, tsize, vu.ptr_cnt, vu.arr_size);
        this->table[vu.var_name].addr = MemoryManager::alloc_static(this->table[vu.var_name].type_size * this->table[vu.var_name].arr_size);
    }

    SymbolTable::var_info SymbolTable::get_varinfo(const string& vname) {
        while(true) {
            if(table.count(vname)) {
                return table[vname];
            }
            if(prev) {
               return prev->get_varinfo(vname); 
            } else {
                break;
            }
        }
        LOG(ERROR)<<"Defination of "<<vname<<" is not found!"<<endl;
        throw -1;
    }
}
