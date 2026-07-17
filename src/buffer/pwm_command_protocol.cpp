
#include "util/pch.h"
#include "pwm_command_protocol.h"

#include "util/util.h"
#include "crc.h"


// FORWARD DECLARATIONS ================================================================================================

namespace PE::PCP {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // INTERNAL FUNCTION DECLARATION ===================================================================================

    // INTERNAL FUNCTION IMPLEMENTATION ================================================================================

    // FUNCTION IMPLEMENTATION =========================================================================================

    // CLASS IMPLEMENTATION ============================================================================================

    slave::slave(const u64 ID) 
        : m_device_id(ID) {


    }

    // CLASS PUBLIC ====================================================================================================

    std::optional<error_code> slave::process_frame(const std::vector<std::byte>& raw_frame) {

        const u8 dev_id = PE::util::get_bytes<u8>(raw_frame, 1);
        const u8 length = PE::util::get_bytes<u8>(raw_frame, 2);
        VALIDATE(is_usable_frame(raw_frame, length), return std::nullopt, "", "Frame is not usable")

        // Extract payload and CRC
        const std::byte* payload_begin = raw_frame.data() + 3;
        const std::vector<std::byte> payload(payload_begin, payload_begin + length);

        test_crc(raw_frame, payload, length);




        // Broadcast?
        const bool is_broadcast = (dev_id == 0xFF);
        if (dev_id != m_device_id && !is_broadcast) {
            // Not for us
            return std::nullopt;
        }

        // Parse inner packet
        if (payload.size() < 3) return std::nullopt; // minimum: cmd_id + pwm_channel(2)

        const u8 cmd_id = static_cast<u8>(payload[0]);
        const u16 channel_mask = static_cast<u16>(payload[1]) | (static_cast<u16>(payload[2]) << 8);
        const std::vector<std::byte> data(payload.begin() + 3, payload.end());

        auto inner_resp = executeCommand(cmd_id, channel_mask, data);
        if (!inner_resp.has_value()) {
            // No response needed (e.g. broadcast or internal error already handled)
            return std::nullopt;
        }

        // Build outer response frame
        return buildResponseFrame(inner_resp.value());
    }

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

    error_code slave::is_usable_frame(const std::vector<std::byte>& raw_frame, const u8 length) {

        // Minimum size: start(1) + dev_id(1) + length(1) + CRC(2) = 5
        if (raw_frame.size() < 5)                                   return std::nullopt;
        if (raw_frame[0] != std::byte{0xAA})                        return std::nullopt;
        if (raw_frame.size() != static_cast<size_t>(4 + length))    return std::nullopt; //  Check total size
        //   4 = start + dev_id + length + CRC(2)
        
        return {};
    }


    error_code slave::test_crc(const std::vector<std::byte>& raw_frame, const std::vector<std::byte>& payload,
        const u8 length) {
        
        const u16 received_crc = PE::util::get_bytes<u16>(raw_frame, 3 + length);

        // CRC over [length byte] + [payload]
        std::vector<u8> crc_buf;
        crc_buf.reserve(1 + length);
        crc_buf.push_back(length);
        for (auto b : payload) 
            crc_buf.push_back(static_cast<u8>(b));

        const u16 computed_crc = PE::CRC::calculate(PE::CRC::algorithm::XMODEM, crc_buf);
        if (received_crc != computed_crc) {
            // CRC error – silently ignore frame
            return error_code::crc_mismatch;
        }
        
        return error_code::ok;
    }

}
