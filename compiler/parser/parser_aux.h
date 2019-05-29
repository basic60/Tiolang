#ifndef __TIO_PARSERAUX
#define __TIO_PARSERAUX
#include<string>
#include<vector>
namespace tio
{
    struct VarUnit {
        int ptr_cnt;
        std::string var_name;
        int arr_size;
        VarUnit(int pcnt, std::string vname, int asize);
        VarUnit();
    };

    struct VarList {
        std::vector<VarUnit> vlist;
    }; 
    
}
#endif