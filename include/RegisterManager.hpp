/*
 * =====================================================================================
 *
 *       Filename:  Registers.hpp
 *
 *    Description:  Managers Registers
 *
 *        Version:  1.0
 *        Created:  20.12.2018 01:51:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Qubasa (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _RegisterManager_
#define _RegisterManager_
#include <string>
#include <functional>
#include <map>
#include <vector>
#include "spdlog/sinks/stdout_color_sinks.h" //support for stdout logging
#include "Utils.hpp"

namespace Obfuscator {
namespace ELF {

namespace RegName_wrapper // to be able to import enum through namespace
{
    enum class RegName
    {
        rax,
        rbx,
        rcx,
        rdx,
        rsi,
        rdi,
        rsp,
        rbp,
        r8,
        r9,
        r10,
        r11,
        r12,
        r13,
        r14,
        r15
    };
}
using namespace RegName_wrapper;

class Register
{
    public:
        Register(std::string name): name(name), holdsValue(false){};
        const std::string name;
        bool holdsValue;
};

class RegisterManager
{
    public:
        RegisterManager();
        Register& get_by_str(std::string name);
        Register& get_by_enum(RegName name);
        std::string get_name(RegName name);
        void setRegisterHolding(RegName name, bool holding);
        std::vector<std::reference_wrapper<Register>> disjoint(std::initializer_list<RegName> black_list);
        bool getRegisterHolding(RegName name);

    private:
        std::shared_ptr<spdlog::logger> _logger;

        Register rax{"rax"};
        Register rbx{"rbx"};
        Register rcx{"rcx"};
        Register rdx{"rdx"};
        Register rsi{"rsi"};
        Register rdi{"rdi"};
        Register rsp{"rsp"};
        Register rbp{"rbp"};
        Register r8{"r8"};
        Register r9{"r9"};
        Register r10{"r10"};
        Register r11{"r11"};
        Register r12{"r12"};
        Register r13{"r13"};
        Register r14{"r14"};
        Register r15{"r15"};

        const std::map<RegName, std::reference_wrapper<Register>> all_registers = {
            { RegName::rax,  std::ref(rax) },
            { RegName::rbx,  std::ref(rbx) },
            { RegName::rcx,  std::ref(rcx) },
            { RegName::rdx,  std::ref(rdx) },
            { RegName::rsi,  std::ref(rsi) },
            { RegName::rdi,  std::ref(rdi) },
            { RegName::rsp,  std::ref(rsp) },
            { RegName::rbp,  std::ref(rbp) },
            { RegName::r8,   std::ref(r8)  },
            { RegName::r9,   std::ref(r9)  },
            { RegName::r10,  std::ref(r10) },
            { RegName::r11,  std::ref(r11) },
            { RegName::r12,  std::ref(r12) },
            { RegName::r13,  std::ref(r13) },
            { RegName::r14,  std::ref(r14) },
            { RegName::r15,  std::ref(r15) }
        };

};
}
}

#endif
