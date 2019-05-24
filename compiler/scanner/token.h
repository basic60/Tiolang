#ifndef __TIO_TOKEN
#define __TIO_TOKEN
#include<string>
namespace tio
{
    #define TOKEN_ID 1
    #define TOKEN_KEYWORD 2
    #define TOKEN_SPLITER 3
    #define TOKEN_NUMBER 4
    #define TOKEN_OPERATOR 5

    struct token {
        int token_type;
        std::string raw_str;
        int line_num;
        token(int tp, const std::string& rst,int line_number);
    };
} 
#endif