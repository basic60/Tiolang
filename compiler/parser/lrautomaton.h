#ifndef __TIO_LRAUTOMATON
#define __TIO_LRAUTOMATON
#include<string>
#include<vector>
#include<memory>
#include<map>
#include<set>
#include<stack>
#include"symbol.h"
namespace tio
{
    struct atm_state {
        std::vector<lritem> head;
        std::vector<lritem> closure;
    };

    class LRAutomaton {
    private:
        std::vector<atm_state> states;
        std::shared_ptr<std::vector<lritem>> items;
        int find_itemhd_in_state(const std::vector<lritem>& it);
        void cal_first(const symbol& smb);
        std::map<symbol, std::set<symbol>> first_set;
    public:
        std::map<std::pair<int, symbol>, int> Goto;
        std::map<std::pair<int, symbol>, std::pair<char, int>> Action;
        LRAutomaton();
        void build_automaton();
        void set_items(std::shared_ptr<std::vector<lritem>> itm);
    };
}
#endif