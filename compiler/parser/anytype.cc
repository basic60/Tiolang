#include"anytype.h"
#include<memory>
#include<iostream>
#include<exception>
#include"log/log.h"
using namespace std;
namespace tio
{
    const char* AnyCastException::what() const noexcept {
        if(exp_type == CAST_EXP_EMPTY) {
            return "Any type is empty. Cann't cast.";
        } else if(exp_type == CAST_EXP_DFTP) {
            return "Type different. Cann't convert!";
        }
        return "";
    }

    AnyCastException::AnyCastException(int tp): exp_type(tp) {}

    bool anytp::empty() {
        return !(bool)ptr;
    }
}