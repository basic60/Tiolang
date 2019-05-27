#include<string>
#include"token.h"
using namespace std;
namespace tio
{
    token::token(int tp, const string& rst, int line_number) {
        this->token_type = tp;
        this->raw_str = rst;
        this->line_num = line_number;
    }

    token::token(){}
}