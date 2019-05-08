
#include <LIEF/LIEF.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" //support for stdout logging

#include "Utils.hpp"
#include "Packer.hpp"

using namespace LIEF::ELF;
using namespace Obfuscator::ELF;

int main(int argc, const char *argv[]) {

    // Init Logger
    spdlog::set_level(spdlog::level::info);
    auto console = get_logger("main");
    console->debug("Debug messages are enabled!");
    console->info("Started 0Pack") ;

    // Argument parsing
    if (argc != 4) {
        console->error("Usage: <in_binary> <payload_src> <out_binary>");
        exit(-1);
    }

    std::string payload_path(argv[2]);

    // Check if targeted binary exists
    if(!does_file_exist(argv[1]))
    {
        console->error("File not found: {}", argv[1]);
        exit(-1);
    }

    // Check if payload bin exists
    if(!does_file_exist(argv[2]))
    {
        console->error("File not found: {}", argv[2]);
        exit(-1);
    }

    // Parse targeted binary
    Packer packer{argv[1], argv[3]};

    // Read compiled decryptor stub into binary vector
    std::ifstream payload_fd{payload_path, std::ios::in };
    if (!payload_fd.is_open()) {
        console->error("Couln't open payload src file: {}", payload_path.c_str());
        exit(-1);
    }
    std::string payload_buffer{ std::istreambuf_iterator<char>(payload_fd), std::istreambuf_iterator<char>()};
    payload_fd.close();

    console->info(fmt::format("Original entrypoint is {:#x}", packer.get_entrypoint() ));

    Loader loader;
    loader.push_all_registers({RegName::rbp, RegName::rsp});
    loader.append(payload_buffer);
    loader.pop_all_registers({RegName::rbp, RegName::rsp});

    size_t payload_size = loader.compile().size() + 10;
    std::vector<std::reference_wrapper<Relocation>> empty_buf_relocs = packer.init_relocs(0, payload_size);

    console->info("Estimated payload size {}", payload_size);
    console->info("Empty buf relocs {}", empty_buf_relocs.size());

    size_t jmp_addr = packer.get_entrypoint();

    console->info("Jmp address: {:#x}", jmp_addr);

    loader.append(R"(
            jmp {:#x}
            )", jmp_addr );


    std::vector<uint8_t> compiled_payload = loader.compile();
    if(compiled_payload.size() > payload_size)
    {
        console->error("The compiled payload is bigger then the initialized relocations!");
        exit(2);
    }

    loader.compile_to_standalone(".");

    console->info("Real payload size {}", compiled_payload.size());

    // Set first segment to writeable
    packer.set_first_seg_writeable();
    packer.buf_to_reloc(compiled_payload, empty_buf_relocs);

    console->info(fmt::format("New entrypoint is {:#x}", packer.get_entrypoint() ));

    // Set entrypoint to 0
    packer.set_entrypoint(0);

    packer.build();
}
