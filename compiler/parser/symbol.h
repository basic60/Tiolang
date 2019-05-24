#ifndef __TIO_SYMBOL
#define __TIO_SYMBOL
#include<string>
#include<vector>
namespace tio
{
    #define SYMBOL_TERMINAL 1
    #define SYMBOL_NONTERMINAL 2
    struct symbol {
        int stype;
        std::string raw;
    };
    
    struct lritem {
        symbol head;
        std::vector<symbol> body;
    };
}
#endif