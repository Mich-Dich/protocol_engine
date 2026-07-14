
#include "util/pch.h"
#include "protocol_engine.h"
#include "util/io/serializer_yaml.h"


// FORWARD DECLARATIONS ================================================================================================

namespace PE {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // INTERNAL FUNCTION DECLARATION ===================================================================================

    // INTERNAL FUNCTION IMPLEMENTATION ================================================================================

    // FUNCTION IMPLEMENTATION =========================================================================================

    // CLASS IMPLEMENTATION ============================================================================================

    protocol_engine::protocol_engine(const std::filesystem::path& config_file, const u64 self_id) {

        VALIDATE(load_protocol_def(config_file), return, "", "Failed to load protocol")
        VALIDATE(!m_protocol.start_frame.empty(), return, "", "Failed to load starter frame, cant do any work")

    }

    // CLASS PUBLIC ====================================================================================================

    void protocol_engine::print_protocol_def() {

        LOG(trace, "Protocol:");
        LOG(trace, "  name             : {}", m_protocol.name);
        LOG(trace, "  description      : {}", m_protocol.description);
        LOG(trace, "  version          : {}", util::to_string(m_protocol.version));
        LOG(trace, "  default_endian   : {}", util::enum_to_string(m_protocol.default_endian));
        LOG(trace, "  start_frame      : {}", m_protocol.start_frame);

        LOG(trace, "  frames:");
        for (const auto& [frame_name, frame] : m_protocol.frames) {
            LOG(trace, "    [{}]", frame_name);
            LOG(trace, "      description       : {}", frame.description);
            LOG(trace, "      response_frame    : {}", frame.response_frame);
            LOG(trace, "      id                : {}", frame.id);
            LOG(trace, "      use_crc           : {}", frame.use_crc);
            if (frame.use_crc) {
                // Build over_fields string manually
                std::string over_fields_str;
                for (size_t i = 0; i < frame.crc_settings.over_fields.size(); ++i) {
                    if (i > 0) over_fields_str += ", ";
                    over_fields_str += frame.crc_settings.over_fields[i];
                }
                LOG(trace, "      crc_settings:");
                LOG(trace, "        algorithm     : {}", util::enum_to_string(frame.crc_settings.algorithm));
                LOG(trace, "        polynomial    : 0x{:08X}", frame.crc_settings.polynomial);
                LOG(trace, "        initial_value : 0x{:04X}", frame.crc_settings.initial_value);
                LOG(trace, "        final_xor     : 0x{:04X}", frame.crc_settings.final_xor);
                LOG(trace, "        reflect_in    : {}", frame.crc_settings.reflect_in);
                LOG(trace, "        reflect_out   : {}", frame.crc_settings.reflect_out);
                LOG(trace, "        byte_order    : {}", util::enum_to_string(frame.crc_settings.byte_order));
                LOG(trace, "        over_fields   : {}", over_fields_str);
            }
            LOG(trace, "      fields:");
            for (size_t i = 0; i < frame.fields.size(); ++i) {
                const auto& field = frame.fields[i];
                LOG(trace, "        [{}] {}", i, field.name);
                LOG(trace, "          description : {}", field.description);
                LOG(trace, "          type        : {}", util::enum_to_string(field.type));
                if (field.type == field_type::frame) {
                    LOG(trace, "          frame_name  : {}", field.frame_name);
                }
            }
        }
    }

        
    bool protocol_engine::load_protocol_def(const std::filesystem::path& config_file) {

        bool success = false;
        serializer::yaml (config_file, "protocol", serializer::option::load, &success)
            .entry(KEY_VALUE(m_protocol.name))
            .entry(KEY_VALUE(m_protocol.description))
            .entry(KEY_VALUE(m_protocol.version))
            .entry(KEY_VALUE(m_protocol.default_endian))
            .entry(KEY_VALUE(m_protocol.start_frame))
            .unordered_map(KEY_VALUE(m_protocol.frames),
                std::function<void(serializer::yaml&, frame_data&)>(
                    [&](serializer::yaml& frame_y, frame_data& frame) {

                        frame_y.entry(KEY_VALUE(frame.description))
                            .entry(KEY_VALUE(frame.description))
                            .entry(KEY_VALUE(frame.response_frame))
                            .entry(KEY_VALUE(frame.id))

                            .vector(KEY_VALUE(frame.fields),
                                std::function<void(serializer::yaml&, const u64)>(
                                    [&](serializer::yaml& field_y, const u64 x) {
                                        
                                        frame_y.entry(KEY_VALUE(frame.fields[x].name))
                                            .entry(KEY_VALUE(frame.fields[x].description))
                                            .entry(KEY_VALUE(frame.fields[x].type));

                                            switch (frame.fields[x].type)
                                            {
                                                // Parameter Data fields, used as parameter in function call -> dont need to deserialize
                                                case field_type::u_8:               break;
                                                case field_type::u_16:              break;
                                                case field_type::u_32:              break;
                                                case field_type::u_64:              break;

                                                case field_type::i_8:               break;
                                                case field_type::i_16:              break;
                                                case field_type::i_32:              break;
                                                case field_type::i_64:              break;

                                                case field_type::boolean:           break;


                                                case field_type::data:              break;
                                                case field_type::fixed_1_byte:      break;
                                                case field_type::fixed_2_byte:      break;
                                                case field_type::fixed_3_byte:      break;
                                                case field_type::fixed_4_byte:      break;
                                                case field_type::self_ID:           break;
                                                case field_type::length_1_byte:     break;
                                                
                                                case field_type::CRC:    { 

                                                    frame.use_crc = true;
                                                    field_y.sub_section("crc", [&](serializer::yaml& y_crc) {

                                                        y_crc.entry(KEY_VALUE(frame.crc_settings.algorithm))
                                                            .entry(KEY_VALUE(frame.crc_settings.polynomial))
                                                            .entry(KEY_VALUE(frame.crc_settings.initial_value))
                                                            .entry(KEY_VALUE(frame.crc_settings.final_xor))
                                                            .entry(KEY_VALUE(frame.crc_settings.reflect_in))
                                                            .entry(KEY_VALUE(frame.crc_settings.reflect_out))
                                                            .entry(KEY_VALUE(frame.crc_settings.over_fields))
                                                            .entry(KEY_VALUE(frame.crc_settings.byte_order));
                                                    });

                                                } break;

                                                case field_type::frame:     {

                                                    // if [type] is [field_type::frame] -> then [frame_name] holds the name of the frame it points to
                                                    std::string frame_name{};
                                                    field_y.entry(KEY_VALUE(frame_name));
                                                    
                                                } break;

                                                default:                    break;
                                            }
                                            
                                    }
                                )
                            );
                    }
                )
            );

        ASSERT(success, "", "Failed to serialize protocol");
        return success;    
    }


    void protocol_engine::create_cpp_code(const std::filesystem::path& output_path) {

        
    }

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

}
