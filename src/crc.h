
#pragma once



// FORWARD DECLARATIONS ================================================================================================

namespace PE::CRC {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================
    
    enum class algorithm {
        // CRC-8 variants
        CRC8 = 0,
        CRC8_MAXIM,
        CRC8_SMBUS,

        // CRC-16 variants
        XMODEM,
        CCITT,        // CCITT (Kermit)
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

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    [[nodiscard]] constexpr u64 calculate(const algorithm algo, const std::span<std::byte>& data);
        
    // TEMPLATE DECLARATION ============================================================================================

    // Compile-time bit reflection utility
    template <typename T>
    constexpr T reflect(T val, const int width) noexcept;


    // Generic bit-by-bit CRC calculator (constexpr, works for any width)
    template <typename T, int width, T poly, T init, T xor_out, bool reflect_in, bool reflect_out>
    constexpr T compute_bitwise(std::span<const std::byte> data) noexcept;

    // CLASS DECLARATION ===============================================================================================

}

#include "crc.inl"
