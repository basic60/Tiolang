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
    }

    void Interpreter::execute(string fname) {
        ifstream ifs(fname);
        string line;
        while (getline(ifs, line)) {
            if(line == "") {
                continue;
            }

            queue<string> q;
            bool laddr = false;
            bool raddr = false;
            bool& swt = laddr;
            int op_cnt = 0;
            for(auto& i : split(line, ' ')) {
                if(i == "@") {
                    swt = true;
                } else if (i == ",") {
                    swt = raddr;
                } else {
                    q.push(move(i));
                    op_cnt++;
                }
            }

            string var = q.front();
            q.pop();
            Command&& cmd = Command::parse_cmd(var);

            if(cmd.name == "move" && op_cnt == 3) {
                const Operand&& opl = Operand::parse_operand(q.front());q.pop();
                const Operand&& opr = Operand::parse_operand(q.front());q.pop();
                if(opl.type == Operand::opd_type::number) {
                    if(!laddr) {
                        LOG(ERROR)<<"Cann't move value to a constant!"<<endl;
                        throw -1;
                    }
                }
            } else {
                LOG(ERROR)<<"Unknown command: " + line<<". Please check the command foramt."<<endl;
                throw -1;
            }
        }
    }

    Command Command::parse_cmd(const string& cmd) {
        Command ret;
        if(cmd[cmd.size() -1 ] == 'c' || cmd[cmd.size() -1 ] == 'i' || cmd[cmd.size() -1 ] == 'l') {
            ret.vsize = cmd[cmd.size() -1 ] == 'c' ? 1 : cmd[cmd.size() -1 ] == 'i' ? 4 : 8;
            ret.name = cmd.substr(0, cmd.size() - 1);
        } else {
            ret.name = cmd;
        }
        return ret;
    }

    Operand Operand::parse_operand(std::string opd) {
        Operand ret;
        if(Register::valid_reg(opd)) {
            ret.type = opd_type::reg;
            ret.raw_val = opd;
        }
        return ret;
    }
}