#include"lrparser.h"
#include"symbol_table.h"
#include"log/log.h"
#include"memory_manager.h"
#include<iostream>
#include"parser_aux.h"
#include<memory>
#include<deque>
using namespace std;
using namespace logger;
namespace tio
{
    type_info::type_info(string tn, int pcnt, const deque<int>& rk):
        type_name(tn), ptr_cnt(pcnt), rank(rk) {
        if(ptr_cnt > 0) {
            type_size = POINTER_SIZE;
        } else if(type_name == "int") {
            type_size = INT_SIZE;
        } else if(type_name == "long") {
            type_size = LONG_SIZE;
        } else if(type_name == "char") {
            type_size = CHAR_SIZE;
        } else if(type_name == "void") {
            type_size = 0;
        }
        
        arr_size = 1;
        for(const auto& i : rank){
            arr_size *= i;
        }
    }

    func_info::func_info() {}
    
    type_info::type_info() {}

    var_info::var_info():proc_cnt(0) {}

    var_info::var_info(const VarUnit& vu, long long address, int scp) : 
       type_info(vu.type_name, vu.ptr_cnt, vu.rank), addr(address), scope(scp), proc_cnt(0) { }

    void SymbolTable::add_to_table_glb(const VarUnit& vu) {
        if(table.count(vu.var_name)) {
            LOG(ERROR)<<"Duplicate declaration of: "<<vu.var_name<<endl;
            throw -1;
        } 
        this->table[vu.var_name] = var_info(vu, 0, SCOPE_GLOBAL);
        this->table[vu.var_name].addr = MemoryManager::alloc_static(this->table[vu.var_name].type_size * this->table[vu.var_name].arr_size);
    }

    void SymbolTable::set_prev_table(SymbolTable* pre) {
        this->prev = pre;
    }

    func_info::func_info(const FuncUnit& fu):
        type_info(fu.ret_type_name, fu.ptr_cnt, deque<int>()), para_lst(fu.plist), entry_line(fu.entry_line) {}

    func_info FuncTable::get_func(const std::string& fname) {
        if(!table.count(fname)) {
            LOG(ERROR)<<"Function "<<fname<<" not found!"<<endl;
            throw -1;
        }
        return table[fname];
    }
        

    var_info SymbolTable::get_varinfo(const string& vname) {
        if(table.count(vname)) {
            return table[vname];
        }
        if(prev) {
            return prev->get_varinfo(vname); 
        }
        LOG(ERROR)<<"Defination of "<<vname<<" is not found!"<<endl;
        throw -1;
    }

    void FuncTable::add_func(const FuncUnit& fu) {
        if(table.count(fu.func_name)) {
            LOG(ERROR)<<"Duplicate declaration of function: "<<fu.func_name<<endl;
        }
        table[fu.func_name] = func_info(fu);
    }

    void SymbolTable::add_to_table_loc(const VarUnit& vu, int offset) {
        if(table.count(vu.var_name)) {
            LOG(ERROR)<<"Duplicate declaration of: "<<vu.var_name<<endl;
            throw -1;
        } 
        
        this->table[vu.var_name] = var_info(vu, offset, SCOPE_LOCAL);
    }

    bool type_info::operator==(const type_info& other) const {
        if(type_name == other.type_name && ptr_cnt == other.ptr_cnt && arr_size == other.arr_size && rank.size() == other.rank.size()) {
            for(size_t i =0; i != rank.size(); i++) {
                if(rank[i] != other.rank[i]) {
                    return false;
                }
            }
        } else {
            return false;
        }
        return true;
    }

    bool type_info::operator!=(const type_info& other) const {
        return !(*this == other);
    }

    string type_info::to_string() const {
        string tmp;
        tmp += type_name;
        int cnt = ptr_cnt;
        while (cnt--) {
            tmp += "*";
        }
        tmp += " ";
        if(rank.size()) {
            for(auto i : rank) {
                tmp += "[" + std::to_string(i) + "]";
            }
        }
        return tmp;
    }


}
