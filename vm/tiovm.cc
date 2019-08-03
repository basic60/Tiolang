#include<iostream>
#include"reg.h"
#include"mmu.h"
#include"interpreter.h"
#include"str_helper/str_helper.h"
using namespace std;
using namespace tiovm;
int main() {
    try {
        Interpreter intc;
        intc.execute("test.log");
    } catch(int& x) {
        cout<<"Exit code: "<<x<<endl;
        return x;
    }
    cout<<"fin"<<endl;
    return 0;
} 