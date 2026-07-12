
#include <meta>

#include "util/io/serializer_yaml.h"
#include "util/io/logger.h"
#include "util/system.h"
#include "util/crash_handler.h"
#include "util/util.h"


enum class endian {

    little = 0,
    big,
};


enum class field_type {

    byte_1 = 0,
    byte_2,
    byte_3,
    byte_4,
    CRC_8,
    CRC_16,
    CRC_32,
    CRC_64,
    frame,
};


struct field {

    std::string                 name{};
    std::string                 description{};
    field_type                  type = field_type::byte_1;
    std::string                 frame_name{};
};


struct frame_data {

    std::string                 description{};
    u8                          start_byte{};
    std::vector<field>          fields{};
};


enum class crc_algorithm {

    // CRC-8 variants
    CRC8 = 0,
    CRC8_MAXIM,
    CRC8_SMBUS,

    // CRC-16 variants
    XMODEM,
    CCITT,
    CCITT_FALSE,
    MODBUS,
    IBM,          // same as ARC
    MAXIM,
    USB,
    DNP,

    // CRC-32 variants
    CRC32,        // standard Ethernet/ZIP / ISO-HDLC
    CRC32_MPEG2,
    CRC32C,       // Castagnoli
    CRC32_BZIP2,
    CRC32_JAMCRC,

    // CRC-64 variants
    CRC64_ISO,
    CRC64_ECMA
};


struct crc_data {

    crc_algorithm               algorithm;       // e.g. "XMODEM"
    u32                         polynomial;      // could be 16-bit, but uint32_t covers all sizes
    u16                         initial_value;
    u16                         final_xor;
    bool                        reflect_in;
    bool                        reflect_out;
    std::vector<std::string>    over_fields;     // names of the fields to compute CRC over
    endian                      byte_order;      // how the CRC value is stored (little/big)
};


struct protocol {
    
    std::string                 name{};
    std::string                 description{};
    PE::version                 version{};
    endian                      default_endian = endian::little;

    frame_data                  physical_frame{};
};



int main() {

    PE::logger::init("[$B$T:$J$E] [$B$R $L$X $Q - $I:$P:$G$E] $C$Z", true, PE::util::get_executable_path() / "logs", 
        "protocol_engine.log", true);
    PE::logger::register_label_for_thread("main"); 
    PE::logger::set_buffer_threshold(PE::logger::severity::warn);
    PE::crash_handler::attach();
    PE::crash_handler::subscribe(PE::logger::shutdown);


    protocol prot{};
    std::unordered_map<std::string, frame_data> buffer{};
    buffer.emplace("set_buffer_size", frame_data{});
    buffer.emplace("false_error", frame_data{});

    bool success = false;
    PE::serializer::yaml("./protocol/PCP_def.yml", "protocol", PE::serializer::option::save, &success)
        .entry(KEY_VALUE(prot.name))
        .entry(KEY_VALUE(prot.description))
        .entry(KEY_VALUE(prot.version))
        .entry(KEY_VALUE(prot.default_endian))

        .unordered_map(KEY_VALUE(buffer),
            std::function<void(PE::serializer::yaml&, frame_data&)>(
                [&](PE::serializer::yaml& frame_y, frame_data& frame) {

                    frame_y.entry(KEY_VALUE(frame.description))
                        .entry(KEY_VALUE(frame.start_byte))
                        .vector(KEY_VALUE(frame.fields),
                            std::function<void(PE::serializer::yaml&, const u64)>(
                                [&](PE::serializer::yaml& field_y, const u64 x) {
                                    field_y.entry(KEY_VALUE(frame.fields[x].name))
                                            .entry(KEY_VALUE(frame.fields[x].description))
                                            .entry(KEY_VALUE(frame.fields[x].type))
                                            .entry(KEY_VALUE(frame.fields[x].frame_name));
                                }
                            )
                        );
                }
            )
        );


    ASSERT(success, "", "Failed to serialize protocol");
    LOG(trace, "Protocol:");
    LOG(trace, "  name           : {}", prot.name);
    LOG(trace, "  description    : {}", prot.description);
    LOG(trace, "  version        : {}", PE::util::to_string(prot.version));                // assumes PE::version is formattable
    LOG(trace, "  default_endian : {}", PE::util::enum_to_string(prot.default_endian));

    const auto& frame = prot.physical_frame;
    LOG(trace, "  physical_frame:");
    LOG(trace, "    start_byte   : 0x{:02X}", frame.start_byte);
    LOG(trace, "    fields       : {}", frame.fields.size());

    for (size_t i = 0; i < frame.fields.size(); ++i) {
        const auto& f = frame.fields[i];
        LOG(trace, "    field[{}]:", i);
        LOG(trace, "      name        : {}", f.name);
        LOG(trace, "      description : {}", f.description);
        LOG(trace, "      type        : {}", PE::util::enum_to_string(f.type));
        if (!f.frame_name.empty()) {
            LOG(trace, "      frame_name  : {}", f.frame_name);
        }
    }


    PE::crash_handler::detach();
    PE::logger::shutdown();
    return EXIT_SUCCESS;
}
