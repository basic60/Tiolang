#ifndef STR_HELPER_H_
#define STR_HELPER_H_
#include<string>
#include<vector>
namespace str
{
    std::string strip(const std::string& s);
    std::vector<std::string> split(const std::string& str, char target);
    std::vector<std::string> split(const std::string& str, char target, int num);
} 
#endif