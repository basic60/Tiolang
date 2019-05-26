#include<string>
#include<vector>
#include<memory>
#include<stack>              
#include<set>
#include<iostream>
#include<algorithm>
#include"symbol.h"
#include"lrautomaton.h"
#include"log/log.h"
using namespace std;
namespace tio
{
    LRAutomaton::LRAutomaton() {}

    void LRAutomaton::set_items(shared_ptr<vector<lritem>> itm) {
        this->items = itm;

        // initilize the first set 
        for(auto i : *items) {
            if(first_set[i.head].size() == 0) cal_first(i.head);
            for(auto j : i.body) {
                if(first_set[j].size() == 0) cal_first(j);
            }
        }
    }

    void LRAutomaton::build_automaton() {
        lritem fst_itm = (*items)[0];
        fst_itm.ahsym.insert({SYMBOL_TERMINAL, "eof"});
        atm_state stmp;
        stmp.head = fst_itm;

        states.push_back(stmp);
        stack<int> s;           // stack to store state id
        s.push(0);
        while (!s.empty()) {    //  process each state
            int id = s.top();
            s.pop();

            const lritem& fst_item = states[id].head;
            stack<symbol> hdstack;
            stack<set<symbol>> ahstack;
            map<symbol, bool> has_expanded;

            if(fst_item.pos < fst_item.body.size()) {
                hdstack.push(fst_item.body[fst_item.pos]);
                has_expanded[fst_item.body[fst_item.pos]] = true;
                if(fst_item.pos + 1 < fst_item.body.size()) {
                    ahstack.push(first_set[fst_item.body[fst_item.pos + 1]]);
                } else {
                    ahstack.push(fst_item.ahsym);
                }
            }

            while (!hdstack.empty()) {       // get all the colsure
                const symbol sb_hd = hdstack.top();
                const set<symbol> ah_dft = ahstack.top();

                hdstack.pop();
                ahstack.pop();
                for(auto i : *items) {  // get the items starts with cs.top()
                    if(i.head == sb_hd) {
                        i.ahsym = ah_dft;
                        states[id].closure.push_back(i);                // Add a new closure item 
                        if(i.body[0].stype == SYMBOL_NONTERMINAL && !has_expanded[i.body[0]]) {
                            has_expanded[i.body[0]] = true;
                            hdstack.push(i.body[0]);
                            if(i.pos + 1 < i.body.size()) {
                                ahstack.push(first_set[i.body[i.pos + 1]]);
                            } else {
                                ahstack.push(ah_dft);
                            }
                        }
                    }
                }
            }

            // Fill the action and goto table.
            for(auto i : states[id].closure) {
                fill_table(i, id, s);        
            }
            
            fill_table(states[id].head, id, s);
        }

        // ====================================TEST======================================
        // Print All the automaton states. 
        // for(int i = 0; i < states.size(); i++) {
        //     cout<<i<<" : "<<endl;
        //     cout<<states[i].head<<endl;

        //     for(auto j : states[i].closure) {
        //         cout<<j<<endl;
        //     }
        // }

        // Print the Action table:
        // cout<<endl<<"Action:"<<endl;
        // for(auto &i : Action ) {
        //     cout<<i.first.first<<", "<<i.first.second.raw<<": "<<i.second.first<<" "<<i.second.second<<endl;
        // }

        // Print the Goto table:
        // cout<<"Goto:"<<endl;
        // for(auto &i : Goto ) {
        //     cout<<i.first.first<<", "<<i.first.second.raw<<": "<<i.second<<endl;
        // }
    }

    void LRAutomaton::fill_table(lritem i, int id, stack<int>& s) {
        if(i.pos == i.body.size()) { // reduction
            for(auto j : i.ahsym) {
                if(i.id == 1) {
                    Action[make_pair(id,j)] = make_pair('a', i.id);
                } else {
                    Action[make_pair(id,j)] = make_pair('r', i.id);
                }
            }
        } else {
            i.pos++;
            int newid = find_item_in_state(i);
            if(newid == -1) {       // Create the new state;
                atm_state new_st;
                new_st.head = i;
                states.push_back(new_st);
                s.push(states.size() - 1);
                if(i.body[i.pos - 1].stype == SYMBOL_TERMINAL) {
                    Action[make_pair(id, i.body[i.pos - 1])] = make_pair('s', states.size() - 1);
                } else {
                    Goto[make_pair(id, i.body[i.pos - 1])] = states.size() -1;
                }
            } else {
                if(i.body[i.pos - 1].stype == SYMBOL_TERMINAL) {
                    Action[make_pair(id, i.body[i.pos - 1])] = make_pair('s', newid);
                } else {
                        Goto[make_pair(id, i.body[i.pos - 1])] = newid;
                }
            }
        }
    }


    void LRAutomaton::cal_first(const symbol& sb) {
        if(sb.stype == SYMBOL_TERMINAL) {
            first_set[sb].insert(sb);
        }

        int loop_time = 0;
        vector<bool> dend(items->size() + 10, 0);
        bool flag = true;
        while(flag) {
            flag = false;
            for(auto i : *items) {
                if(i.head == sb) {
                    if(loop_time == 0 && i.body[0].stype == SYMBOL_TERMINAL) {
                        first_set[sb].insert(i.body[0]);
                        dend[i.id] = true;
                        flag = true;
                        continue;
                    }

                    if(loop_time == 0) {
                        if(i.body[0] != i.head) {
                            cal_first(i.body[0]);
                            for(auto res : first_set[i.body[0]]) {
                                first_set[sb].insert(res);
                            }
                            flag = true;
                        }
                    } else if(loop_time < i.body.size()) {
                        if(!dend[i.id] && first_set[i.body[loop_time - 1]].count({SYMBOL_TERMINAL, "eof"})) {
                            cal_first(i.body[loop_time]);
                            for(auto res : first_set[i.body[loop_time]]) {
                                first_set[sb].insert(res);
                            }
                            flag = true;
                        } else {
                            dend[i.id] = true;
                        }
                    }
                }
            }
            loop_time++;
        }
    }

    int LRAutomaton::find_item_in_state(const lritem& it) {
        for(size_t i = 0; i !=  states.size(); i++) {
            if(states[i].head == it) {
                return i;
            }
        }
        return -1;
    }
}