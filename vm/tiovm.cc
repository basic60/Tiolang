#include<iostream>
#include"reg.h"
#include"mmu.h"
#include"interpreter.h"
using namespace std;
using namespace tiovm;
int main() {
    try {
        MMU m;
        m.set32(VM_PAGE_SIZE - 1, 0x55aa55dd8a8c9c1c);
        cout<<"set finished!"<<endl;
        cout<<"get: 0x"<<hex<<m.get32(VM_PAGE_SIZE - 1)<<endl;
        
        Interpreter intc;
        intc.execute("test.log");
    } catch(int& x) {
        cout<<"Exit code: "<<x<<endl;
        return x;
    }
    return 0;
} 