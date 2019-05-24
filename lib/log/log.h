#ifndef __TIO_LOG
#define __TIO_LOG
#include<iostream>
#include<string>
namespace tio
{
    #define LOG(TP) GlobalLogger::get_output_stream(#TP) <<"["<<__FILE__<<":"<<__LINE__<<"] "<<#TP<<": "  

    class GlobalLogger {
    public:
        static std::ostream& get_output_stream(const char* para);
    };
}
#endif