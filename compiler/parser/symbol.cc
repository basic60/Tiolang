#include<string>
#include<iostream>
#include<vector>
#include"symbol.h"
using namespace std;
namespace tio
{
    symbol::symbol(int tp, string val):stype(tp), raw(val) {}
    
    symbol::symbol() {
        stype = 0;
    }

    bool symbol::operator==(const symbol& other) const {
        return this->stype == other.stype && this->raw == other.raw;
    }

    bool symbol::operator!=(const symbol& other) const {
        return !(*this == other);
    }

    bool symbol::operator<(const symbol& other) const {
        return raw < other.raw;
    }

    lritem::lritem() {
        pos = 0;
        id = 0;
    }

    bool lritem::operator==(const lritem& other) const {
        if(this->head == other.head && this->pos == other.pos) {
            for(size_t i = 0; i != this->body.size(); i++) {
                if(this->body[i] != other.body[i]) {
                    return false;
                }
            }

            if(this->ahsym.size() != other.ahsym.size()) {
                return false;
            } else {
                for(auto j : this->ahsym) {
                    if(!other.ahsym.count(j)) {
                        return false;
                    }
                }
            }
            return true;
        }
        return false;
    }

    ostream& operator<<(std::ostream & os, const lritem& s) {
        os<<s.head.raw<<" -> ";
        for(auto i : s.body) {
            cout<<i.raw<<" ";
        }
        os<<" <"<<s.pos<<"> ";
        for(auto i : s.ahsym) {
            os<<i.raw<<" ";
        }
        return os;
    }
}