#include"log.h"
#include<iostream>
#include<cstring>
using namespace std;
namespace logger
{
    ostream& GlobalLogger::get_output_stream(const char* para) {
        if(!strcmp(para,"ERROR")) {
            return cerr;
        } else if(!strcmp(para, "INFO")) {
            return cout;
        } else {
            LOG(ERROR)<<"Cann't find stream "<<para<<endl;
            throw -1;
        }
    }
}
