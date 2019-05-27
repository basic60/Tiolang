#include<iostream>
#include<vector>
#include<fstream>
#include<memory>
#include<string>
#include"../scanner/scanner.h"
#include"../scanner/token.h"
#include"lrparser.h"
#include"log/log.h"
#include"str_helper/str_helper.h"
using namespace std;
namespace tio
{
    int LRParser::load_grammer(const char* fpath) {
        ifstream ifs(fpath);
        if(ifs.fail()) {
            LOG(ERROR)<<"Open grammer file: "<<fpath<<" failed!"<<endl;
            return -1;
        }

        string line;
        int cnt = 0;
        while (getline(ifs, line)) {
            line = str::strip(line);
            if(line[0] == '#' || line == "") {
                continue;
            }

            vector<string> segs = str::split(line, ' ');
            lritem itmp;
            for(int i = 0; i != segs.size(); i++) {
                if(i == 0) {
                    if(segs[i][0] == '_' && segs[i][1] == '_') {
                        LOG(ERROR)<<"Terminal cann't deduce!"<<endl;
                        return -1;
                    }
                    itmp.head = {SYMBOL_NONTERMINAL, segs[i]};
                    continue;
                } else if(segs[i] == "->") {
                    continue;
                }
                
                if(segs[i][0] == '_' && segs[i][1] == '_') {
                    itmp.body.push_back({SYMBOL_TERMINAL, segs[i].substr(2)});
                } else {
                    itmp.body.push_back({SYMBOL_NONTERMINAL, segs[i]});
                }
            }
            itmp.id = ++cnt;
            items->push_back(itmp);
        }

        //print grammer
        // for(auto i : *items) {
        //     cout<<i.head.raw<<":"<<i.head.stype<<" -> ";
        //     for(auto j : i.body) {
        //         cout<<j.raw<<":"<<j.stype<<" ";
        //     }
        //     cout<<endl;
        // }

        atm.set_items(items);
        atm.build_automaton();
        return 0;
    }

    int LRParser::generate_code(shared_ptr<Scanner> sc, string outpath) {
        if(!items->size()) {
            LOG(ERROR)<<"Empty garmmer!"<<endl;
            return -1;
        }

        stack<int> s;
        s.push(0);
        bool skip = false;
        token tk;
        while(sc->tkcount() || skip) {
            if(!skip) {
                tk = sc->get_next_token();
            }
            // cout<<"cur tk: "<<tk.raw_str<<endl;

            skip = false;
            symbol sb;
            sb.stype = SYMBOL_TERMINAL;
            if(tk.token_type == TOKEN_ID) {
                sb.raw = "id";
            } else if(tk.token_type == TOKEN_NUMBER) {
                sb.raw = "number";
            } else {
                sb.raw = tk.raw_str;
            }

            if(!atm.Action.count(make_pair(s.top(), sb)) &&\
             !atm.Action.count(make_pair(s.top(), symbol(SYMBOL_TERMINAL, "eof")))) {
                LOG(ERROR)<<"Unexcepted token: "<<sb.raw<<" Line: "<<tk.line_num<<endl;
                return -1;
            } else {
                char act; 
                int nid; 
                if(atm.Action.count(make_pair(s.top(), sb))) {
                    act = atm.Action[make_pair(s.top(), sb)].first;
                    nid = atm.Action[make_pair(s.top(), sb)].second;
                } else {
                    act = atm.Action[make_pair(s.top(), symbol(SYMBOL_TERMINAL, "eof"))].first;
                    nid = atm.Action[make_pair(s.top(), symbol(SYMBOL_TERMINAL, "eof"))].second;
                    skip = true;
                }

                if(act == 's') {
                    // LOG(INFO)<<"s"<<" "<<nid<<" "<<endl;
                    s.push(nid);
                } else if(act == 'r') {
                    // LOG(INFO)<<"r "<<nid<<"  "<<(*items)[nid - 1]<<endl;
                    int cnt = (*items)[nid - 1].body.size();
                    while(cnt--) s.pop();

                    symbol gosb = (*items)[nid - 1].head;
                    if(!atm.Goto.count(make_pair(s.top(), gosb))) {
                        LOG(ERROR)<<"Unexcepted symbol: "<<sb.raw<<" Line: "<<tk.line_num<<endl;
                        return -1;
                    } else {
                        // LOG(INFO)<<"From "<<s.top()<<" goto "<<atm.Goto[make_pair(s.top(), gosb)]<<endl;
                        s.push(atm.Goto[make_pair(s.top(), gosb)]);
                        skip = true;
                    }
                } else if(act == 'a') {
                    if(sc->tkcount() || sb.raw != "eof") {
                        LOG(ERROR)<<"Reduce finished. But still has token left!"<<endl;
                        return -1;
                    }
                    break;
                }
            }
        }
        LOG(INFO)<<"Accepted!"<<endl;
        return 0;
    }

    LRParser::LRParser() {
        items.reset(new vector<lritem>());
    }
}