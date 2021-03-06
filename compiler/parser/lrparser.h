#ifndef __TIO_LRPARSER
#define __TIO_LRPARSER
#include<string>
#include<memory>
#include<vector>
#include"../scanner/token.h"
#include"../scanner/scanner.h"
#include"symbol.h"
#include"lrautomaton.h"
#include"symbol_table.h"
namespace tio
{
    class LRParser {
    private:
        std::vector<std::string> code;
        std::shared_ptr<std::vector<lritem>> items;
        std::stack<unsigned long> false_list;
        std::vector<std::vector<unsigned long>> true_list;
        std::stack<unsigned long> while_head;
        std::stack<unsigned long> loop_exit;
        void process_if();
        void process_elif_st();
        void process_while();
        LRAutomaton atm;
        SymbolTable sbtable;    // Global symbol table
        FuncTable ftable;
        int entry_point;
        
        std::string type_suffix(const type_info& tp);
        std::string format_command(std::string pattern, int sz, ...);
        int raw_type_size(const type_info& tp);
    public:
        int load_grammer(const char* fpath);
        int generate_code(std::shared_ptr<Scanner> sc, std::string fpath);
        LRParser();
    };
}
#endif