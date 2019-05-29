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
        std::shared_ptr<std::vector<lritem>> items;
        LRAutomaton atm;
        SymbolTable sbtable;
    public:
        int load_grammer(const char* fpath);
        int generate_code(std::shared_ptr<Scanner> sc, std::string fpath);
        LRParser();
    };
}
#endif