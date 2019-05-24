#ifndef __TIO_SCANNER
#define __TIO_SCANNER
#include<string>
#include<queue>
#include<set>
#include"token.h"
namespace tio
{
    class Scanner {
    private:
        std::queue<token> tokens;
        enum class dfa_state {
            start,
            number,
            opt,
            spt,
            id
        };

        std::set<std::string> keyword_set;

    public:
        void scan(FILE* fp);
        inline size_t tkcount() { return tokens.size(); }
        token get_next_token();
        Scanner();
    };
}
#endif