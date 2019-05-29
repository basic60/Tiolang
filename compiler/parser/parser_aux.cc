#include"parser_aux.h"
#include<string>
#include<iostream>
using namespace std;
namespace tio
{
    VarUnit::VarUnit(int pcnt, std::string vname, int asize):ptr_cnt(pcnt), var_name(vname), arr_size(asize) { }

    VarUnit::VarUnit():ptr_cnt(0), arr_size(1){}
}