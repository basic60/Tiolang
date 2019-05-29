#ifndef __TIO_SYMBOL_TABLE
#define __TIO_SYMBOL_TABLE
#include<memory>
#include<map>
#include"lrparser.h"
#include"parser_aux.h"
namespace tio
{
    class SymbolTable {
    private:
        std::shared_ptr<SymbolTable> prev;
        std::shared_ptr<SymbolTable> next;

        struct var_info {
            std::string type_name;
            int type_size;
            int ptr_size;
            int arr_size;
            long long addr;

            var_info(std::string tn, int tsz, int psz, int asz);
            var_info();
            
            // var_info& operator=(const va_list& aa);
        };

        std::map<std::string, var_info> table;
    public:
        void add_to_table(const std::string& tname, const int& tsize, const VarUnit& vu);
        var_info get_varinfo(const std::string& vname);
    };
}
#endif