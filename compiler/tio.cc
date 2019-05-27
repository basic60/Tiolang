#include<iostream>
#include<fstream>
#include<memory>
#include<vector>
#include<map>
#include<string>
#include"log/log.h"
#include"scanner/token.h"
#include"scanner/scanner.h"
#include"parser/lrparser.h"
using namespace std;
using namespace tio;
void test_scanner(shared_ptr<Scanner> sc) {
    map<int ,string> mp;mp[1] = "id";mp[2] = "keyword";mp[3] = "spliter";; mp[4] = "number"; mp[5] = "operator";
    while(sc->tkcount()) {
        token tk = sc->get_next_token();
        cout<<tk.line_num<<" type:["<<mp[tk.token_type]<<"]\traw_str: "<<tk.raw_str<<endl;
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) {
            cerr<<"Usage: tio <filename>"<<endl;
            exit(-1);
        }

        FILE* fp = fopen(argv[1], "r");
        if(!fp) {
            cerr<<"Open "<<argv[1]<<" failed!"<<endl;
            exit(-1);
        }

        shared_ptr<Scanner> sc(new Scanner());
        sc->scan(fp);
        // test_scanner(sc);        
        shared_ptr<LRParser> pr(new LRParser());
        pr->load_grammer("grammer.txt");
        pr->generate_code(sc, "code.log");
        fclose(fp);
    } catch(int& val) {
        cerr<<"exit code: "<<val<<endl;
        exit(val);
    }
    return 0;
}