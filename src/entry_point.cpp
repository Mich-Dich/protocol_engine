
#include <meta>

#include "util/io/logger.h"
#include "util/system.h"
#include "util/crash_handler.h"
#include "protocol_engine.h"
#include "util/argument_parser.h"


// FORWARD DECLARATIONS ================================================================================================

// CONSTANTS ===========================================================================================================

// Define expected arguments
const std::vector<PE::argument_parser::argument_spec> specs =
{
    {
        .name = "protocol_def_path",
        .short_name = "p",
        .required = true,
        .positional = true,
        .position = 0,
        .type = "path",
        .default_value = std::filesystem::path{},
        .help_text = "Path to the protocol definition file"
    },
    {
        .name = "output_path",
        .short_name = "o",
        .required = false,
        .positional = false,
        .position = 0,
        .type = "path",
        .default_value = std::filesystem::path{},
        .help_text = "output path for creating the generated code"
    }
};

// MACROS ==============================================================================================================

// TYPES ===============================================================================================================

// STATIC VARIABLES ====================================================================================================

// INTERNAL FUNCTION DECLARATION =======================================================================================

// INTERNAL FUNCTION IMPLEMENTATION ====================================================================================

int main(int argc, char* argv[]) {

    PE::logger::init("[$B$T:$J$E] [$B$R $L$X $Q - $I:$P:$G$E] $C$Z", true, PE::util::get_executable_path() / "logs", 
        "protocol_engine.log", true);
    PE::logger::register_label_for_thread("main"); 
    PE::logger::set_buffer_threshold(PE::logger::severity::warn);
    PE::crash_handler::attach();
    PE::crash_handler::subscribe(PE::logger::shutdown);

    std::error_code error{};
    const PE::argument_parser::parsed_result parsed = PE::argument_parser::parse(specs, argc, argv, error);
    VALIDATE(!error, PE::logger::shutdown(); PE::crash_handler::detach(); return EXIT_FAILURE, 
    "", "Argument error [{}]", error.message())

    PE::protocol_engine prot(PE::argument_parser::get<std::filesystem::path>(parsed, "protocol_def_path"), 1);
    prot.print_protocol_def();


    std::filesystem::path output_path = PE::argument_parser::get<std::filesystem::path>(parsed, "output_path");
    if (!output_path.empty()) {

        VALIDATE(PE::vfs::is_directory(output_path, error) && !error, 
            PE::logger::shutdown(); PE::crash_handler::detach(); return EXIT_FAILURE, "", "Provided path is not a directory")
    
        PE::vfs::create_directories(output_path, error);
        VALIDATE(!error, PE::logger::shutdown(); PE::crash_handler::detach(); return EXIT_FAILURE, 
            "", "Failed to ensure Directory exists")

        prot.create_cpp_code(output_path);
    }

    
    PE::crash_handler::detach();
    PE::logger::shutdown();
    return EXIT_SUCCESS;
}

// FUNCTION IMPLEMENTATION =============================================================================================

// CLASS IMPLEMENTATION ================================================================================================

// CLASS PUBLIC ========================================================================================================

// CLASS PROTECTED =====================================================================================================

// CLASS PRIVATE =======================================================================================================
