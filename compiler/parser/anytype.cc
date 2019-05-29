#include"anytype.h"
#include<memory>
#include<iostream>
#include"log/log.h"
using namespace std;
namespace tio
{
    bool anytp::empty() {
        return !(bool)ptr;
    }
}