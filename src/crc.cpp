
#include "util/pch.h"
#include "crc.h"


// FORWARD DECLARATIONS ================================================================================================

namespace PE::CRC {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // INTERNAL FUNCTION DECLARATION ===================================================================================

    // INTERNAL FUNCTION IMPLEMENTATION ================================================================================

    // FUNCTION IMPLEMENTATION =========================================================================================

    [[nodiscard]] constexpr u64 calculate(const algorithm algo, const std::span<std::byte>& data) {

        switch (algo) {
            // CRC-8
            case algorithm::CRC8:           return compute_bitwise<u8, 8, 0x07, 0x00, 0x00, false, false>(data);
            case algorithm::CRC8_MAXIM:     return compute_bitwise<u8, 8, 0x31, 0x00, 0x00, true, true>(data); // Dallas 1-Wire
            case algorithm::CRC8_SMBUS:     return compute_bitwise<u8, 8, 0x07, 0x00, 0x00, false, false>(data);

            // CRC-16
            case algorithm::XMODEM:         return compute_bitwise<u16, 16, 0x1021, 0x0000, 0x0000, false, false>(data);
            case algorithm::CCITT:          return compute_bitwise<u16, 16, 0x1021, 0x0000, 0x0000, true, true>(data); // CCITT (Kermit)
            case algorithm::CCITT_FALSE:    return compute_bitwise<u16, 16, 0x1021, 0xFFFF, 0x0000, false, false>(data);
            case algorithm::MODBUS:         return compute_bitwise<u16, 16, 0x8005, 0xFFFF, 0x0000, true, true>(data);
            case algorithm::IBM:            return compute_bitwise<u16, 16, 0x8005, 0x0000, 0x0000, true, true>(data); // ARC
            case algorithm::MAXIM:          return compute_bitwise<u16, 16, 0x8005, 0x0000, 0xFFFF, true, true>(data);
            case algorithm::USB:            return compute_bitwise<u16, 16, 0x8005, 0xFFFF, 0xFFFF, true, true>(data);
            case algorithm::DNP:            return compute_bitwise<u16, 16, 0x3D65, 0x0000, 0xFFFF, true, true>(data);

            // CRC-32
            case algorithm::CRC32:          return compute_bitwise<u32, 32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true>(data); // Ethernet / ZIP / ISO-HDLC
            case algorithm::CRC32_MPEG2:    return compute_bitwise<u32, 32, 0x04C11DB7, 0xFFFFFFFF, 0x00000000, false, false>(data);
            case algorithm::CRC32C:         return compute_bitwise<u32, 32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true, true>(data); // Castagnoli
            case algorithm::CRC32_BZIP2:    return compute_bitwise<u32, 32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, false, false>(data);
            case algorithm::CRC32_JAMCRC:   return compute_bitwise<u32, 32, 0x04C11DB7, 0xFFFFFFFF, 0x00000000, true, true>(data);

            // CRC-64
            case algorithm::CRC64_ISO:      return compute_bitwise<u64, 64, 0x000000000000001B, 0xFFFFFFFFFFFFFFFF, 
                0xFFFFFFFFFFFFFFFF, true, true>(data);

            case algorithm::CRC64_ECMA:     return compute_bitwise<u64, 64, 0x42F0E1EBA9EA3693, 0x0000000000000000,
                0x0000000000000000, false, false>(data); // ECMA-182
        }
        
        // Should never be reached
        return 0;
    }

    // CLASS IMPLEMENTATION ============================================================================================

    // CLASS PUBLIC ====================================================================================================

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

}
