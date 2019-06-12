#ifndef __TIO_PARSERAUX
#define __TIO_PARSERAUX
#include<string>
#include<vector>
#include<deque>
namespace tio
{
    struct VarUnit {
        int ptr_cnt;
        std::string type_name;
        std::string var_name;
        int arr_size;
        std::deque<int> rank;
        VarUnit(int pcnt, std::string vname, const std::deque<int>& rk);
        VarUnit(std::string type_name, int pcnt, std::string vname, const std::deque<int>& rk);
        VarUnit();
    };

    using VarList = std::vector<VarUnit>;

    using LocalDeclList = std::vector<VarList>;

    struct FuncUnit {
        std::string func_name;
        std::string ret_type_name;
        int ptr_cnt;
        int entry_line;
        VarList plist;
        FuncUnit(std::string fname, std::string ret_tp, int pcnt, int line, const VarList& vl);
    };
}
#endif