
#include <LIEF/LIEF.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" //support for stdout logging
#include <cxxopts.hpp>


#include "Utils.hpp"
#include "Packer.hpp"

using namespace LIEF::ELF;
using namespace Obfuscator::ELF;

int main(int argc, char *argv[]) {

    // Argument parser
    cxxopts::Options options("0pack", "Injects shellcode as relocations into an ELF binary");

    options.add_options()
      ("d,debug", "Enable debugging")
      ("i,input", "Input file path. Required.", cxxopts::value<std::string>())
      ("p,payload", "Fasm payload path. ", cxxopts::value<std::string>())
      ("b,bin_payload", "Binary payload path.", cxxopts::value<std::string>())
      ("o,output", "Output file path. Required.", cxxopts::value<std::string>())
      ("s,strip", "Strip the binary. Optional.")
      ;

    auto result = options.parse(argc, argv);

    if(!result.count("input") || !result.count("output") || (!result.count("payload") && !result.count("bin_payload")))
    {
        std::cout << options.help({}) << std::endl;
        exit(1);
    }

    bool debugMode = result["debug"].as<bool>();

    if(result.count("payload") && result.count("bin_payload"))
    {
        std::cerr << "Options 'payload' and 'bin_payload' are mutually exclusive" << std::endl;
        exit(1);
    }

    std::string payload_path = "";
    if(result.count("payload"))
    {
        payload_path = result["payload"].as<std::string>();
    }

    if(result.count("bin_payload"))
    {
        payload_path = result["bin_payload"].as<std::string>();
    }

    std::string src_file = result["input"].as<std::string>();
    std::string output_file = result["output"].as<std::string>();

    // Init Logger
    if(debugMode)
    {
        spdlog::set_level(spdlog::level::debug);
    }else
    {
        spdlog::set_level(spdlog::level::info);
    }

    auto console = get_logger("main");
    console->debug("Debug messages are enabled!");
    console->info("Started 0Pack") ;


    // Check if targeted binary exists
    if(!does_file_exist(src_file))
    {
        console->error("File not found: {}", src_file);
        exit(1);
    }

    // Check if payload bin exists
    if(!does_file_exist(payload_path))
    {
        console->error("File not found: {}", payload_path);
        exit(1);
    }

    // Parse targeted binary
    Packer packer{src_file, output_file};

    // Shared variables
    std::vector<std::reference_wrapper<Relocation>> empty_buf_relocs;
    std::vector<uint8_t> compiled_payload;
    size_t jmp_addr;

    // If fasm payload
    if(result.count("payload"))
    {
        // Read compiled decryptor stub into binary vector
        std::ifstream payload_fd{payload_path, std::ios::in };
        if (!payload_fd.is_open()) {
            console->error("Couln't open payload src file: {}", payload_path);
            exit(1);
        }
        std::string payload_buffer{ std::istreambuf_iterator<char>(payload_fd), std::istreambuf_iterator<char>()};
        payload_fd.close();

        console->info("Original entrypoint is {:#x}", packer.get_entrypoint() );

        Loader loader;
        loader.push_all_registers({RegName::rbp, RegName::rsp});
        loader.append(payload_buffer);
        loader.pop_all_registers({RegName::rbp, RegName::rsp});

        size_t payload_size = loader.compile().size() + 10;
        empty_buf_relocs = packer.init_relocs(0, payload_size);

        console->debug("Estimated payload size {}", payload_size);
        console->debug("Empty buf relocs {}", empty_buf_relocs.size());

        jmp_addr = packer.get_entrypoint();

        console->debug("Jmp address: {:#x}", jmp_addr);

        loader.append(R"(
                jmp {:#x}
                )", jmp_addr );


        compiled_payload = loader.compile();
        if(compiled_payload.size() > payload_size)
        {
            console->error("The compiled payload is bigger then the initialized relocations!");
            exit(1);
        }

        /* loader.compile_to_standalone("."); */

        console->info("Payload size: {}", compiled_payload.size());
    }

    // If binary payload
    if(result.count("bin_payload"))
    {
        // Read compiled decryptor stub into binary vector
        std::ifstream payload_fd{payload_path, std::ios::in | std::ios::binary };
        if (!payload_fd.is_open()) {
            console->error("Couln't open payload bin file: {}", payload_path);
            exit(-1);
        }
        compiled_payload = { std::istreambuf_iterator<char>(payload_fd), std::istreambuf_iterator<char>()};
        payload_fd.close();

        console->info("Original entrypoint is {:#x}", packer.get_entrypoint() );

        size_t payload_size = compiled_payload.size() + 10;
        empty_buf_relocs = packer.init_relocs(0, payload_size);

        console->debug("Estimated payload size {}", payload_size);
        console->debug("Empty buf relocs {}", empty_buf_relocs.size());

        jmp_addr = packer.get_entrypoint();

        console->debug("Jmp address: {:#x}", jmp_addr);

        // Append jmp to main
        Loader loader;
        loader.append(R"(
                jmp {:#x}
                )", jmp_addr );

        std::vector<uint8_t> jmp_p = loader.compile();
        compiled_payload.insert(compiled_payload.end(), jmp_p.begin(), jmp_p.end());

        if(compiled_payload.size() > payload_size)
        {
            console->error("The compiled payload is bigger then the initialized relocations!");
            exit(1);
        }

        /* loader.compile_to_standalone("."); */

        console->info("Payload size: {}", compiled_payload.size());
    }

    // Set first segment to rwx
    packer.set_first_seg_rwx();
    packer.buf_to_reloc(compiled_payload, empty_buf_relocs);

    console->debug(fmt::format("Entrypoint after adding Relocs is {:#x}", packer.get_entrypoint() ));

    if(jmp_addr != packer.get_entrypoint())
    {
        console->error("The jmp address should match the entrypoint! But it does not");
        fs::remove(packer.get_destination());
        exit(1);
    }

    if(result.count("strip"))
    {
        console->info("Stripping static symbols");
        packer.strip();
        console->debug(fmt::format("Entrypoint after stripping is {:#x}", packer.get_entrypoint() ));
    }


    console->info("Setting entrypoint to 0");
    // Set entrypoint to 0
    packer.set_entrypoint(0);

    packer.build();
}
