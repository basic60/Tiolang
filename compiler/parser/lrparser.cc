#include<iostream>
#include<vector>
#include<fstream>
#include<memory>
#include<string>
#include<sstream>
#include<stdarg.h>
#include"../scanner/scanner.h"
#include"../scanner/token.h"
#include"lrparser.h"
#include"log/log.h"
#include"str_helper/str_helper.h"
#include"parser_aux.h"
using namespace logger;
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
        ofstream ofs(outpath);
        if(ofs.fail()) {
            LOG(ERROR)<<"Open output file: "<<outpath<<" failed!"<<endl;
            throw -1;
        }

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

            if(!atm.Action.count(make_pair(s.top(), sb)) &&
             !atm.Action.count(make_pair(s.top(), symbol(SYMBOL_TERMINAL, "eof")))) {
                LOG(ERROR)<<"Unexcepted token: "<<sb.raw<<" Line: "<<tk.line_num<<endl;
                return -1;
            } else {
                char act; // action, 's' means shift and 'r' means reduce
                int nid;  // Shift to state i or reduce by item[nid -1]
                if(atm.Action.count(make_pair(s.top(), sb))) {
                    act = atm.Action[make_pair(s.top(), sb)].first;
                    nid = atm.Action[make_pair(s.top(), sb)].second;
                } else {
                    act = atm.Action[make_pair(s.top(), symbol(SYMBOL_TERMINAL, "eof"))].first;
                    nid = atm.Action[make_pair(s.top(), symbol(SYMBOL_TERMINAL, "eof"))].second;
                    skip = true;
                }

                if(act == 's') {    //   shift
                    // LOG(INFO)<<"s"<<" "<<sb.raw<<" "<<endl;
                    s.push(nid);
                    line.push_back(sb);

                    if(sb.raw == "else") {
                        code.push_back("wait"); // true jump of last code block.
                        true_list[true_list.size() - 1].push_back(code.size() - 1);

                        code[false_list.top()] = "jne " + to_string(code.size()); // point to next code block
                        false_list.pop();                    
                    } else if(sb.raw == "elif") {
                        code.push_back("wait"); // true jump of last code block.
                        true_list[true_list.size() - 1].push_back(code.size() - 1);

                        int back_num = false_list.top();
                        false_list.pop();
                        code[back_num] = "jne " + to_string(code.size()); // point to next code block
                    }
                } else if(act == 'r') {     // reduce
                    LOG(INFO)<<"r "<<nid<<"  "<<(*items)[nid - 1]<<endl;
                    int cnt = (*items)[nid - 1].body.size();

                    symbol push_smb = (*items)[nid - 1].head;
                    switch (nid) {
                    case 11: // TypeSpecifier -> __int
                    case 12: // TypeSpecifier -> __long
                    case 13: // TypeSpecifier -> __char
                    case 14: // TypeSpecifier -> __void
                        push_smb.data = line[line.size() - 1].raw;
                        break;
                    case 16: // Pointer -> __eof   Pointer中存储整数，记录指针个数。
                        push_smb.data = 0;
                        break;
                    case 15: // Pointer -> Pointer __*
                        push_smb.data = anytp_cast<int>(line[line.size()-2].data) + 1;
                        break;
                    case 10: // Array -> __eof     Array存储deque<int>，记录数组维度
                        push_smb.data = deque<int>();
                        break;
                    case 9: { // Array -> Array __[ __number __]
                        deque<int> rktmp  = anytp_cast<deque<int>>(line[line.size() - 4].data);
                        rktmp.push_back(int(anytp_cast<int>(line[line.size() - 2].data)));
                        push_smb.data = rktmp;
                        break;
                    }
                    case 8: // varDeclID -> Pointer __id Array     VarUnit 中记录了指针个数，变量名和数组维度。  
                        push_smb.data = VarUnit(anytp_cast<int>(line[line.size() - 3].data),  
                                                anytp_cast<string>(line[line.size() - 2].data),
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
                    case 20:    //  params -> __eof
                    case 19: {  // params -> paraList  初始化局部符号表。参数加入符号表。生成函数初始化代码。
                        loc_table.reset(new SymbolTable());
                        loc_table->set_prev_table(&sbtable);
                        loc_offset = 0;

                        // funDeclaration -> TypeSpecifier Pointer __id __( paraList 符号表中添加函数
                        code.push_back(format_command("mov %s , %s", 8, "rbp", "rsp"));    // 修改rbp为栈顶，每个函数的第一行语句。
                        if(anytp_cast<string>(line[line.size() - 3].data) == "main") {
                            entry_point = code.size() - 1;
                        }
                        if(nid == 19) {
                            ftable.add_func(FuncUnit(anytp_cast<string>(line[line.size() - 3].data), 
                                anytp_cast<string>(line[line.size() - 5].data),
                                anytp_cast<int>(line[line.size() - 4].data),
                                code.size() - 1,                         // 　函数入口点
                                anytp_cast<VarList>(line[line.size() - 1].data)));                        
                            int offset = 16; // skip pc and rbp in the stack, total 16 bytes
                            for(const auto& i : anytp_cast<VarList>(line[line.size() - 1].data)) {
                                if(i.type_name == "void" && i.ptr_cnt == 0) {
                                    LOG(ERROR)<<"Variable cann't be void!"<<endl;
                                    throw -1;
                                }
                                loc_table->add_to_table_loc(i, offset); 
                                if(i.ptr_cnt) {
                                    offset += 8;
                                } else if(i.type_name == "char") {
                                    offset += 1;
                                } else if(i.type_name == "int") {
                                    offset += 4;
                                } else if(i.type_name == "long") {
                                    offset += 8;
                                }
                            }
                        } else {
                            ftable.add_func(FuncUnit(anytp_cast<string>(line[line.size() - 3].data), 
                                anytp_cast<string>(line[line.size() - 5].data),
                                anytp_cast<int>(line[line.size() - 4].data),
                                code.size() - 1,                         // 　函数入口点
                                VarList() ));   
                        }
                        break;
                    }
                    case 41: // localDeclarations -> varDeclaration 
                    case 40: // localDeclarations -> localDeclarations varDeclaration
                        for(auto& i : anytp_cast<VarList>(line[line.size() - 1].data)) {
                            if(i.type_name == "void" && i.ptr_cnt == 0) {
                                LOG(ERROR)<<"Variable cann't be void!"<<endl;
                                throw -1;
                            }

                            if(i.ptr_cnt || i.type_name == "long") {
                                loc_offset -= 8 * i.arr_size;           // loc_offset为距离rbp的偏移
                                code.push_back(format_command("add %s , %d", 8, "rsp", -8 * i.arr_size));
                            } else if(i.type_name == "char") {
                                loc_offset -= 1 * i.arr_size;
                                code.push_back(format_command("add %s , %d", 1, "rsp", -1 * i.arr_size));
                            } else if(i.type_name == "int") {
                                loc_offset -= 4 * i.arr_size;
                                code.push_back(format_command("add %s , %d", 4, "rsp", -4 * i.arr_size));
                            }
                            loc_table->add_to_table_loc(i, loc_offset);
                        }
                        break;
                    case 94:  // constant -> __number          
                        code.push_back(format_command("push %d", 4, anytp_cast<int>(line[line.size() - 1].data)));
                        push_smb.data = type_info("int", 0, deque<int>()); 
                        break;
                    case 95: // constant -> __true              
                        code.push_back(format_command("push %d", 4, 1));
                        push_smb.data = type_info("int", 0, deque<int>()); 
                        break;
                    case 96: // constant -> __false             
                        code.push_back(format_command("push %d", 4, 0));
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    case 88: // immutable -> constant    继承类型
                        push_smb.data = line[line.size() - 1].data;                        
                        break;
                    case 86: // immutable -> __( expression __)    继承类型 
                        push_smb.data = line[line.size() - 2].data;       
                        break;   
                    case 84: { // mutable -> __id               Return var_info
                        var_info vfo = loc_table->get_varinfo(anytp_cast<string>(line[line.size() - 1].data));
                        if(vfo.rank.size() == 0) {  // 不是数组则直接生成代码。
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("push @ %l", vfo.type_size, vfo.addr));
                            } else {
                                code.push_back(format_command("mov %s , %s", 8, "rax", "rbp"));
                                code.push_back(format_command("add %s , %l", 8, "rax", vfo.addr));
                                code.push_back(format_command("push @ %s", vfo.type_size, "rax"));
                            }
                        }
                        push_smb.data = vfo;
                        break;
                    }
                    case 85: { // mutable -> mutable __[ expression __]         Return var_info
                        var_info vfo = anytp_cast<var_info>(line[line.size() - 4].data);
                        vfo.proc_cnt++;

                        if(vfo.ptr_cnt == vfo.rank.size()) {    // 数组维度符合才生成代码。
                            code.push_back(format_command("mov %s , %d", 4, "rax", 1));           // index is stored in rax
                            for(int i = vfo.rank.size() - 1; i >= 0; i--) {      // cal the index
                                code.push_back(format_command("pop %s", 4, "r0"));
                                code.push_back(format_command("mul %s , %d", 4, "rax", "r0"));
                            }

                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov %s , %l", 8, "rbx", vfo.addr));
                                code.push_back(format_command("add %s , %d", 4, "rbx", "rax"));
                            } else {
                                code.push_back(format_command("mov %s , %s", 8, "rbx", "rbp"));
                                code.push_back(format_command("add %s , %d", 4, "rbx", vfo.addr));
                                code.push_back(format_command("add %s , %s", 4, "rbx", "rax"));
                            }
                            code.push_back(format_command("push @ %s", vfo.type_size, "rbx"));
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
                        ParaList plst = anytp_cast<ParaList>(line[line.size() - 2].data);                        
                        if(anytp_cast<string>(line[line.size() - 4].data) == "print") {
                            if(plst.size() != 1) {
                                LOG(ERROR)<<"Cann't print more than 1 parameter!"<<endl;
                                throw -1;
                            }
                            code.push_back(format_command("pop %s", plst[0].type_size, "r4")); // clear the stack frame
                            code.push_back(format_command("print %s", plst[0].type_size, "r4")); // clear the stack frame   
                            push_smb.data = type_info("void", 0, deque<int>());
                            break;                     
                        }

                        func_info fio = ftable.get_func(anytp_cast<string>(line[line.size() - 4].data));
                        if(fio.para_lst.size() != plst.size()) {
                            LOG(ERROR)<<"Parament count not match!"<<endl;
                            throw -1;
                        }

                        for(int i = 0 ;i != fio.para_lst.size(); i++) { // parameter type check  在语法分析的过程中参数已经从右往左进栈。
                            if(!(fio.para_lst[i].type_name == plst[i].type_name && 
                                 fio.para_lst[i].ptr_cnt == plst[i].ptr_cnt)) {
                                LOG(ERROR)<<"Parameter not match!"<<endl;
                                throw -1;
                            }
                        }

                        code.push_back(format_command("call %l", 8, fio.entry_line)); // pusn ip; push rbp
                        type_info func_tp = type_info(fio.type_name, fio.ptr_cnt, deque<int>());
                        for(auto i : plst) {
                            code.push_back(format_command("pop %s", i.type_size, "r4")); // clear the stack frame
                        }
                        code.push_back(format_command("push %s", func_tp.type_size, "r0")); // pusn ip; push rbp
                        push_smb.data = func_tp;
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

                        if(anytp_cast<string>(line[line.size() - 1].data) == "-") {
                            if(tp.ptr_cnt) {
                                LOG(ERROR)<<"Wrong type argument to unary \'-\' "<<endl;
                                throw -1;
                            }
                            code.push_back(format_command("mul %s , %d", tp.type_size, "rsp", -1));
                        } else if(anytp_cast<string>(line[line.size() - 1].data) == "!") {
                            if(tp.ptr_cnt) {
                                LOG(ERROR)<<"Wrong type argument to unary \'!\' "<<endl;
                                throw -1;
                            }
                            code.push_back(format_command("not %s", tp.type_size, "rsp"));
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

                        code.push_back(format_command("pop %s", tp.type_size, "rbx"));
                        code.push_back(format_command("pop %s", tp.type_size, "rax"));
                        if(opt == "*") {
                            code.push_back(format_command("mul %s , %s", tp.type_size, "rax", "rbx"));
                        } else if(opt == "/") {
                            code.push_back(format_command("div %s , %s", tp.type_size, "rax", "rbx"));
                        } else if(opt == "%") {
                            code.push_back(format_command("mod %s , %s", tp.type_size, "rax", "rbx"));
                        }
                        code.push_back(format_command("push %s", tp.type_size, "rax"));

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

                        code.push_back(format_command("pop %s", tpr.type_size, "rbx"));
                        code.push_back(format_command("pop %s", tpl.type_size, "rax"));
                        code.push_back("cmp rax , rbx");
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
                        code.push_back(cmd + to_string(code.size() + 3));
                        code.push_back(format_command("push %d", 4, 0));  // not meet the condition
                        code.push_back("jmp " + to_string(code.size() + 2));
                        code.push_back(format_command("push %d", 4, 1));  // meet the condition
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    }
                    case 57: { // andExpression -> andExpression __&& relExpression
                        type_info tpl = anytp_cast<type_info>(line[line.size() - 3].data);   
                        type_info tpr = anytp_cast<type_info>(line[line.size() - 1].data);
                        code.push_back(format_command("pop %s", tpr.type_size, "rbx"));
                        code.push_back(format_command("pop %s", tpl.type_size, "rax"));
                        code.push_back(format_command("and %s , %s",tpl.type_size, "rax", "rbx"));
                        code.push_back(format_command("push %s", 4, "lf"));
                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    }
                    case 58: // andExpression -> relExpression
                        push_smb.data = line[line.size() -1].data;
                        break;
                    case 55: {   // simpleExpression -> simpleExpression __|| andExpression
                        type_info tpl = anytp_cast<type_info>(line[line.size() - 3].data);   
                        type_info tpr = anytp_cast<type_info>(line[line.size() - 1].data);
                        code.push_back(format_command("pop %s", tpr.type_size, "rbx"));
                        code.push_back(format_command("pop %s", tpl.type_size, "rax"));
                        code.push_back(format_command("or %s , %s", tpl.type_size, "rax", "rbx"));
                        code.push_back(format_command("push %s", 4, "lf"));
                        if(line.size() - 5 >= 0 && line[line.size() - 5].raw == "if") {     // IfStatement -> __if __( simpleExpression __) Statement ElifStat
                            process_if();
                        } else if(line.size() - 5 >= 0 && line[line.size() - 5].raw == "elif") {
                            process_elif_st();
                        }

                        push_smb.data = type_info("int", 0, deque<int>());
                        break;
                    }
                    case 54:  // expression -> simpleExpression 
                        push_smb.data = line[line.size() -1].data;
                        break;
                    case 56: {    // simpleExpression -> andExpression
                        if(line.size() - 3 >= 0 && line[line.size() - 3].raw == "if") {
                            process_if();
                        } else if(line.size() - 3 >= 0 && line[line.size() - 3].raw == "elif") {
                            process_elif_st();
                        }
                        push_smb.data = line[line.size() - 1].data;
                        break;
                    }
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
                            LOG(ERROR)<<"Type different! left_type: "<<tpl.to_string()<<" right_type:"<<tpr.to_string()<<endl;
                            throw -1;
                        } else {
                            code.push_back(format_command("pop %s", tpr.type_size, "rbx"));
                            code.push_back(format_command("pop %s", tpl.type_size, "rax"));
                            if(opt == "+") {
                                code.push_back(format_command("add %s , %s", tpl.type_size, "rax", "rbx"));
                            } else {
                                code.push_back(format_command("mul %s , %d", tpr.type_size, "rbx", -1));                                
                                code.push_back(format_command("add %s , %s", tpl.type_size, "rax", "rbx"));                               
                            }
                            code.push_back(format_command("push %s", tpl.type_size, "rax"));
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
                        code.push_back(format_command("add @ %s , %d", tp.type_size, "rsp", 1));
                        push_smb.data = tp;
                        break;
                    }
                    case 53: { // expression -> mutable __--
                        type_info tp = anytp_cast<type_info>(line[line.size() -2].data);
                        code.push_back(format_command("add @ %s , %d", tp.type_size, "rsp", -1));
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

                        code.push_back(format_command("pop %s", tp.type_size, "rbx"));
                        code.push_back(format_command("pop %s", vfo.type_size, "rax"));
                        if(vfo.type_size < tp.type_size) {
                            LOG(ERROR)<<"type not match!"<<endl;
                            throw -1;
                        } else if(vfo.rank.size()) {
                            LOG(ERROR)<<"Array lacks index!"<<endl;
                            throw -1;                        
                        }
                        

                        if(vfo.scope == SCOPE_LOCAL) {
                            code.push_back(format_command("mov %s , %s", 8, "rcx", "rbp"));
                            code.push_back(format_command("add %s , %l", 8, "rcx", vfo.addr));
                        }
                        if(opt == "=") {
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov @ %l , %s", vfo.type_size, vfo.addr, "rbx"));
                            } else {
                                code.push_back(format_command("mov @ %s , %s", vfo.type_size, "rcx", "rbx"));
                            }
                        } else if(opt == "+=") {
                            code.push_back(format_command("add %s , %s", vfo.type_size, "rax", "rbx"));
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov @ %l , %s", vfo.type_size, vfo.addr, "rax"));
                            } else {
                                code.push_back(format_command("mov @ %s , %s", vfo.type_size, "rcx", "rax"));
                            }
                        } else if(opt == "-=") {
                            code.push_back(format_command("mul %s , %d", tp.type_size, "rbx", -1));
                            code.push_back(format_command("add %s , %s", vfo.type_size, "rax", "rbx"));
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov @ %l , %s", vfo.type_size, vfo.addr, "rax"));
                            } else {
                                code.push_back(format_command("mov @ %s , %s", vfo.type_size, "rcx", "rax"));
                            }
                        } else if(opt == "/=") {
                            code.push_back(format_command("div %s , %s", vfo.type_size, "rax", "rbx"));
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov @ %l , %s", vfo.type_size, vfo.addr, "rax"));
                            } else {
                                code.push_back(format_command("mov @ %s , %s", vfo.type_size, "rcx", "rax"));
                            }
                        } else if(opt == "*=") {
                            code.push_back(format_command("mul %s , %s", vfo.type_size, "rax", "rbx"));
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov @ %l , %s", vfo.type_size, vfo.addr, "rax"));
                            } else {
                                code.push_back(format_command("mov @ %s , %s", vfo.type_size, "rcx", "rax"));
                            }
                        } else if(opt == "%=") {
                            code.push_back(format_command("mod %s , %s", vfo.type_size, "rax", "rbx"));
                            if(vfo.scope == SCOPE_GLOBAL) {
                                code.push_back(format_command("mov @ %l , %s", vfo.type_size, vfo.addr, "rax"));
                            } else {
                                code.push_back(format_command("mov @ %s , %s", vfo.type_size, "rcx", "rax"));
                            }
                        }

                        push_smb.data = type_info(vfo.type_name, vfo.ptr_cnt, deque<int>());
                        // 类型相等赋值
                        break;
                    }
                    case 32:    // returnStatement -> __return __;
                        code.push_back("ret");
                        break;
                    case 33:    // returnStatement -> __return expression __;
                        code.push_back(format_command("pop %s", anytp_cast<type_info>(line[line.size() - 2].data).type_size, "r0"));
                        code.push_back("ret");                        
                        break;
                    case 35: {  // ElifStat -> __eof
                        for(auto i : true_list[true_list.size() - 1]) {
                            code[i] = "jmp " + to_string(code.size());  // backpatch
                        }
                        true_list.pop_back();

                        int back_num = false_list.top();
                        false_list.pop();
                        code[back_num] = "jne " + to_string(code.size()); // point to next code block
                        break;
                    }
                    case 37: { // ElifStat -> __else Statement
                        for(auto i : true_list[true_list.size() - 1]) {
                            code[i] = "jmp " + to_string(code.size());  // backpatch
                        }
                        true_list.pop_back();
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
        
        ofs<<entry_point<<endl;
        for(auto i : code) {
            ofs<<i<<endl;
        }
        ofs.close();
        return 0;
    }

    LRParser::LRParser() {
        items.reset(new vector<lritem>());
    }

    void LRParser::process_if() {
        code.push_back(format_command("pop %s", 4, "rax"));
        code.push_back(format_command("cmp %s , %d", 4, "rax", 1));
        code.push_back("wait");  // wait backpatching
        false_list.push(code.size() - 1);
        true_list.push_back({});
    }

    void LRParser::process_elif_st() {
        code.push_back(format_command("pop %s", 4, "rax"));
        code.push_back(format_command("cmp %s , %d", 4, "rax", 1));
        code.push_back("wait");  // wait backpatching
        false_list.push(code.size() - 1);
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

    string LRParser::format_command(string pattern, int sz, ...) {
        char cmd_suffix = sz == 1 ? 'c' : sz == 4 ? 'i' : 'l';
        string ret = "";
        va_list vp;
        va_start(vp, sz);
        bool fmt = false;
        bool fst_emp = true;
        for(auto i : pattern) {
            if(i == '%') {
                fmt = true;
                continue;
            }
            if(fmt) {
                string val = "";
                if(i == 's') {
                    val = va_arg(vp, char*);
                } else if(i == 'd') {
                    val = to_string(va_arg(vp, int));
                } else if(i == 'l') {
                    val = to_string(va_arg(vp, long long));
                }
                ret += val;
                fmt = false;
            } else {
                if(i == ' ' && fst_emp) {
                    fst_emp = false;
                    ret += '_';
                    ret += cmd_suffix;
                }
                ret += i;
            }
        }
        va_end(vp);
        LOG(INFO)<<ret<<endl;
        return ret;
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