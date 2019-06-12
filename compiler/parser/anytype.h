#ifndef __TIO_ANYTYPE
#define __TIO_ANYTYPE
#include<memory>
#include<iostream>
#include<exception>
#include"log/log.h"
namespace tio
{
    #define CAST_EXP_EMPTY 1
    #define CAST_EXP_DFTP 2
    class AnyCastException : public std::exception {
    public:
        int exp_type;
        AnyCastException(int tp);
        const char* what() const noexcept;
    };

    class anytp {
    private:
        struct placeholder {
            virtual const std::type_info& type() const = 0;
        };

        template<typename T>
        struct real_data : public placeholder {
            T val;

            const std::type_info& type() const {
                return typeid(T);
            }

            real_data(const T& x):val(x) {}
        };
        std::shared_ptr<placeholder> ptr;
    public:
        template<typename T>
        anytp& operator=(const T& other);

        template<typename T>
        anytp& operator=(const T*& other);

        bool empty();

        template<typename T>
        friend T anytp_cast(anytp& atp);
    };

    template<typename T>
    anytp& anytp::operator=(const T& other) {
        this->ptr.reset(new real_data<T>(other));
        return *this;
    }

    template<typename T>
    T anytp_cast(anytp& atp)  {
        if(atp.empty()) {
            LOG(ERROR)<<"Any type is empty. Cann't cast."<<std::endl;
            throw AnyCastException(CAST_EXP_EMPTY);
        }

        try {
            return static_cast<anytp::real_data<T>*>(atp.ptr.get())->val;
        } catch(...) {
            LOG(ERROR)<<"Type different. Cann't convert! "<<std::endl;
            throw AnyCastException(CAST_EXP_DFTP);
        }
    }
}
#endif