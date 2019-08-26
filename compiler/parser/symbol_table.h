#ifndef __TIO_SYMBOL_TABLE
#define __TIO_SYMBOL_TABLE
#include<memory>
#include<map>
#include<deque>
#include"lrparser.h"
#include"parser_aux.h"
namespace tio
{
    #define POINTER_SIZE 8
    #define INT_SIZE 4
    #define LONG_SIZE 8
    #define CHAR_SIZE 1

    #define SCOPE_GLOBAL 1
    #define SCOPE_LOCAL 2



    struct type_info {      //  符号表记录类型数据基类
        std::string type_name;
        int ptr_cnt;
        std::deque<int> rank;
        int arr_size;
        int type_size;
        
        type_info(std::string tp_name, int pcnt, const std::deque<int>& rk);
        type_info();

        bool operator==(const type_info& other) const;
        bool operator!=(const type_info& other) const;
        std::string to_string() const;
    };

    struct var_info : public type_info {      //  符号表存储的变量信息。
        long long addr;
        int scope;      // 全局变量的地址是绝对地址，局部变量的地址是距离rbp的偏移。
        int proc_cnt;
        var_info(const VarUnit& vu, long long addr, int scp);
        var_info();
    };

    struct func_info : public type_info {      //  符号表存储的函数信息。
        int entry_line;
        VarList para_lst;
        func_info(const FuncUnit& fu);
        func_info();
    };

    using ParaList = std::vector<type_info>;

    class SymbolTable {
    private:
        SymbolTable* prev;
        int tot_size;

        std::map<std::string, var_info> table;
    public:
        void add_to_table_glb(const VarUnit& vu);
        void add_to_table_loc(const VarUnit& vu, int offset);
        void set_prev_table(SymbolTable* pre);
        
        var_info get_varinfo(const std::string& vname);
    };

    class FuncTable {
    private:
        std::map<std::string, func_info> table;
    public:
        void add_func(const FuncUnit& fu);

        func_info get_func(const std::string& fname);
    };
}
#endif