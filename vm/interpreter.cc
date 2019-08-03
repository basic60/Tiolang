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
        while (getline(ifs, line)) {
            if(line == "") {
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
                    cout<<swt<<" "<<laddr<<endl;
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

    void Interpreter::execute(string fname) {
        pre_process(fname);
        uint64 pc = 0;                      // program counter
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
                uint64 val = 0;
                if(opl.type == Operand::opd_type::number) {
                    if(!laddr) {
                        LOG(ERROR)<<"Left operand isn't a valid address!"<<endl;
                        throw -1;
                    }
                    target_addr = stol(opl.raw_val, 0, 0);
                    
                    if(opr.type == Operand::opd_type::number) {
                        val = stol(opr.raw_val, 0, 0);
                        if(raddr) {
                            val = mmu.get64(val);
                        }
                    } else {
                        val = regs[opr.raw_val].get64();
                    }
                    mmu.set_by_size(target_addr, val, cmd.vsize);
                } else {
                    Register& regl = regs[opl.raw_val];
                    if(!laddr) {
                        if(opr.type == Operand::opd_type::reg) {
                            if(!raddr) {
                                regl.set_by_size(regs[opr.raw_val].get64(), cmd.vsize);
                            } else {
                                regl.set_by_size(mmu.get64(regs[opr.raw_val].get64()), cmd.vsize);
                            }
                        } else {
                            if(!raddr) {
                                regl.set_by_size(stol(opr.raw_val, 0, 0), cmd.vsize);
                            } else {
                                regl.set_by_size(mmu.get64(stol(opr.raw_val, 0, 0)), cmd.vsize);
                            }
                        }
                    } else {
                        if(opr.type == Operand::opd_type::reg) {
                            if(!raddr) {
                                mmu.set_by_size(regl.get64(), regs[opr.raw_val].get64(), cmd.vsize);
                            } else {
                                mmu.set_by_size(regl.get64(), mmu.get64(regs[opr.raw_val].get64()), cmd.vsize);
                            }
                        } else {
                            if(!raddr) {
                                mmu.set_by_size(regl.get64(), stol(opr.raw_val, 0, 0), cmd.vsize);
                            } else {
                                mmu.set_by_size(regl.get64(), mmu.get64(stol(opr.raw_val, 0, 0)), cmd.vsize);
                            }
                        }
                    }
                }
                ++pc;
            } else if(cmd.name == "push" && op_cnt == 2) {
                uint64 val = 0;
                if(opl.type == Operand::opd_type::number) {
                    val = laddr ? mmu.get64(stol(opl.raw_val, 0, 0)) : stol(opl.raw_val, 0, 0);
                } else {
                    Register& reg = regs[opl.raw_val];
                    val = laddr ? mmu.get64(reg.get64()) : reg.get64();
                }
                regs["rsp"].set64(regs["rsp"].get64() - cmd.vsize);

                mmu.set_by_size(regs["rsp"].get64(), val, cmd.vsize);
                ++pc;
            } else if(cmd.name == "pop" && op_cnt == 2) {
                uint64 val = mmu.get64(regs["rsp"].get64());
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
                if(laddr) {
                    LOG(ERROR)<<"Error command format!"<<endl;
                    throw -1;
                }
                if(opl.type == Operand::opd_type::number) {  // Print the data in the memory.
                    cout<<opl.raw_val<<": "<<mmu.get_by_size(stol(opl.raw_val, 0, 0), cmd.vsize)<<endl;
                } else {
                    cout<<opl.raw_val<<": "<<regs[opl.raw_val].get_by_size(cmd.vsize)<<endl;
                }
                pc++;
            } else if(cmd.name == "cmp" && op_cnt == 3) {
                pc++;
            } else if(cmd.name == "jmp" && op_cnt == 2) {
                if(laddr) {
                    LOG(ERROR)<<"Error command format!"<<endl;
                    throw -1;
                }
                if(opl.type == Operand::opd_type::number) {
                    pc = stol(opl.raw_val, 0, 0);
                } else {
                    pc = regs[opl.raw_val].get64();
                }
            } else if(cmd.name == "add" && op_cnt == 3) {
            } else if(cmd.name == "mul" && op_cnt == 3) {
            } else if(cmd.name == "div" && op_cnt == 3) {
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
                LOG(ERROR)<<"Not invalid number!"<<endl;
                throw -1;
            }
            ret.type = opd_type::number;
            ret.raw_val = opd;
        }
        return ret;
    }

}