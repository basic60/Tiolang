#include<iostream>
#include<vector>
#include<fstream>
#include<memory>
#include<string>
#include<sstream>
#include"../scanner/scanner.h"
#include"../scanner/token.h"
#include"lrparser.h"
#include"log/log.h"
#include"str_helper/str_helper.h"
#include"parser_aux.h"
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
        // ofstream ofs(outpath);
        // if(ofs.fail()) {
        //     LOG(ERROR)<<"Open output file: "<<outpath<<" failed!"<<endl;
        //     throw -1;
        // }

        LOG(INFO)<<"Start generate code!"<<endl;
        if(!items->size()) {
            LOG(ERROR)<<"Empty garmmer!"<<endl;
            return -1;
        }

        stack<int> s;
        vector<symbol> line;
        s.push(0);
        bool skip = false;
        token tk;
        shared_ptr<SymbolTable> loc_table;
        int loc_offset = 0;
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
                sb.data = tk.raw_str;
            } else if(tk.token_type == TOKEN_NUMBER) {
                sb.raw = "number";
                sb.data = stoi(tk.raw_str);
            } else {
                sb.raw = tk.raw_str;
                sb.data = tk.raw_str;
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
                    line.push_back(sb);
                } else if(act == 'r') {
                    // LOG(INFO)<<"r "<<nid<<"  "<<(*items)[nid - 1]<<endl;
                    int cnt = (*items)[nid - 1].body.size();

                    symbol push_smb = (*items)[nid - 1].head;
                    // cout<<nid<<endl;
                    switch (nid) {
                    case 11: // TypeSpecifier -> __int
                    case 12: // TypeSpecifier -> __long
                    case 13: // TypeSpecifier -> __char
                    case 14: // TypeSpecifier -> __void
                        push_smb.data = line[line.size() - 1].raw;
                        break;
                    case 16: // Pointer -> __eof
                        push_smb.data = 0;
                        break;
                    case 15: // Pointer -> Pointer __*
                        push_smb.data = anytp_cast<int>(line[line.size()-2].data) + 1;
                        break;
                    case 10: // Array -> __eof
                        push_smb.data = deque<int>();
                        break;
                    case 9: { // Array -> Array __[ __number __]
                        deque<int> rktmp  = anytp_cast<deque<int>>(line[line.size() - 4].data);
                        rktmp.push_back(int(anytp_cast<int>(line[line.size() - 2].data)));
                        push_smb.data = rktmp;
                        break;
                    }
                    case 8: // varDeclID -> Pointer __id Array
                        push_smb.data = VarUnit(anytp_cast<int>(line[line.size() - 3].data),\
                                                anytp_cast<string>(line[line.size() - 2].data),\
                                                anytp_cast<deque<int>>(line[line.size() - 1].data));
                        break;
                    case 7: {  // varDeclList -> varDeclID
                        VarList vltmp;
                        vltmp.push_back(anytp_cast<VarUnit>(line[line.size() - 1].data));
                        push_smb.data = vltmp;
                        break;
                    }
                    case 6: {  // varDeclList -> varDeclList __, varDeclID
                        VarList vltmp;
                        for(auto& j : anytp_cast<VarList>(line[line.size() - 3].data)) {
                            vltmp.push_back(j);
                        }
                        vltmp.push_back(anytp_cast<VarUnit>(line[line.size() -1].data));
                        push_smb.data = vltmp;
                        break;
                    }
                    case 5: { // varDeclaration -> TypeSpecifier varDeclList __;        Return varlist
                        string tp = anytp_cast<string>(line[line.size() - 3].data);
                        
                        VarList vlist = anytp_cast<VarList>(line[line.size() - 2].data);
                        for(auto& j : vlist) {
                            j.type_name = tp;
                        }
                        push_smb.data = vlist;
                        break;
                    }
                    case 4: //declaration -> varDeclaration
                        for(const auto& j : anytp_cast<VarList>(line[line.size() - 1].data)) {
                            if(j.type_name == "void" && j.ptr_cnt == 0) {
                                LOG(ERROR)<<"Varialbe type cann't be void."<<endl;
                                throw -1;
                            }
                            sbtable.add_to_table_glb(j);
                        }
                        break;
                    case 22: { // paraList -> TypeSpecifier Pointer __id        Retrun VarList
                        VarList vlist;
                        vlist.push_back(VarUnit(
                            anytp_cast<string>(line[line.size() - 3].data), 
                            anytp_cast<int>(line[line.size() - 2].data), 
                            anytp_cast<string>(line[line.size() - 1].data),
                            deque<int>()));
                        push_smb.data = vlist;
                        break;
                    }
                    case 21: { // paraList -> paraList __, TypeSpecifier Pointer __id    Retrun VarList
                        VarList vlist = anytp_cast<VarList>(line[line.size() - 5].data);
                        vlist.push_back(VarUnit(anytp_cast<string>(line[line.size() - 3].data),
                                                anytp_cast<int>(line[line.size() - 2].data),
                                                anytp_cast<string>(line[line.size() - 1].data), deque<int>()));
                        push_smb.data = vlist;
                        break;
                    }
                    case 19: {  // params -> paraList 构建局部符号表
                        loc_table.reset(new SymbolTable());
                        loc_table->set_prev_table(&sbtable);
                        
                        code.push_back("pushl sp");
                        if(anytp_cast<string>(line[line.size() - 3].data) == "main") {
                            entry_point = code.size() - 1;
                        }

                        // funDeclaration -> TypeSpecifier Pointer __id __( paraList 符号表中添加函数
                        ftable.add_func(FuncUnit(anytp_cast<string>(line[line.size() - 3].data), 
                                                  anytp_cast<string>(line[line.size() - 5].data),
                                                  anytp_cast<int>(line[line.size() - 4].data),
                                                  code.size() - 1,
                                                  anytp_cast<VarList>(line[line.size() - 1].data)));
                        code.push_back("mov bp, sp");
                        
                        int offset = 16; // skip bp and ip in the stack, total 16 bytes
                        for(const auto& i : anytp_cast<VarList>(line[line.size() - 1].data)) {
                            if(i.type_name == "void" && i.ptr_cnt == 0) {
                                LOG(ERROR)<<"Variable cann't be void!"<<endl;
                                throw -1;
                            }
                            if(i.ptr_cnt) {
                                offset += 8;
                            } else if(i.type_name == "char") {
                                offset += 1;
                            } else if(i.type_name == "int") {
                                offset += 4;
                            } else if(i.type_name == "long") {
                                offset += 8;
                            }
                            loc_table->add_to_table_loc(i, offset);
                        }
                        break;
                    }
                    case 20: { //  params -> __eof
                        loc_table.reset(new SymbolTable());
                        loc_table->set_prev_table(&sbtable);
                        
                        code.push_back("pushl sp");
                        if(anytp_cast<string>(line[line.size() - 3].data) == "main") {
                            entry_point = code.size() - 1;
                        }

                        // funDeclaration -> TypeSpecifier Pointer __id __( paraList 符号表中添加函数
                        ftable.add_func(FuncUnit(anytp_cast<string>(line[line.size() - 3].data), 
                                                  anytp_cast<string>(line[line.size() - 5].data),
                                                  anytp_cast<int>(line[line.size() - 4].data),
                                                  code.size() - 1,
                                                  VarList()));
                        code.push_back("mov bp, sp");
                        break;
                    }
                    case 41: // localDeclarations -> varDeclaration 
                    case 40: // localDeclarations -> localDeclarations varDeclaration
                        for(auto& i : anytp_cast<VarList>(line[line.size() - 1].data)) {
                            if(i.type_name == "void" && i.ptr_cnt == 0) {
                                LOG(ERROR)<<"Variable cann't be void!"<<endl;
                                throw -1;
                            }

                            if(i.ptr_cnt) {
                                loc_offset -= 8 * i.arr_size;
                                code.push_back("sub sp, " + to_string(8 * i.arr_size));
                            } else if(i.type_name == "char") {
                                loc_offset -= 1 * i.arr_size;
                                code.push_back("sub sp, " + to_string(1 * i.arr_size));
                            } else if(i.type_name == "int") {
                                loc_offset -= 4 * i.arr_size;
                                code.push_back("sub sp, " + to_string(4 * i.arr_size));
                            } else if(i.type_name == "long") {
                                loc_offset -= 8 * i.arr_size;
                                code.push_back("sub sp, " + to_string(8 * i.arr_size));
                            } 
                            loc_table->add_to_table_loc(i, loc_offset);
                        }
                        break;
                    case 94:  // constant -> __number          
                        code.push_back("pushi " + to_string(anytp_cast<int>(line[line.size() - 1].data))); // constant -> __number
                        push_smb.data = type_info("int", 0, deque<int>()); 
                        break;
                    case 95: // constant -> __true              
                        code.push_back("pushi 1"); 
                        push_smb.data = type_info("int", 0, deque<int>()); 
                        break;
                    case 96: // constant -> __false             
                        code.push_back("pushi 0"); 
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    case 88: // immutable -> constant        
                        push_smb.data = line[line.size() - 1].data;                        
                        break;
                    case 86: // immutable -> __( expression __) 
                        push_smb.data = line[line.size() - 2].data;       
                        break;   
                    case 84: { // mutable -> __id               Return var_info
                        var_info vfo = loc_table->get_varinfo(anytp_cast<string>(line[line.size() - 1].data));
                        if(vfo.rank.size() == 0) {
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(asmc("push", vfo) + " " + to_string(vfo.addr)); 
                            } else {
                                if(vfo.addr >= 0) {
                                    code.push_back(asmc("push", vfo) + " bp+" + to_string(vfo.addr)); 
                                } else {
                                    code.push_back(asmc("push", vfo) + " bp" + to_string(vfo.addr)); 
                                }
                            }
                        }
                        push_smb.data = vfo;
                        break;
                    }
                    case 85: { // mutable -> mutable __[ expression __]         Return var_info
                        var_info vfo = anytp_cast<var_info>(line[line.size() - 4].data);
                        vfo.proc_cnt++;

                        int tsz = 1;
                        code.push_back("movi ax, 0");           // index is stored in ax
                        if(vfo.ptr_cnt == vfo.rank.size()) {
                            for(int i = vfo.rank.size() - 1; i >= 0; i--) {
                                string reg = "r" + to_string(i);
                                code.push_back("popi " + reg);
                                code.push_back("mul " + reg + ", " + to_string(tsz));
                                code.push_back("add ax, " + reg);
                                tsz *= vfo.rank[i];
                            }
                            if(vfo.addr >= 0) {
                                code.push_back("mov bx, bp+" + to_string(vfo.addr));
                            } else {
                                code.push_back("mov bx, bp" + to_string(vfo.addr));
                            }
                            code.push_back("mul ax, " + to_string(vfo.type_size));
                            code.push_back("add bx, ax"); // base address + offset address
                            code.push_back(asmc("push", vfo) + " @ rbx");
                        }

                        push_smb.data = vfo;
                        break;
                    }
                    case 82: {// factor -> mutable
                        var_info vfo = anytp_cast<var_info>(line[line.size() - 1].data);
                        if(vfo.rank.size() && vfo.proc_cnt != vfo.rank.size()) {
                            LOG(ERROR)<<"Array lack index!"<<endl;
                            throw -1;
                        }
                        push_smb.data = line[line.size() - 1].data;                   
                        break;
                    }
                    case 83: // factor -> immutable
                        push_smb.data = line[line.size() - 1].data;                   
                        break;
                    case 93: { // argList -> expression
                        ParaList plst;
                        plst.push_back(anytp_cast<type_info>(line[line.size() - 1].data));
                        push_smb.data = plst;
                        break;
                    }
                    case 92: { // argList -> argList __, expression
                        ParaList plst = anytp_cast<ParaList>(line[line.size() - 3].data);
                        plst.push_back(anytp_cast<type_info>(line[line.size() - 1].data));
                        push_smb.data = plst;
                        break;
                    }
                    case 89: { // Call -> __id __( args __)
                        func_info fio = ftable.get_func(anytp_cast<string>(line[line.size() - 4].data));
                        ParaList plst = anytp_cast<ParaList>(line[line.size() - 2].data);
                        if(fio.para_lst.size() != plst.size()) {
                            LOG(ERROR)<<"Parament count not match!"<<endl;
                            throw -1;
                        }

                        for(int i = 0 ;i != fio.para_lst.size(); i++) { // parameter type check 
                            if(!(fio.para_lst[i].type_name == plst[i].type_name && 
                                 fio.para_lst[i].ptr_cnt == plst[i].ptr_cnt)) {
                                LOG(ERROR)<<"Parameter not match!"<<endl;
                                throw -1;
                            }
                            code.push_back(asmc("pop", plst[i]) + " r" + to_string(i));
                        }

                        for(int i = fio.para_lst.size() ;i >= 0; i--) {
                            code.push_back(asmc("push", plst[i]) + " r" + to_string(i)); // 参数从右往左入栈                        
                        }

                        code.push_back("call " + to_string(fio.entry_line));

                        push_smb.data = type_info(fio.type_name, fio.ptr_cnt, deque<int>());
                        break;
                    }
                    case 87: // immutable -> Call
                        push_smb.data = line[line.size() -1].data;
                        break;
                    case 90: // args -> argList
                        push_smb.data = line[line.size() - 1].data;
                        break;
                    case 91: // args -> __eof
                        push_smb.data = ParaList();
                        break;
                    case 78: // unaryop -> __-
                    case 79: // unaryop -> __*
                    case 80: // unaryop -> __&
                    case 81: // unaryop -> __!
                        push_smb.data = line[line.size() - 1].raw;
                        break;
                    case 77: { // unaryExpression -> factor
                        type_info tp = anytp_cast<type_info>(line[line.size() - 1].data);
                        if(tp.rank.size()) {
                            tp.rank.clear();
                            tp.arr_size = 1;
                        }
                        push_smb.data = tp;
                        break;
                    }
                    case 76: { // unaryExpression -> unaryop factor 
                        var_info vfo;
                        type_info tp;
                        bool is_leftval = true;
                        try {
                            vfo = anytp_cast<var_info>(line[line.size() - 1].data);
                            tp = vfo; 
                        } catch(AnyCastException& e) {
                            if(e.exp_type == CAST_EXP_DFTP) {
                                tp = anytp_cast<type_info>(line[line.size() - 1].data);
                                is_leftval = false;
                            } else {
                                throw e;
                            }
                        }

                        if(anytp_cast<string>(line[line.size() - 1].data) == "*") {
                            if(!tp.ptr_cnt) {
                                LOG(ERROR)<<"Wrong type argument to unary \'*\' "<<endl;
                                throw -1;
                            }

                            tp.ptr_cnt--;
                            code.push_back("popl rax");
                            if(!tp.ptr_cnt) {
                                code.push_back(asmc("mov",tp) + " ax,@ ax");
                                code.push_back(asmc("push",tp) + " ax");
                                if(tp.type_name == "char") {
                                    tp.type_size = 1;
                                } else if(tp.type_name == "int") {
                                    tp.type_size = 4;
                                } else if(tp.type_name == "long") {
                                    tp.type_size = 8;
                                }
                            } else {
                                code.push_back("movl ax, @ ax");
                                code.push_back("pushl ax");
                            }
                        } else if(tp.type_name == "&") {
                            if(!is_leftval) {
                                LOG(ERROR)<<"Cann't get the address of a right value!"<<endl;
                                throw -1;
                            }

                            code.push_back(asmc("pop", tp) + " rax");    // discard the origin value
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back("pushl " + to_string(vfo.addr));
                            } else if(vfo.scope == SCOPE_LOCAL) {
                                if(vfo.addr > 0) {
                                    code.push_back("pushl bp+" + to_string(vfo.addr));
                                } else {
                                    code.push_back("pushl bp" + to_string(vfo.addr));
                                }
                            }

                            tp.ptr_cnt++;
                            tp.type_size = 8;
                        } else if(anytp_cast<string>(line[line.size() - 1].data) == "-") {
                            if(tp.ptr_cnt) {
                                LOG(ERROR)<<"Wrong type argument to unary \'-\' "<<endl;
                                throw -1;
                            }
                            code.push_back(asmc("mul", tp) + " @ sp, -1");
                        } else if(anytp_cast<string>(line[line.size() - 1].data) == "!") {
                            if(tp.ptr_cnt) {
                                LOG(ERROR)<<"Wrong type argument to unary \'!\' "<<endl;
                                throw -1;
                            }
                            code.push_back(asmc("not", tp) + " @ sp");
                        }

                        if(tp.rank.size()) {
                            tp.rank.clear();
                            tp.arr_size = 1;
                        }
                        push_smb.data = tp;
                        break;
                    }
                    case 64: // term -> unaryExpression
                        push_smb.data = line[line.size() - 1].data;
                        break;
                    case 63: { // term -> term mulop unaryExpression     // mulop两侧的类型必须相等
                        const type_info& tp = anytp_cast<type_info>(line[line.size() - 3].data);
                        const string& opt = anytp_cast<string>(line[line.size() - 2].data);
                        const type_info& other_tp = anytp_cast<type_info>(line[line.size() - 1].data);
                        if(tp != other_tp) {
                            LOG(ERROR)<<"Different types: "<<tp.to_string()<<" and "<<anytp_cast<type_info>(line[line.size() - 1].data).to_string()<<endl;
                            throw -1;
                        } else if(tp.ptr_cnt || other_tp.ptr_cnt) {
                            LOG(ERROR)<<"Cann't apply "<<opt<<" to pointer variable!"<<endl;
                            throw -1;
                        }

                        code.push_back(asmc("pop", tp) + " bx");
                        code.push_back(asmc("pop", tp) + " ax");
                        if(opt == "*") {
                            code.push_back(asmc("mul", tp) + " ax,bx");
                        } else if(opt == "/") {
                            code.push_back(asmc("div", tp) + " ax,bx");
                        } else if(opt == "%") {
                            code.push_back(asmc("mod", tp) + " ax,bx");
                        }
                        code.push_back(asmc("push", tp) + " ax");

                        push_smb.data = tp;
                        break;
                    }
                    case 73: // mulop -> __*
                        push_smb.data = string("*");
                        break;
                    case 74: // mulop -> __/
                        push_smb.data = string("/");
                        break;
                    case 75: // mulop -> __%
                        push_smb.data = string("%");
                        break;
                    case 59: { // relExpression -> sumExpression relop sumExpression
                        type_info tpl = anytp_cast<type_info>(line[line.size() - 3].data);
                        type_info tpr = anytp_cast<type_info>(line[line.size() - 3].data);
                        string opt = anytp_cast<string>(line[line.size() -2].data);
                        
                        if(tpl.ptr_cnt || tpr.ptr_cnt) {
                            LOG(ERROR)<<"Pointer type cann't rel!"<<endl;
                            throw -1;
                        }

                        code.push_back(asmc("pop", tpl) + "bx");
                        code.push_back(asmc("pop", tpl) + "ax");
                        code.push_back("cmp ax, bx");
                        string cmd;
                        if(opt == "==") {
                            cmd = "je ";
                        } else if(opt == ">=") {
                            cmd = "jge ";
                        } else if(opt == "<=") {
                            cmd = "jle ";
                        } else if(opt == ">") {
                            cmd = "jg ";
                        } else if(opt == "<") {
                            cmd = "jl ";
                        } else if(opt == "!=") {
                            cmd = "jne ";
                        }
                        code.push_back(cmd + to_string(code.size() + 2));

                        code.push_back("pushi 0");
                        code.push_back("pushi 1");
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    }
                    case 57: { // andExpression -> andExpression __&& relExpression
                        type_info tpl = anytp_cast<type_info>(line[line.size() - 3].data);   
                        type_info tpr = anytp_cast<type_info>(line[line.size() - 1].data);
                        code.push_back(asmc("pop", tpr) + " bx");   
                        code.push_back(asmc("pop", tpl) + " ax");   
                        code.push_back("and ax, bx");
                        code.push_back("pushi logr");
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    }
                    case 58: // andExpression -> relExpression
                        push_smb.data = line[line.size() -1].data;
                        break;
                    case 55: {   // simpleExpression -> simpleExpression __|| andExpression
                        type_info tpl = anytp_cast<type_info>(line[line.size() - 3].data);   
                        type_info tpr = anytp_cast<type_info>(line[line.size() - 1].data);
                        code.push_back(asmc("pop", tpr) + " bx");   
                        code.push_back(asmc("pop", tpl) + " ax");   
                        code.push_back("or ax, bx");
                        code.push_back("pushi logr");
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                        break;
                    }
                    case 54:  // expression -> simpleExpression 
                        push_smb.data = line[line.size() -1].data;
                        break;
                    case 56:    // simpleExpression -> andExpression
                        push_smb.data = line[line.size() - 1].data;
                        break;
                    case 60: // relExpression -> sumExpression
                        push_smb.data = line[line.size() - 1].data;
                        break;
                    case 65:  // relop -> __<=
                        push_smb.data = string("<=");
                        break;
                    case 66: // relop -> __<
                        push_smb.data = string("<");
                        break;
                    case 67: // relop -> __>
                        push_smb.data = string(">");
                        break;
                    case 68: // relop -> __>=
                        push_smb.data = string(">=");
                        break;
                    case 69: // relop -> __==
                        push_smb.data = string("==");
                        break;
                    case 70: // relop -> __!=
                        push_smb.data = string("!=");
                        break;        
                    case 62: // sumExpression -> term
                        push_smb.data = line[line.size() - 1].data;
                        break;
                    case 61: { // sumExpression -> sumExpression sumop term
                        type_info tpl = anytp_cast<type_info>(line[line.size() -3].data);
                        type_info tpr = anytp_cast<type_info>(line[line.size() -1].data);
                        string opt = anytp_cast<string>(line[line.size() - 2].data);
                        if(tpl != tpr) {
                            if(tpl.ptr_cnt && tpr.ptr_cnt == 0 && tpr.type_name == "int") {
                                code.push_back("popi ax"); // offset
                                code.push_back("popl bx"); // base address
                                push_smb.data = tpl;
                            }
                            else if(tpl.ptr_cnt == 0 && tpr.ptr_cnt && tpl.type_name == "int") {
                                code.push_back("popl bx"); // base address
                                code.push_back("popi ax"); // offset
                                push_smb.data = tpr;
                            } else {
                                LOG(ERROR)<<"Type different! left_type: "<<tpl.to_string()<<" right_type:"<<tpr.to_string()<<endl;
                                throw -1;
                            }

                            type_info tptmp = tpl;
                            tptmp.ptr_cnt--;
                            code.push_back("muli ax, " + to_string(raw_type_size(tptmp)));
                            if(opt == "+") {
                                code.push_back("addl bx, ax");
                            } else {
                                code.push_back("subl bx, ax");
                            }

                            code.push_back("pushl bx");                            
                        } else {
                            code.push_back(asmc("pop", tpl) + " bx");
                            code.push_back(asmc("pop", tpl) + " ax");
                            if(opt == "+") {
                                code.push_back(asmc("add", tpl) + " ax, bx");
                            } else {
                                code.push_back(asmc("sub", tpl) + " ax, bx");
                            }

                            code.push_back(asmc("push", tpl) + "ax");
                            push_smb.data = tpl;
                        }
                        break;
                    }
                    case 71: // sumop -> __+
                        push_smb.data = string("+");
                        break;
                    case 72: // sumop -> __+
                        push_smb.data = string("-");
                        break;
                    case 52: { // expression -> mutable __++
                        type_info tp = anytp_cast<type_info>(line[line.size() -2].data);
                        code.push_back(asmc("add", tp) + " @ sp, 1");
                        push_smb.data = tp;
                        break;
                    }
                    case 53: { // expression -> mutable __--
                        type_info tp = anytp_cast<type_info>(line[line.size() -2].data);
                        code.push_back(asmc("sub", tp) + " @ sp, 1");
                        push_smb.data = tp;
                        break;  
                    }
                    case 48: // expression -> mutable __+= expression
                    case 49: // expression -> mutable __-= expression
                    case 50: // expression -> mutable __*= expression
                    case 51: // expression -> mutable __/= expression
                    case 47: { // expression -> mutable __= expression
                        var_info vfo = anytp_cast<var_info>(line[line.size() - 3].data);
                        type_info tp = anytp_cast<type_info>(line[line.size() -1].data);
                        string opt = anytp_cast<string>(line[line.size() -2].data);

                        code.push_back(asmc("pop", tp) + " bx"); // expression val
                        code.push_back(asmc("pop", vfo) + " ax"); // mutable val

                        if(vfo.type_size < tp.type_size) {
                            LOG(ERROR)<<"type not match!"<<endl;
                            throw -1;
                        } else if(vfo.rank.size()) {
                            LOG(ERROR)<<"Array lacks index!"<<endl;
                            throw -1;                        
                        }

                        if(opt == "=") {
                            code.push_back(asmc("mov", tp) + " ax, bx");
                        } else if(opt == "+=") {
                            code.push_back(asmc("add", tp) + " ax, bx");
                        } else if(opt == "-=") {
                            code.push_back(asmc("sub", tp) + " ax, bx");
                        } else if(opt == "/=") {
                            code.push_back(asmc("div", tp) + " ax, bx");
                        } else if(opt == "*=") {
                            code.push_back(asmc("mul", tp) + " ax, bx");
                        } else if(opt == "%=") {
                            code.push_back(asmc("mod", tp) + " ax, bx");
                        }

                        push_smb.data = type_info(vfo.type_name, vfo.ptr_cnt, deque<int>());
                        // 类型相等赋值
                        break;
                    }
                    default:
                        break;
                    }

                    while(cnt--) {
                        s.pop();
                        line.pop_back();
                    }

                    symbol gosb = (*items)[nid - 1].head;
                    if(!atm.Goto.count(make_pair(s.top(), gosb))) {
                        LOG(ERROR)<<"Unexcepted symbol: "<<sb.raw<<" Line: "<<tk.line_num<<endl;
                        return -1;
                    } else {
                        // LOG(INFO)<<"From "<<s.top()<<" goto "<<atm.Goto[make_pair(s.top(), gosb)]<<endl;
                        s.push(atm.Goto[make_pair(s.top(), gosb)]);
                        line.push_back(push_smb);
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

        for(auto i : code) {
            cout<<i<<endl;
        }
        return 0;
    }

    LRParser::LRParser() {
        items.reset(new vector<lritem>());
    }

    string LRParser::type_suffix(const type_info& tp) {
        if(tp.ptr_cnt || tp.type_name == "long") {
            return "l";
        } else if(tp.type_name == "char") {
            return "c";
        } else if(tp.type_name == "int") {
            return "i";
        }
    }

    string LRParser::asmc(string cmd, const type_info& tp) {
        return cmd + type_suffix(tp);
    }

    int LRParser::raw_type_size(const type_info& tp) {
        if(tp.type_name == "long" || tp.ptr_cnt) {
            return 8;
        } else if(tp.type_name == "char") {
            return 1;
        } else if(tp.type_name == "int") {
            return 4;
        } 
        return 0;
    }
}