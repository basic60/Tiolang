#ifndef __TIO_SYMBOL
#define __TIO_SYMBOL
#include<string>
#include<vector>
#include<set>
#include<ostream>
#include"anytype.h"
namespace tio
{
    #define SYMBOL_TERMINAL 1
    #define SYMBOL_NONTERMINAL 2
    struct symbol {
        int stype;  // Ternminal or Nonterminal
        std::string raw;
        anytp data;

        symbol(int tp, std::string v);
        symbol();
        bool operator==(const symbol& other) const;
        bool operator!=(const symbol& other) const;
        bool operator<(const symbol& other) const;
    };
    
    struct lritem {
        int id;
        symbol head;
        std::vector<symbol> body;

        int pos;
        std::set<symbol> ahsym;  // look ahead symbol

        lritem();

        bool operator==(const lritem& other) const;

        friend std::ostream& operator<<(std::ostream & os, const lritem& s);
    };
}
#endif