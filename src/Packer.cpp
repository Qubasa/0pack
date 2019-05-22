/*
 * =====================================================================================
 *
 *       Filename:  Packer.cpp
 *
 *    Description:  Implementation of reloc packer
 *
 *        Version:  1.0
 *        Created:  19.12.2018 23:16:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Luis Hebendanz (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <LIEF/logging.hpp>
#include "Packer.hpp"

namespace Obfuscator {
namespace ELF {

    Packer::Packer(const std::string& src_path, const std::string dst_path)
    {

        _logger = get_logger("Packer");

        elf = Parser::parse(src_path);

        // Check if file is x64 binary
        if(elf->type() != ELF_CLASS::ELFCLASS64)
        {
            throw LIEF::not_supported("Only x64 binaries are supported right now");
        }

        // Check if binary is pie
        // Inbuild is pie function checks for an optional segment that is common in PIEs but not necessary
        Segment& first_segment = elf->get(SEGMENT_TYPES::PT_LOAD);
        if((elf->header().file_type() != E_TYPE::ET_DYN || first_segment.virtual_address() != 0))
        {
            throw LIEF::not_supported("Only PIE binaries are supported right now");
        }

        this->dst_path = dst_path;
    }

    std::string Packer::get_destination()
    {
        return this->dst_path;
    }

    /*
     * Adds a relocation that writes a single byte to the specified offset
     * */
     Relocation& Packer::add_write_reloc(uint8_t byte, uint64_t dest)
    {
        Relocation treloc = Relocation(dest,    // offset from binary
                RELOC_x86_64::R_X86_64_64,      // type of reloc
                byte,                           // byte to write
                true);                          // isRela
        Relocation& ret = elf->add_dynamic_relocation(treloc);
        elf->write(dst_path);

        return ret;
    }

    /*
     * Create a list of uninitialized (zeroed) write relocs
     * file_offset -> The file offset where the relocs should write to
     * */
    std::vector<std::reference_wrapper<Relocation>> Packer::init_relocs(size_t file_offset, size_t size)
     {
        /* Symbol temp_null_sym = Symbol(random_string(10)); */
        /* Symbol& null_sym = elf->add_dynamic_symbol(temp_null_sym); */

        /* null_sym.type(ELF_SYMBOL_TYPES::STT_NOTYPE); // st_info field */
        /* null_sym.binding(SYMBOL_BINDINGS::STB_WEAK); // global bindings don't work. TODO Why? */
        /* null_sym.value(0); */
        /* null_sym.size(0); // 0 means uknown size */


        /* size_t countDynSym = std::distance(elf->dynamic_symbols().begin(), elf->dynamic_symbols().end()); */

        /* _logger->info("Null sym is: {}", countDynSym); */

        /* elf->write(dst_path); */

        Symbol& null_sym = *elf->dynamic_symbols().begin();

        if(null_sym.value() != 0)
        {
            _logger->error("First dynamic symbol does not have a value of 0");
            throw LIEF::not_supported("Only binaries with the first dynsym value set to 0");
        }

        std::vector<std::reference_wrapper<Relocation>> relocs;

        for(size_t i = 0; i < size; i++) // +1 for jmp instruction at beginning
        {
            Relocation& rel = add_write_reloc(0, file_offset+i);
            rel.info(0); // Set to 0 symbol
            /* _logger->debug("Setting reloc to section index: {}", rel.info()); */
            relocs.push_back(rel);
        }

        return relocs;
     }

     /*
      * Tanslates a byte buffer to write relocs
      * */
    std::vector<std::reference_wrapper<Relocation>> Packer::buf_to_reloc(std::vector<uint8_t> bytes,  std::vector<std::reference_wrapper<Relocation>> init_reloc)
    {
        if(bytes.size() > init_reloc.size())
        {
            throw std::invalid_argument( "byte buffer is bigger then reloc list" );
        }

        for(size_t i = 0; i < bytes.size(); i++)
        {
            uint8_t b = bytes[i];


            Relocation& c_rel = init_reloc[i];

            c_rel.addend(b);

        }

        elf->write(dst_path);
        return init_reloc;
    }

    /*
     * jmp_address -> The address where to jmp to
     * relocs -> List of uninitialized relocations where the jmp with the address will be injected into
     * */
    std::vector<std::reference_wrapper<Relocation>> Packer::jmp_seg_reloc( uint64_t jmp_address, std::vector<std::reference_wrapper<Relocation>> relocs)
    {
        // TODO: Changed the input relocs to a vector
        // this change hasen't been tested so it is very possible that this function has a bug
        // probably the error is in the order in which the bytes gets written

        uint64_t virt = jmp_address;
        virt -= 5; // Because jmp adds own instr length to jmp address
        _logger->debug("Reloc jmp address is: 0x{0:x}", jmp_address);

        for(size_t i = 0; i < relocs.size(); i++)
        {
            Relocation& rel = relocs[i];

            if(i == 0)
            {
                rel.addend(0xe9); // jmp instruction
                continue;
            }

            unsigned char current = ((unsigned char*) &virt)[i-1];

            _logger->debug("Writing byte as realloc 0x{0:x}", current);

            rel.addend(current);
        }

        elf->write(dst_path);
        return relocs;
    }


    /*
     * Randomizes the addresses of the static symbol table
     * Confuses disassemblers a lot
     * */
    void Packer::randomize_static_symbol_table()
    {
        // Init pseudo random generator
        std::default_random_engine generator;
        generator.seed(time(NULL));
        std::uniform_int_distribution<int> distribution(0x0, elf->original_size()-100); // generates number in the range of the binary
        auto rand_offset = std::bind ( distribution, generator );


        // Fuckup static symbol table to double the fun
        for(Symbol& s: elf->static_symbols())
        {
            if(s.name() == "main")
            {
                s.value(rand_offset());
                _logger->debug("New fake address of main func: 0x{0:x}", s.value());
            }else
            {
                s.value(rand_offset());
            }
        }
        elf->write(dst_path);
    }


     void Packer::strip()
    {
        elf->strip();
        elf->write(dst_path);
    }


    /*
     * Sets the entrypoint
     * */
    void Packer::set_entrypoint(uint64_t entry)
    {
        elf->header().entrypoint(entry);
        elf->write(dst_path);
        _logger->debug("Setting entrypoint to 0x{0:x}", get_entrypoint());
    }

    /*
     * Returns the entrypoint
     * */
    uint64_t Packer::get_entrypoint()
    {
        return elf->header().entrypoint();
    }


    /*
     *  TODO: Randomize selection of bytes
     *  Picks references to bytes out of the section of the binary matching the bytes in the flag argument.
     *  Used for creating a "hashsum" over the binary and hiding a flag.
     * */
    std::vector<std::pair<unsigned char, uint64_t>> Packer::flag_to_ptrs(std::initializer_list<std::string> blacklist_sections, std::string flag)
    {
        std::vector<std::pair<unsigned char, uint64_t>> ptrs;

        for(size_t flag_index = 0; flag_index < flag.size(); flag_index++)
        {
            unsigned char flag_char = flag[flag_index];

            for(Section& sec : elf->sections())
            {
                // Check if is blacklisted
                bool blacklisted = false;
                for(std::string bl : blacklist_sections)
                {
                    if(bl.compare(sec.name()) == 0)
                    {
                        blacklisted = true;
                        break;
                    }
                }
                if(blacklisted)
                {
                    _logger->debug("Blacklisted {}", sec.name());
                    continue;
                }

                bool found = false;
                std::vector<uint8_t> sect_content = sec.content();
                for(size_t i = 0; i < sect_content.size(); i++)
                {
                    uint8_t sect_char = sect_content[i];

                    if(flag_char == sect_char)
                    {
                        uint64_t offset = sec.virtual_address() + i;
                        _logger->info("Found char {} at offset {} in section {}", flag_char, offset, sec.name());
                        ptrs.push_back(std::make_pair(sect_char, offset));
                        found = true;
                        break;
                    }
                } // End for section content bytes

                if(found)
                {
                    break;
                }
            } // End for section
        } // End for flag bytes

        for(std::pair<unsigned char, uint64_t> p : ptrs)
        {
            std::cout << p.first  << ", " << p.second << std::endl;
        }

        // Check if every byte of flag has a pointer
        if(ptrs.size() < flag.size())
        {
            _logger->error("Only found {}/{} pointers", ptrs.size(), flag.size());
            throw std::runtime_error("Didn't find all pointers");
        }

        _logger->info("Found {}/{} pointers", ptrs.size(), flag.size());
        return ptrs;
    }

    /*
     * Xor a section of a binary with a simple 1 byte key
     * */
    Section& Packer::xor_section(std::string section_name, uint8_t key)
    {
        Section& sect = elf->get_section(section_name);
        std::vector<uint8_t> sect_content = sect.content();

        for(auto i = sect_content.begin(); i != sect_content.end(); i++){
            *i ^=  key;
        }
        sect.content(sect_content);
        elf->write(dst_path);

        return sect;
    }

    /*
     * Finished building the packed executable
     */
    void Packer::build()
    {
        _logger->info("Packed binary written to: {}", dst_path.path::c_str());
        fs::permissions(dst_path, fs::perms::owner_all);
    }


    /*
     * Gets the first segment of the binary
     * */
    Segment& Packer::get_first_seg()
    {
        Segment& first_segment = elf->get(SEGMENT_TYPES::PT_LOAD);
        return first_segment;
    }


    /*
     * Sets the first segment of the binary to be writeable
     * */
    void Packer::set_first_seg_rwx()
    {
        Segment& first_segment = elf->get(SEGMENT_TYPES::PT_LOAD);
        first_segment.add(ELF_SEGMENT_FLAGS::PF_W);
        first_segment.add(ELF_SEGMENT_FLAGS::PF_R);
        first_segment.add(ELF_SEGMENT_FLAGS::PF_X);
    }

    /*
     * Creates a new empty load segment
     * To add content to this segment you have to specify the size with segment.physical_size(int size)
     * */
    Segment& Packer::new_empty_load_segment()
    {
        Segment temp_segment{};
        temp_segment.content({0});
        temp_segment.physical_size(1000);
        temp_segment.flags(ELF_SEGMENT_FLAGS::PF_X | ELF_SEGMENT_FLAGS::PF_R);
        temp_segment.type(SEGMENT_TYPES::PT_LOAD);
        Segment& ret_segment = elf->add(temp_segment);
        elf->write(dst_path);

        return ret_segment;
    }

    /*
     * Creates a load segment with content loaded into it
     *
     * */
    Segment& Packer::new_load_segment(std::vector<uint8_t> content)
    {
        if(content.size() == 0)
        {
            throw std::invalid_argument("Content in vector has to be greater then zero");
        }

        Segment temp_segment{};
        temp_segment.content( content);
        temp_segment.flags(ELF_SEGMENT_FLAGS::PF_X | ELF_SEGMENT_FLAGS::PF_R);
        temp_segment.type(SEGMENT_TYPES::PT_LOAD);
        Segment& ret_segment = elf->add(temp_segment);
        elf->write(dst_path);

        return ret_segment;
    }

    /*
     * Copies the sections defined in orig_sects and creates dummy copies with broken assembly in it
     *
     * */
    void Packer::create_copy_dummy_sections(std::vector<std::string> orig_sects, size_t num)
    {
        for(size_t i = 0; i < num; i++)
        {
            std::string orig_name = orig_sects[i%orig_sects.size()];

            if(!elf->has_section(orig_name))
            {
                throw std::invalid_argument("Section not found");
            }

            const Section& orig = elf->get_section(orig_name);

            Section& dummy = copy_to_rand_sect(orig.name());

            std::vector<uint8_t> content = dummy.content();
            for(size_t d=0; d<content.size(); d++)
            {
                // Every 4 bytes
                if(d%2 == 0)
                {
                    content[d] = content[d] % 11; // TODO: Make random byte
                }

            }
            dummy.content(content);
        }

        elf->write(dst_path);
    }

    /*
     * Copies a section name and randomizes its name
     * */
    Section& Packer::copy_to_rand_sect(std::string section_name)
    {
        std::string rand_sect_name;
        while(true)
        {
            rand_sect_name = "." + random_string(10);
            if(elf->has_section(rand_sect_name))
            {
                _logger->debug("Rand sect name already exists: {}", rand_sect_name);
                continue;
            }
            _logger->debug("Found new rand sect name: {}", rand_sect_name);
            break;
        }

        if(!elf->has_section(section_name))
        {
            _logger->error("Section not found: {}", section_name);
            throw std::invalid_argument("Section not found");
        }

        Section& sect = elf->get_section(section_name);
        Section tmp_sect = Section(rand_sect_name);

        // Copy over data and flags
        tmp_sect.content(sect.content());
        tmp_sect.size(sect.size());
        tmp_sect.add(ELF_SECTION_FLAGS::SHF_ALLOC); // TODO: Make perms for every section equal
        tmp_sect.type(sect.type());

        Section& rand_sect = elf->add(tmp_sect);

        _logger->debug("Rand sect address 0x{0:x}", rand_sect.virtual_address());


        elf->write(dst_path);
        return rand_sect;
    }
}
}
