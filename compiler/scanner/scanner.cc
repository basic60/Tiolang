#include<string>
#include<iostream>
#include<cstdio>
#include"scanner.h"
#include"token.h"
#include"log/log.h"
using namespace logger;
using namespace std;
namespace tio
{
    #define MEET_SPACE(c) ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t')
    #define MEET_SPLITER(c) ((c) == '(' || (c) == ')' || (c) == '[' || (c) == ']' || (c) == '{' \
                            || (c) == '}'  || (c) == '\"' || (c) == '\'' || (c) == ';')
    #define MEET_OPERATOR(c) ((c) == '+' || (c) == '-' || (c) == '*' || (c) == '/' || (c) == '.' || (c) == ','\
                            || (c) == '>' || (c) == '<' || (c) == '!' || (c) == '=' || (c) == '&' || (c) == '|' || (c) == '%')
    Scanner::Scanner() {
        keyword_set = {"int", "char", "void", "long", "return", "if", "while", "continue", "break",
                       "true", "false", "else", "elif"};
    }

    void Scanner::scan(FILE* fp) {
        dfa_state state = dfa_state::start;
        string s;
        char c;
        int line = 1;bool in_comment = false;
        while (~(c = fgetc(fp))) {
            if(c == '\n') {
                line++;
                in_comment = false;
            }
            if(in_comment || c == '#') {
                in_comment = true;
                continue;
            }

            switch (state) {
            case dfa_state::start:
                if(MEET_SPACE(c)) {
                    continue;
                } else if(isdigit(c)) {
                    s += c;
                    state = dfa_state::number;
                } else if(MEET_OPERATOR(c)) {
                    s += c;
                    state = dfa_state::opt;
                } else if(MEET_SPLITER(c)) {
                    s += c;
                    state = dfa_state::spt;
                } else if(isalpha(c) || c == '_') {
                    s += c;
                    state = dfa_state::id;
                } else {
                    LOG(ERROR)<<"Lexical analysis failed! Invalid char: "<<c<<" at line "<<line<<"."<<endl;
                    throw -1;
                }
                break;
            case dfa_state::number:
                if(isdigit(c)) {
                    s += c;
                } else if(MEET_SPACE(c)){
                    tokens.push({TOKEN_NUMBER, s, line});
                    s = "";
                    state = dfa_state::start;
                } else if(MEET_OPERATOR(c)) {
                    tokens.push({TOKEN_NUMBER, s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::opt;
                } else if(MEET_SPLITER(c)) {
                    tokens.push({TOKEN_NUMBER, s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::spt;
                } else {
                    LOG(ERROR)<<"Lexical analysis failed! Invalid char: "<<c<<" at line "<<line<<"."<<endl;
                    throw -1;
                }
                break;
            case dfa_state::opt:
                if(c == '=' && (s[0] == '=' || s[0] == '>' || s[0] == '<' || s[0] == '!' || s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/') ||
                 c == '<' && s[0] == '<' || c == '>' && s[0] == '>' ||
                 s[0] == '&' && c == '&' || s[0] == '|' && c == '|' ||
                 s[0] == '+' && c == '+' || s[0] == '-' && c == '-') {
                    s += c;
                    tokens.push({TOKEN_OPERATOR, s, line});
                    s = "";
                    state = dfa_state::start;
                } else if(MEET_SPACE(c)) {
                    tokens.push({TOKEN_OPERATOR, s, line});
                    s = "";
                    state = dfa_state::start;
                } else if(isdigit(c)) {
                    tokens.push({TOKEN_OPERATOR, s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::number;
                } else if(isalpha(c) || c == '_') {
                    tokens.push({TOKEN_OPERATOR, s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::id;
                } else if(MEET_SPLITER(c)) {
                    tokens.push({TOKEN_OPERATOR, s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::spt;
                } else if(MEET_OPERATOR(c)) {
                    tokens.push({TOKEN_OPERATOR, s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::opt;
                } else {
                    cout<<s<<endl;
                    LOG(ERROR)<<"Lexical analysis failed! Invalid char: "<<c<<" at line "<<line<<"."<<endl;
                    throw -1;
                }
                break;
            case dfa_state::spt:
                tokens.push({TOKEN_SPLITER, s, line});
                s = "";
                if(MEET_SPACE(c)) {
                    state =dfa_state::start;
                } else if(MEET_OPERATOR(c)) {
                    s += c;
                    state = dfa_state::opt;
                } else if(MEET_SPLITER(c)) {
                    s += c;
                    state = dfa_state::spt;
                } else if(isdigit(c)) {
                    s += c;
                    state = dfa_state::number;
                } else if(isalpha(c) || c == '_') {
                    s += c;
                    state = dfa_state::id;
                } else {
                    LOG(ERROR)<<"Lexical analysis failed! Invalid char: "<<c<<" at line "<<line<<"."<<endl;
                    throw -1;
                }
                break;
            case dfa_state::id:
                if(isalpha(c) || isdigit(c) || c == '_') {
                    s += c;
                } else if(MEET_SPACE(c)) {
                    tokens.push({keyword_set.count(s) ? TOKEN_KEYWORD : TOKEN_ID , s, line});
                    s = "";
                    state = dfa_state::start;
                } else if(MEET_OPERATOR(c)) {
                    tokens.push({keyword_set.count(s) ? TOKEN_KEYWORD : TOKEN_ID , s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::opt;
                } else if(MEET_SPLITER(c)) {
                    tokens.push({keyword_set.count(s) ? TOKEN_KEYWORD : TOKEN_ID , s, line});
                    s.clear();
                    s.push_back(c);
                    state = dfa_state::spt;
                } else {
                    LOG(ERROR)<<"Lexical analysis failed! Invalid char: "<<c<<" at line "<<line<<"."<<endl;
                    throw -1;
                }
                break;
            }
        }

        if(s != "") {
            switch (state) {
            case dfa_state::id:
                tokens.push({keyword_set.count(s) ? TOKEN_KEYWORD : TOKEN_ID , s, line});
                break;
            case dfa_state::number:
                tokens.push({TOKEN_NUMBER, s, line});
                break;
            case dfa_state::opt:
                tokens.push({TOKEN_OPERATOR, s, line});
                break;
            case dfa_state::spt:
                tokens.push({TOKEN_SPLITER, s, line});
                break;
            }
        }

        tokens.push({TOKEN_KEYWORD, "eof", line});
    }



    token Scanner::get_next_token() {
        if(!tokens.size()) {
            LOG(ERROR)<<"No more tokens";
            throw(-1);
        }
        token tmp = tokens.front();
        tokens.pop();
        return tmp;
    }
}