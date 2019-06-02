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
        lritem fst_item = (*items)[0];
        fst_item.ahsym.insert({SYMBOL_TERMINAL, "eof"});
        atm_state stmp;
        stmp.head.push_back(fst_item);

        states.push_back(stmp);
        stack<int> s;           // stack to store state id
        s.push(0);
        while (!s.empty()) {    //  process each state
            int id = s.top();
            s.pop();

            map<pair<int, set<symbol>>,  bool> has_expanded;
            for(auto const& fst_item : states[id].head) {
                stack<symbol> hdstack;
                stack<set<symbol>> ahstack;

                if(fst_item.pos < fst_item.body.size()) {
                    hdstack.push(fst_item.body[fst_item.pos]);
                    if(fst_item.pos + 1 < fst_item.body.size()) {
                        ahstack.push(first_set[fst_item.body[fst_item.pos + 1]]);
                    } else {
                        ahstack.push(fst_item.ahsym);
                    }
                    has_expanded[make_pair(fst_item.id, ahstack.top())] = true;
                }

                while (!hdstack.empty()) {       // get all the colsure
                    symbol sb_hd = hdstack.top();
                    set<symbol> ah_dft = ahstack.top();

                    hdstack.pop();
                    ahstack.pop();
                    for(auto i : *items) {  // get the items starts with cs.top()
                        if(i.head == sb_hd) {
                            i.ahsym = ah_dft;
                            bool suc = false;
                            for(int t = 0; t < states[id].closure.size(); t++) {
                                lritem& tite = states[id].closure[t];
                                if(tite.head == i.head && tite.body.size() == i.body.size()) {
                                    suc = true;
                                    for(int k = 0; k < i.body.size(); k++) {
                                        if(tite.body[k] != i.body[k]) {
                                            suc =false;
                                        }
                                    }
                                    if(suc) {
                                        for(auto j : i.ahsym) {
                                            tite.ahsym.insert(j);
                                        }
                                        ah_dft = tite.ahsym;
                                        break;
                                    }
                                }
                            }
                            if(!suc) {
                                states[id].closure.push_back(i);                // Add a new closure item 
                            }

                            if(i.body[0].stype == SYMBOL_NONTERMINAL) {
                                hdstack.push(i.body[0]);
                                if(i.pos + 1 < i.body.size()) {
                                    ahstack.push(first_set[i.body[i.pos + 1]]);
                                } else {
                                    ahstack.push(ah_dft);
                                }
                                if(has_expanded[make_pair(i.id, ahstack.top())]) {
                                    ahstack.pop();
                                    hdstack.pop();
                                } else {
                                    has_expanded[make_pair(i.id, ahstack.top())] = true;
                                }
                            }
                        }
                    }
                }
            }

            set<symbol> sym_set;
            vector<lritem> tmp_hd;
            for(const auto& i : states[id].head) {
                if(i.pos == i.body.size()) { // process reduction
                    for(auto lk : i.ahsym) {
                        pair<int, symbol> key = make_pair(id, lk);
                        if(Action.count(key) && Action[key].first == 'r' && Action[key].second !=  i.id) {
                            LOG(ERROR)<<"Reduce-Reduce conflict! When state "<<id<<" meets "<<lk.raw<<" It can reduce to:\n"<<
                            (*items)[Action[key].second-1]<<"\tOR\n"<<i<<endl;
                            throw -1;
                        } else {
                            Action[key] = make_pair(i.id == 1 ? 'a' : 'r', i.id);
                        }        
                    }
                } else {
                    sym_set.insert(i.body[i.pos]);
                }
            }
            for(const auto& i : states[id].closure) {
                if(i.pos == i.body.size()) { // process reduction
                    for(auto lk : i.ahsym) {
                        pair<int, symbol> key = make_pair(id, lk);
                        if(Action.count(key) && Action[key].first == 'r' && Action[key].second !=  i.id) {
                            LOG(ERROR)<<"Reduce-Reduce conflict! When state "<<id<<" meets "<<lk.raw<<". It can reduce to:\n"<<
                            (*items)[Action[key].second - 1]<<"\tOR\n"<<i<<endl;
                            throw -1;
                        } else {
                            Action[key] = make_pair('r', i.id);
                        }        
                    }
                } else {
                    sym_set.insert(i.body[i.pos]);
                }
            }

            // process shift and fill the gote talbe
            for(const auto& smb : sym_set) { 
                tmp_hd.clear();
                for(auto i : states[id].head) {     // get all the items need to shift or goto
                    i.pos++;
                    if(i.body[i.pos - 1] == smb) { 
                        tmp_hd.push_back(i);
                    }
                }
                for(auto i : states[id].closure) {
                    i.pos++;
                    if(i.body[i.pos - 1] == smb) {
                        tmp_hd.push_back(i);
                    }
                }
                
                int newid = find_itemhd_in_state(tmp_hd); 
                if(newid == -1) { // create a new state
                    atm_state new_st;
                    for(auto& j : tmp_hd) {
                        new_st.head.push_back(j);
                    }
                    states.push_back(new_st);
                    s.push(states.size() - 1);
                    newid = states.size() - 1;
                }

                if(smb.stype == SYMBOL_TERMINAL) {
                    if(Action.count(make_pair(id, smb)) && Action[make_pair(id, smb)].first == 'r') {
                        LOG(ERROR)<<"Shift-Reduce conflict! When state "<<id<<" meets "<<smb.raw<<". It can reduce to:\n"<<
                        (*items)[Action[make_pair(id, smb)].second - 1];
                        throw -1;
                    }
                    Action[make_pair(id, smb)] = make_pair('s', newid);
                } else {
                    Goto[make_pair(id, smb)] = newid;
                }
            }
        }

        // ====================================TEST======================================
        //Print All the automaton states. 
        for(int i = 0; i < states.size(); i++) {
            cout<<i<<" : "<<endl;
            for(auto j : states[i].head) {
                cout<<j<<endl;
            }

            for(auto j : states[i].closure) {
                cout<<j<<endl;
            }
        }

        //Print the Action table:
        cout<<endl<<"Action:"<<endl;
        for(auto &i : Action ) {
            cout<<i.first.first<<", "<<i.first.second.raw<<": "<<i.second.first<<" "<<i.second.second<<endl;
        }

        //Print the Goto table:
        cout<<"Goto:"<<endl;
        for(auto &i : Goto ) {
            cout<<i.first.first<<", "<<i.first.second.raw<<": "<<i.second<<endl;
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

    int LRAutomaton::find_itemhd_in_state(const vector<lritem>& it) {
        for(size_t i = 0; i !=  states.size(); i++) {
            int cnt = 0;
            for(const auto& itv : it) {
                bool suc = false;
                for(const auto& j : states[i].head) {
                    if(j == itv) {
                        suc = true;
                        break;
                    }
                }
                if(!suc)  break;
                cnt++;
            }
            if(cnt == it.size()) {
                return i;
            }
        }
        return -1;
    }
}