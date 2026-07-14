
#pragma once



// FORWARD DECLARATIONS ================================================================================================

namespace PE {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    enum class endian {

        little = 0,
        big,
    };


    enum class field_type {

        u_8 = 0,
        u_16,
        u_32,
        u_64,

        i_8,
        i_16,
        i_32,
        i_64,

        boolean,
        
        data,
        fixed_1_byte,
        fixed_2_byte,
        fixed_3_byte,
        fixed_4_byte,
        self_ID,
        length_1_byte,
        CRC,
        frame,
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


    struct field {

        std::string                 name{};
        std::string                 description{};
        field_type                  type = field_type::u_8;
        std::string                 frame_name{};
    };


    struct crc {

        crc_algorithm               algorithm;       // e.g. "XMODEM"
        u32                         polynomial;      // could be 16-bit, but u3 covers all sizes
        u16                         initial_value;
        u16                         final_xor;
        bool                        reflect_in;
        bool                        reflect_out;
        std::vector<std::string>    over_fields;     // names of the fields to compute CRC over
        endian                      byte_order;      // how the CRC value is stored (little/big)
    };


    struct frame_data {

        std::string                 description{};
        std::string                 response_frame{};
        u64                         id{};
        bool                        use_crc = false;
        crc                         crc_settings{};
        std::vector<field>          fields{};
    };


    struct protocol {
        
        std::string                         name{};
        std::string                         description{};
        PE::version                        version{};
        endian                              default_endian = endian::little;
        std::string                         start_frame{};

        std::unordered_map<std::string, frame_data>   frames{};
    };


    struct field_value : std::variant<
        std::monostate,
        bool,
        u8, u16, u32, u64,
        i8, i16, i32, i64,
        std::vector<std::byte>,                            // raw bytes
        std::vector<field_value>,                          // dynamic array of variant objects
        std::unordered_map<std::string, field_value>       // nested map
    > {
        using variant::variant;
    };

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    // TEMPLATE DECLARATION ============================================================================================

    // CLASS DECLARATION ===============================================================================================

    class protocol_engine {
    public:

        protocol_engine(const std::filesystem::path& config_file, const u64 self_id);

        
        void print_protocol_def();

        
        void create_cpp_code(const std::filesystem::path& output_path);
        
    private:

        bool load_protocol_def(const std::filesystem::path& config_file);

        protocol        m_protocol{};

    };

}
