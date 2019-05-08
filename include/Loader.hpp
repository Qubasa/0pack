/*
 * =====================================================================================
 *
 *       Filename:  Loader.hpp
 *
 *    Description:  Loader builder
 *
 *        Version:  1.0
 *        Created:  19.12.2018 23:34:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author: Luis Hebenbdanz (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _Loader_
#define _Loader_

#include <sstream>
#include <iostream>
#include <vector>
#include <experimental/filesystem>

#include <LIEF/LIEF.hpp>
#include "Utils.hpp"
#include "RegisterManager.hpp"
#include "fmt/format.h"

using namespace LIEF::ELF;
using namespace Obfuscator::ELF::RegName_wrapper;
namespace fs = std::experimental::filesystem;

namespace Obfuscator {
namespace ELF {

class Loader
{
    public:
        Loader();
        /* Loader(const Loader&) = delete; */
        /* Loader& operator=(const Loader& l) = delete; */

        void push_all_registers(std::initializer_list<RegName> black_list);
        void pop_all_registers(std::initializer_list<RegName> black_list);
        std::vector<uint8_t> compile();
        std::vector<uint8_t> compile(fs::path dest);
        RegName get_current_pid();
        RegName count_digits(RegName name);
        RegName int_to_str(RegName num_digits);
        RegName get_base_address();
        void clear();
        void compile_to_standalone(fs::path dest);
        void print();

        template <typename... Args>
        void append(std::string code, const Args & ... args);

        void set_page_rwx(RegName base_address, uint64_t page_offset, uint64_t page_size);
        void set_page_rx(RegName base_address, uint64_t page_offset, uint64_t page_size);
        void xor_decrypt(RegName base_address, uint64_t dest, uint64_t src, size_t size, uint8_t key);
        void jmp(RegName base_address, uint64_t dest);
        void functionPrologue(size_t stackBytes);
        void functionEpilogue();


    protected:
        RegisterManager rmg;
        std::stringstream asm_buf;
        std::shared_ptr<Binary> elf;


    private:
        std::shared_ptr<spdlog::logger> _logger;
};

}
}

#endif
