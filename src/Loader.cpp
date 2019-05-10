/*
 * =====================================================================================
 *
 *       Filename:  Loader.cpp
 *
 *    Description:  Implementation of the loader code
 *
 *        Version:  1.0
 *        Created:  19.12.2018 23:44:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Luis Hebendanz (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <regex>

#include "Loader.hpp"

namespace Obfuscator {
namespace ELF {

    Loader::Loader()
    {
        asm_buf << "use64" << std::endl;
        _logger = get_logger("Loader");
    }


    std::vector<uint8_t> Loader::compile()
    {
        return Loader::compile(fs::temp_directory_path());
    }

    void Loader::compile_to_standalone(fs::path dest_dir)
    {
        _logger->debug("Compiling destination: {}", dest_dir.c_str());

        fs::path src_tmpf = dest_dir / "src.fasm";
        fs::path dst_tmpf = dest_dir / "dst.elf";

        std::string patched_buf = asm_buf.str();

        std::regex re("use64");
        patched_buf = std::regex_replace (patched_buf, re,"format ELF64 executable", std::regex_constants::format_first_only);

        // Write asm buf to tmp file
        std::ofstream src_ostream{src_tmpf, std::ios::out};
        if(!src_ostream.is_open())
        {
            _logger->error("Couln't open compile src file: {}", src_tmpf.c_str());
            exit(-1);
        }
        src_ostream << patched_buf;
        src_ostream.close();

        std::stringstream build_command;
        build_command << "fasm " << src_tmpf << " " << dst_tmpf;
        _logger->info("Executing compile command: {}", build_command.str());

        if(system(build_command.str().c_str()) != 0)
        {
            _logger->error("Compiling failed.");
            exit(1);
        }
    }

    std::vector<uint8_t> Loader::compile(fs::path dest_dir)
    {
        _logger->debug("Compiling destination: {}", dest_dir.c_str());

        fs::path src_tmpf = dest_dir / "src.fasm";
        fs::path dst_tmpf = dest_dir / "dst.bin";

        // Write asm buf to tmp file
        std::ofstream src_ostream{src_tmpf, std::ios::out};
        if(!src_ostream.is_open())
        {
            _logger->error("Couln't open compile src file: {}", src_tmpf.c_str());
            exit(-1);
        }
        src_ostream << asm_buf.str();
        src_ostream.close();

        // Compile asm file
        std::stringstream build_command;
        build_command << "fasm " << src_tmpf << " " << dst_tmpf;
        _logger->info("Executing compile command: {}", build_command.str());

        if(system(build_command.str().c_str()) != 0)
        {
            _logger->error("Compiling failed.");
            exit(1);
        }

         // Read compiled decryptor stub into binary vector
        std::ifstream decryptor{dst_tmpf, std::ios::in | std::ios::binary };
        if (!decryptor.is_open()) {
            _logger->error("Couln't open compile src file: {}", dst_tmpf.c_str());
            exit(-1);
        }
        std::vector<uint8_t> decryptor_buffer{ std::istreambuf_iterator<char>(decryptor), std::istreambuf_iterator<char>()};
        decryptor.close();

        return decryptor_buffer;
    }

    void Loader::clear()
    {
        asm_buf.flush();
        asm_buf << "use64" << std::endl;
    }

    void Loader::print()
    {
        std::cout << this->asm_buf.str();
    }


    template <typename... Args>
    void Loader::append(std::string code, const Args & ... args)
    {

        std::stringstream code_stream;
        std::regex re("format ELF64( executable)?");
        code_stream << std::regex_replace (
                fmt::format(
                    code,
                    args...
                    ),
                re,
                "; Replaced format header",
                std::regex_constants::format_first_only);

        std::string current_line;
        while(std::getline(code_stream, current_line ))
        {
            std::regex re_whitespaces("^\\s+");
            asm_buf << std::regex_replace (current_line, re_whitespaces,"", std::regex_constants::match_any) << std::endl;
        }
    }

    RegName Loader::get_current_pid()
    {
        append(R"(
            mov rax, 39 ; sys_getpid
            syscall
            ; Output:
            ; rax --> pid of current process
        )");

        return RegName::rax;
    }

    RegName Loader::count_digits(RegName digit)
    {
        append(R"(
            ; Input:
            ; rax --> reg holding digit
            ; Uses Stack
            ; --------------
            mov rax, {}; save digit to r8
            mov r8, rax
            mov rbx, 10 ; divisor
            xor r9, r9  ; Set to 0
            count:
                xor rdx, rdx ; Set rdx to 0
                inc r9 ; digit counter
                div rbx ; divide rax with 10
                push dx
                test rax, rax
                jnz count
            ; Output:
            ; r8 --> original number
            ; r9 --> Number of digits
            ; stack --> remainders. pop r9 times
        )", rmg.get_name(digit).c_str());

        return RegName::r9;
    }

    RegName Loader::int_to_str(RegName num_digits)
    {
        append(R"(
            ; Input:
            ; rcx --> length of string / number of digits
            ; rsp/stack --> single byte pop-able digits
            ; r14 --> Stack pointer with 20 bytes of space
            ; Uses Stack
            ; -----------
            mov rcx, {} ; rcx as counter for loop
            xor rdi, rdi
            xor r11, r11
            convert:
                pop ax ; get single digit from stack
                add al, 48 ; convert byte to ascii

                lea r10, [r14+r11] ; inc pointer to save character on stack
                mov [r10], al ; save byte to stack
                inc r11
            loop convert
            ; Output:
            ; r14 -> start of pid string
        )", rmg.get_name(num_digits).c_str());

        return RegName::r14;
    }

    /*
     * Note: popf is hardcoded
     */
    void Loader::pop_all_registers(std::initializer_list<RegName> black_list)
    {
        std::vector<std::reference_wrapper<Register>> regs = rmg.disjoint(black_list);

        // Reverse the order of pop instruction vs the push instructions
        std::reverse(regs.begin(), regs.end());

        for(Register& reg : regs)
        {
            asm_buf << "pop " << reg.name << std::endl;
            reg.holdsValue = true;
        }

        // TODO: hardcoded popf
        asm_buf << "popf" << std::endl;
    }

    /*
     * Note: pushf is hardcoded
     */
    void Loader::push_all_registers(std::initializer_list<RegName> black_list)
    {
        std::vector<std::reference_wrapper<Register>> regs = rmg.disjoint(black_list);

        // TODO: hardcoded pushf
        asm_buf << "pushf" << std::endl;

        for(Register& reg : regs)
        {
            asm_buf << "push " << reg.name << std::endl;
            reg.holdsValue = false;
        }
    }

    void Loader::set_page_rwx(RegName base_address, uint64_t page_offset, uint64_t page_size)
    {
        append(R"(
            ; Set the memory page to RWX
            mov rdi, {}
            add rdi, {} ; pointer to beginning of page
            mov rsi, {} ; size
            mov rdx, 7 ; RWX
            mov rax, 0xa ; mprotect syscall
            syscall
        )", rmg.get_name(base_address).c_str(), page_offset, page_size);
    }

    void Loader::set_page_rx(RegName base_address, uint64_t page_offset, uint64_t page_size)
    {
       append(R"(
                ; Set the memory page to RWX
                mov rdi, {}
                add rdi, {} ; pointer to beginning of page
                mov rsi, {} ; size
                mov rdx, 5 ; RX
                mov rax, 0xa ; mprotect syscall
                syscall
            )", rmg.get_name(base_address).c_str(), page_offset, page_size);
    }

    void Loader::jmp(RegName base_address, uint64_t dest)
    {
        _logger->debug("Jumping to address 0x{0:x}", dest);

        append(R"(
            mov r10, {}
            add r10, {}
            jmp r10
        )", rmg.get_name(base_address).c_str(), dest);
    }

    void Loader::xor_decrypt(RegName base_address, uint64_t dest, uint64_t src, size_t size, uint8_t key)
    {
        std::string label = random_string(7); //TODO: Only character string

        append(R"(
            ; Input:
            ; rdi --> Decrypt destination pointer
            ; r10 --> Content to decrypt pointer
            ; --------
            mov rdi, {}
            add rdi, {} ; needed for stosb
            mov rsi, r10 ;
            add rsi, {} ; needed for lodsb
            mov rcx, {}
            "{}:"
                lodsb ; load byte from rsi address save to al and increment address
                xor al, {}
                stosb ; reads byte from al and stores to rdi address
            loop {} ; loops till rcx is 0
                    )", rmg.get_name(base_address).c_str(), dest, src, size, label.c_str(), static_cast<unsigned int>(key), label.c_str());
    }

    void Loader::functionPrologue(size_t stackBytes)
    {
        append(R"(
            push rbp
            mov rbp, rsp
            sub rsp, {}
            )",  stackBytes);
    }

    void Loader::functionEpilogue()
    {
        append(R"(
            mov rsp, rbp
            pop rbp
        )");
    }

    RegName Loader::get_base_address()
    {
        // Allocate 20 bytes because this is max int as characters
        // Allocate 5 bytes for string "/maps"
        functionPrologue(25);

        append(R"(
            mov r14, rsp ; Location of string
        )");

        RegName pid_reg = get_current_pid();
        RegName num_digits = count_digits(pid_reg);
        RegName startOfPIDStr = int_to_str(num_digits);

        append(R"(
          ; ---- Important registers ----
            ; r8 --> pid
            ; r9 --> Number of digits
            ; r14 --> start of pid string
            ; STACK: 25 bytes allocated

            ; Build path in stack
            sub rsp, 6
            mov dword [rsp], "/pro"
            mov word [rsp+4], "c/"
            mov dword [r14+r9], "/map"
            mov word [r14+r9+4], "s"
            mov r14, rsp

            ; ---- Important registers ----
            ; r14 --> start of pid path
            ; STACK: 31 bytes allocated


            ; Open mmap file as read only
            ; r14 --> start of pid path
            mov rax, 2 ; sys_open
            lea rdi, [r14]
            mov rsi, 0 ; O_RDONLY
            mov rdx, 0
            syscall
            ; Output:
            ; rax --> fd


            sub rsp, 12 ; number of characters first address in /proc/<pid>/maps

            ; ---- Important registers ----
            ; rax --> fd
            ; STACK: 43 bytes allocated


            ; Read mmap file
            mov rdi, rax ; Save filedescriptor to rdi
            mov rax, 0 ; read file
            mov rsi, rsp ; use stack as buffer
            mov rdx, 12 ; read the first n bytes
            syscall
            ; Output:
            ; rsp --> base address as string

            ; ---- Important registers ----
            ; rdi --> fd
            ; rsp --> base address as string
            ; STACK: 43 bytes allocated

            ; Close file handle
            mov rax, 3 ; sys_close
            mov rdi, rdi ;fd
            syscall

            ; ---- Important registers ----
            ; rsp --> base address as string
            ; STACK: 43 bytes allocated

            ; Stack base-address as string to register as int
            mov rsi, 12 ; counter for lodsb
            mov rcx, 12
            mov rsi, rsp ; save base address of string to rsi
            jmp string_to_int

            hex_to_int:
            sub al, 0x60
            add al, 9
            ret

            string_to_int:
            ; Used regs: rax, rbx, rdi

            lodsb ; Load byte at address DS:(E)SI into AL
            cmp al, '9' ; Check if is in hex area
            jng no_hex ; If is hex char do different math operation
            call hex_to_int
            jmp norm

            no_hex:
            sub al, 48 ; string to int

            norm:
            mov dl, al

            lodsb ; Load byte at address DS:(E)SI into AL
            cmp al, '9' ; Check if is in hex area
            jng no_hex2 ; If is hex char do different math operation
            call hex_to_int
            jmp norm2

            no_hex2:
            sub al, 48 ; string to int

            norm2:
            mov bl, al

            shl dx, 4
            or dl, bl

            mov dil, dl
            shl rdi, 8

            sub rcx, 2
            test rcx, rcx
            jnz string_to_int
            shr rdi, 8
            ; Output
            ; rdi --> base address of current process

            mov r10, rdi ; Save base address

            ; ---- Important registers ----
            ; r10 --> base address of current process
            ; STACK: 43 bytes allocated

        )");

        functionEpilogue();

        return RegName::r10;
    }

}
}
