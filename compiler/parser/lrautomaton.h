#ifndef __TIO_LRAUTOMATON
#define __TIO_LRAUTOMATON
#include<string>
#include<vector>
#include<memory>
#include<map>
#include<set>
#include"symbol.h"
namespace tio
{
    struct atm_state {
        lritem head;
        std::vector<lritem> closure;
    };

    class LRAutomaton {
    private:
        std::vector<atm_state> states;
        std::shared_ptr<std::vector<lritem>> items;
        int find_item_in_state(const lritem& it);
        void cal_first(const symbol& smb);
        void fill_table(lritem itm, int id, std::stack<int>& s);
        std::map<symbol, std::set<symbol>> first_set;
        
        std::map<std::pair<int, symbol>, int> Goto;
        std::map<std::pair<int, symbol>, std::pair<char, int>> Action;
    public:
        LRAutomaton();
        void build_automaton();
        void set_items(std::shared_ptr<std::vector<lritem>> itm);
    };


}
#endif