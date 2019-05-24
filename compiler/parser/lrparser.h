#ifndef __TIO_LRPARSER
#define __TIO_LRPARSER
#include<string>
#include<memory>
#include<vector>
#include"../scanner/token.h"
#include"symbol.h"
namespace tio
{
    class lr_parser {
    private:
        std::vector<lritem> items;

    public:
        int load_grammer(const char* fpath);
        int generate_code(std::shared_ptr<Scanner> sc, std::string fpath);
    };
}
#endif