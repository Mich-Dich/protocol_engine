
#include "util/pch.h"
#include "pwm_command_protocol.h"


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

    // CLASS PUBLIC ====================================================================================================

    std::optional<error_code> slave::processFrame(const std::vector<std::byte>& rawFrame) {

        // Minimum size: start(1) + devId(1) + length(1) + CRC(2) = 5
        if (rawFrame.size() < 5) return std::nullopt;

        if (rawFrame[0] != std::byte{0xAA}) return std::nullopt;

        uint8_t devId   = static_cast<uint8_t>(rawFrame[1]);
        uint8_t length  = static_cast<uint8_t>(rawFrame[2]);

        // Check total size
        if (rawFrame.size() != static_cast<size_t>(4 + length)) return std::nullopt; // 4 = start+devId+length+CRC(2)

        // Extract payload and CRC
        const std::byte* payloadBegin = rawFrame.data() + 3;
        std::vector<std::byte> payload(payloadBegin, payloadBegin + length);

        uint16_t receivedCrc = static_cast<uint16_t>(rawFrame[3 + length]) |
                            (static_cast<uint16_t>(rawFrame[4 + length]) << 8);

        // CRC over [length byte] + [payload]
        std::vector<uint8_t> crcBuf;
        crcBuf.reserve(1 + length);
        crcBuf.push_back(length);
        for (auto b : payload) crcBuf.push_back(static_cast<uint8_t>(b));
        uint16_t computedCrc = crc16_xmodem(crcBuf.data(), crcBuf.size());

        if (receivedCrc != computedCrc) {
            // CRC error – silently ignore frame
            return std::nullopt;
        }

        // Broadcast?
        bool isBroadcast = (devId == 0xFF);
        if (devId != m_deviceId && !isBroadcast) {
            // Not for us
            return std::nullopt;
        }

        // Parse inner packet
        if (payload.size() < 3) return std::nullopt; // minimum: cmdId + pwm_channel(2)

        uint8_t  cmdId       = static_cast<uint8_t>(payload[0]);
        uint16_t channelMask = static_cast<uint16_t>(payload[1]) |
                            (static_cast<uint16_t>(payload[2]) << 8);
        std::vector<std::byte> data(payload.begin() + 3, payload.end());

        auto innerResp = executeCommand(cmdId, channelMask, data);
        if (!innerResp.has_value()) {
            // No response needed (e.g. broadcast or internal error already handled)
            return std::nullopt;
        }

        // Build outer response frame
        return buildResponseFrame(innerResp.value());
    }

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

}
