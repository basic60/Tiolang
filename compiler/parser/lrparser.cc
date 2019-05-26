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
        
        return 0;
    }

    LRParser::LRParser() {
        items.reset(new vector<lritem>());
    }
}