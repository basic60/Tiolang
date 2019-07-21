#ifndef __TIOVM_INTERPRETER
#define __TIOVM_INTERPRETER
#include"mmu.h"
#include<queue>
namespace tiovm
{
    class Interpreter {
    private:
        MMU mmu;
        bool is_reg;
        void pre_process();
        void pre_process(std::string fname);
    public:
        void execute(std::string fname);
    };

    struct Command {
        std::string name;
        int vsize;

        static Command parse_cmd(const std::string& cmd);
    };

    struct Operand {
        enum class opd_type {
            reg,
            number
        };

        opd_type type;
        string raw_val;
        static Operand parse_operand(std::string val);
    };
}
#endif