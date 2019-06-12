#include"parser_aux.h"
#include<string>
#include<iostream>
using namespace std;
namespace tio
{
    VarUnit::VarUnit(int pcnt, std::string vname, const deque<int>& rk):ptr_cnt(pcnt), var_name(vname), rank(rk) {
        arr_size = 1;
        for(const auto& i : rank) {
            arr_size *= i;
        }
     }

    VarUnit::VarUnit():ptr_cnt(0), arr_size(1) {}

    VarUnit::VarUnit(std::string tpname, int pcnt, std::string vname, const std::deque<int>& rk):
        type_name(tpname), ptr_cnt(pcnt), var_name(vname), rank(rk) {
        arr_size = 1;
        for(const auto& i : rank) {
            arr_size *= i;
        }
    }

    FuncUnit::FuncUnit(string fname, std::string ret_tp, int pcnt, int et_line, const VarList& vl):
        func_name(fname), ret_type_name(ret_tp), ptr_cnt(pcnt), entry_line(et_line), plist(vl) {}

}