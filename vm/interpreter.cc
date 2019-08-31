#include"interpreter.h"
#include"str_helper/str_helper.h"
#include<iostream>
#include<fstream>
#include<string>
#include<queue>
#include"log/log.h"
#include"reg.h"
using namespace std;
using namespace logger;
using namespace str;
namespace tiovm
{
    void Interpreter::pre_process(string fname) {
        ifstream ifs(fname);
        string line;
        bool fst_line = true;
        while (getline(ifs, line)) {
            if(line == "") {
                continue;
            }
            if(fst_line) {
                entry_point = stol(line);
                fst_line = false;
                continue;
            }

            queue<string> q;
            bool laddr = false;
            bool raddr = false;
            bool swt = true;
            int op_cnt = 0;
            for(auto& i : split(line, ' ')) {
                if(i == "@") {
                    swt ? laddr = true : raddr = true;
                } else if (i == ",") {
                    swt = false;
                } else {
                    q.push(move(i));
                    op_cnt++;
                }
            }

            Instruction ins;
            ins.raw_command = line;
            ins.op_cnt = op_cnt;
            ins.laddr = laddr;
            ins.raddr = raddr;
            ins.cmd = Command::parse_cmd(q.front());q.pop();
            if(op_cnt >= 2) {
                ins.opl = Operand::parse_operand(q.front());q.pop();
            }
            if(op_cnt == 3) {
                ins.opr = Operand::parse_operand(q.front());q.pop();
            }
            instructions.push_back(move(ins));
        }
    }

    int64 Interpreter::get_operand_val(const Operand& op, bool addr_flag) {
        int64 val = 0;
        if(op.type == Operand::opd_type::number) {
            val = addr_flag ? mmu.get64(stol(op.raw_val, 0, 0)) : stol(op.raw_val, 0, 0);
        } else {
            val = addr_flag ? mmu.get64(regs[op.raw_val].get64()) : regs[op.raw_val].get64();
        }
    }

    void Interpreter::execute(string fname) {
        pre_process(fname);
        uint64 pc = entry_point;                      // program counter
        if(instructions.size() == 0) {
            return;
        }
        while (true) {
            if(pc == instructions.size()) {
                break;
            } else if(pc > instructions.size()) {
                LOG(ERROR)<<"Error command address: "<<pc<<endl;
                throw -1;
            }
            const Instruction& ins = instructions[pc];
            const Command& cmd = ins.cmd;
            const int& op_cnt = ins.op_cnt;
            const Operand& opl = ins.opl;
            const Operand& opr = ins.opr;
            const bool& laddr = ins.laddr;
            const bool& raddr = ins.raddr;
            LOG(INFO)<<"[Execute] "<<ins.raw_command<<endl;

            if(cmd.name == "mov" && op_cnt == 3) {
                uint64 target_addr = 0;
                int64 val = get_operand_val(opr, raddr);
                if(opl.type == Operand::opd_type::number) {
                    if(!laddr) {
                        LOG(ERROR)<<"Left operand isn't a valid address!"<<endl;
                        throw -1;
                    }
                    target_addr = stol(opl.raw_val, 0, 0);
                    mmu.set_by_size(target_addr, val, cmd.vsize);
                } else {
                    Register& regl = regs[opl.raw_val];
                    if(!laddr) {
                        regl.set_by_size(val, cmd.vsize);
                    } else {
                        mmu.set_by_size(regl.get64(), val, cmd.vsize);
                    }
                }
                ++pc;
            } else if(cmd.name == "push" && op_cnt == 2) {
                int64 val = get_operand_val(opl, laddr);
                regs["rsp"].set64(regs["rsp"].get64() - cmd.vsize);
                mmu.set_by_size(regs["rsp"].get64(), val, cmd.vsize);
                ++pc;
            } else if(cmd.name == "pop" && op_cnt == 2) {
                int64 val = mmu.get64(regs["rsp"].get64());
                if(opl.type == Operand::opd_type::number) {
                    if(!laddr) {
                        LOG(ERROR)<<"Cann't pop value to a number!"<<endl;
                        throw -1;
                    } else {
                        mmu.set_by_size(stol(opl.raw_val, 0, 0), val, cmd.vsize);
                    }
                } else {
                    if(!laddr) {
                        regs[opl.raw_val].set_by_size(val, cmd.vsize);
                    } else {
                        mmu.set_by_size(regs[opl.raw_val].get64(), val, cmd.vsize);
                    }
                }
                regs["rsp"].set64(regs["rsp"].get64() + cmd.vsize);
                ++pc;
            } else if(cmd.name == "print" && op_cnt == 2) {
                if(opl.type == Operand::opd_type::number) {  // Print the data in the memory.
                    if(laddr) {
                        cout<<"@"<<opl.raw_val<<": "<<mmu.get_by_size(stol(opl.raw_val, 0, 0), cmd.vsize)<<endl;
                    } else {
                        cout<<opl.raw_val<<": "<<opl.raw_val<<endl;
                    }
                } else {
                    if(laddr) {
                        cout<<"@"<<opl.raw_val<<": "<<mmu.get_by_size(regs[opl.raw_val].get64(), cmd.vsize)<<endl;
                    } else {
                        cout<<opl.raw_val<<": "<<regs[opl.raw_val].get_by_size(cmd.vsize)<<endl;
                    }
                }
                pc++;
            } else if(cmd.name == "cmp" && op_cnt == 3) { 
                int64 vl = get_operand_val(opl, laddr), vr = get_operand_val(opr, raddr);
                int64 res = vl - vr;
                if(res < 0) {
                    regs["sf"].set64(-1);
                } else if(res == 0) {
                    regs["sf"].set64(0);
                } else if(res > 0) {
                    regs["sf"].set64(1);
                }
                pc++;
            } else if((cmd.name == "jmp" || cmd.name == "jne" || cmd.name == "je"
                    || cmd.name == "jg" || cmd.name == "jge" || cmd.name == "jl"
                    || cmd.name == "jle") && op_cnt == 2) {
                if(laddr) {
                    LOG(ERROR)<<"Error command format!"<<endl;
                    throw -1;
                }
                if(jmp_judge[cmd.name](regs["sf"].get64())) {
                    pc = get_operand_val(opl, laddr);
                } else {
                    pc++;
                }
            } else if((cmd.name == "add" || cmd.name == "mul" || cmd.name == "div")
                     && op_cnt == 3) {
                if(laddr || opl.type != Operand::opd_type::reg) {
                    LOG(ERROR)<<"Left operand of command: "<<ins.raw_command<<" must be a register!"<<endl;
                    throw -1;
                }
                int64 source = get_operand_val(opl, laddr);
                int64 val = get_operand_val(opr, raddr);
                if(cmd.name == "add") {
                    regs[opl.raw_val].set64(source + val);
                } else if(cmd.name == "mul") {
                    regs[opl.raw_val].set64(source * val);
                } else if(cmd.name == "div") {
                    if(val == 0) {
                        LOG(ERROR)<<"Cann't divide 0."<<endl;
                        throw -1;
                    }
                    regs[opl.raw_val].set64(source / val);
                }
                pc++;
            } else if(cmd.name == "call" && op_cnt == 2) {
                if(laddr) {
                    LOG(ERROR)<<"Command format error!"<<endl;
                    throw -1;
                }
                uint64 target_addr = get_operand_val(opl, laddr);
                regs["rsp"].set64(regs["rsp"].get64() - 8);
                mmu.set64(regs["rsp"].get64(), regs["rbp"].get64());         // push rbp
                regs["rsp"].set64(regs["rsp"].get64() - 8);
                mmu.set64(regs["rsp"].get64(), pc + 1);         // push pc
                pc = target_addr;
            } else if(cmd.name == "ret" && op_cnt == 1) {
                pc = mmu.get64(regs["rbp"].get64());
                regs["rsp"].set64(regs["rbp"].get64() + 16);      // skip the rbp and pc
                regs["rbp"].set64(mmu.get64(regs["rbp"].get64() + 8));
            } else if(cmd.name == "not") {
                if(laddr) {
                    LOG(ERROR)<<"Command format error!"<<endl;
                    throw -1;
                }
                uint64 val = get_operand_val(opl, laddr);
                if(val) {

                } else {
                    
                }
            } else {
                LOG(ERROR)<<"Unknown command: " + ins.raw_command << ". Please check the foramt." << endl;
                throw -1;
            }
        }
    }

    Interpreter::Interpreter() {
        for(auto name : Register::usable_regs) {
            regs[name];
        }
        regs["rsp"].set64(4294967296); // 4GB

        jmp_judge["je"] = [](int x){return x == 0;};
        jmp_judge["jne"] = [](int x){return x != 0;};
        jmp_judge["jg"] = [](int x){return x > 0;};
        jmp_judge["jge"] = [](int x){return x >= 0;};
        jmp_judge["jl"] = [](int x){return x < 0;};
        jmp_judge["jle"] = [](int x){return x <= 0;};
        jmp_judge["jmp"] = [](int x){return true;};
    }

    Command Command::parse_cmd(const string& cmd) {
        Command ret;
        if((cmd[cmd.size() -1 ] == 'c' || cmd[cmd.size() -1 ] == 'i' || cmd[cmd.size() -1 ] == 'l')
            && cmd.size() >= 2 && cmd[cmd.size() - 2] == '_' ) {
            ret.vsize = cmd[cmd.size() -1 ] == 'c' ? 1 : cmd[cmd.size() -1 ] == 'i' ? 4 : 8;
            ret.name = cmd.substr(0, cmd.size() - 2);
        } else {
            ret.name = cmd;
        }
        return ret;
    }

    Operand Operand::parse_operand(const std::string& opd) {
        Operand ret;
        if(Register::valid_reg(opd)) {
            ret.type = opd_type::reg;
            ret.raw_val = opd;
        } else {
            try {
                stol(opd, 0, 0);
            } catch(...) {
                LOG(ERROR)<<"Not invalid number: "<<opd<<endl;
                throw -1;
            }
            ret.type = opd_type::number;
            ret.raw_val = opd;
        }
        return ret;
    }

}