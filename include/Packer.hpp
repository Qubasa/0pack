/*
 * =====================================================================================
 *
 *       Filename:  Packer.hpp
 *
 *    Description:  A ELF binary packer library
 *
 *        Version:  1.0
 *        Created:  19.12.2018 23:08:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Luis Hebendanz (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef _Packer_
#define _Packer_

#include <LIEF/LIEF.hpp>
#include "spdlog/sinks/stdout_color_sinks.h" //support for stdout logging
#include <Utils.hpp>
#include <experimental/filesystem>
#include "Loader.hpp"
#include <queue>
#include <tuple>
#include <random>

using namespace LIEF::ELF;
namespace fs = std::experimental::filesystem;

namespace Obfuscator {
namespace ELF {
class Packer
{
    public:
        Packer(const std::string& src_path, const std::string dst_path);

        Packer& operator=(const Packer& ) = delete;
        Packer(const Packer& copy) = delete;

        Relocation& add_write_reloc(uint8_t byte, uint64_t dest);
        std::vector<std::reference_wrapper<Relocation>> buf_to_reloc(std::vector<uint8_t> bytes,  std::vector<std::reference_wrapper<Relocation>> relocs);
        Section& xor_section(std::string section_name, uint8_t key);
        Section& copy_to_rand_sect(std::string section_name);
        Segment& new_load_segment(std::vector<uint8_t> content);
        Segment& new_empty_load_segment();

        std::vector<std::reference_wrapper<Relocation>> jmp_seg_reloc( uint64_t jmp_address, std::vector<std::reference_wrapper<Relocation>> relocs);
        std::vector<std::reference_wrapper<Relocation>> init_relocs(size_t file_offset, size_t size);

         std::vector<std::pair<unsigned char, uint64_t>> flag_to_ptrs(std::initializer_list<std::string> blacklist_sections, std::string flag);
        void create_copy_dummy_sections(std::vector<std::string> orig_sects, size_t num);
        void randomize_static_symbol_table();
        void set_entrypoint(uint64_t entry);
        uint64_t get_entrypoint();
        void set_first_seg_writeable();
        Segment& get_first_seg();
        void build();

        std::unique_ptr<Binary> elf;
        Loader loader;

    private:
        std::shared_ptr<spdlog::logger> _logger;
        fs::path dst_path;
};
}
}

#endif
