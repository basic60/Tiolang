#ifndef __TIO_LRPARSER
#define __TIO_LRPARSER
#include<string>
#include<memory>
#include<vector>
#include"../scanner/token.h"
#include"symbol.h"
#include"lrautomaton.h"
namespace tio
{
    class LRParser {
    private:
        std::shared_ptr<std::vector<lritem>> items;
        LRAutomaton atm;
    public:
        int load_grammer(const char* fpath);
        int generate_code(std::shared_ptr<Scanner> sc, std::string fpath);
        LRParser();
    };
}
#endif