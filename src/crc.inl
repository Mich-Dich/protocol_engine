#pragma once


// FORWARD DECLARATIONS ================================================================================================

namespace PE::crc {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // INTERNAL FUNCTION DECLARATION ===================================================================================

    // INTERNAL FUNCTION IMPLEMENTATION ================================================================================

    // TEMPLATE IMPLEMENTATION =========================================================================================

    // Compile-time bit reflection utility
    template <typename T>
    constexpr T reflect(T val, const int width) noexcept {

        T result = 0;
        for (int i = 0; i < width; ++i)
            if (val & (T{1} << i))
                result |= T{1} << (width - 1 - i);

        return result;
    }


    // Generic bit-by-bit CRC calculator (constexpr, works for any width)
    template <typename T, int width, T poly, T init, T xor_out, bool reflect_in, bool reflect_out>
    constexpr T compute_bitwise(std::span<const std::byte> data) noexcept {

        T crc = init;
        constexpr T msb_mask = T{1} << (width - 1);
        constexpr T reversed_poly = reflect(poly, width);

        for (auto b : data) {
            if constexpr (reflect_in) {
                // Reflect input byte, then process LSB first
                T byte_val = reflect(static_cast<T>(b), 8);
                crc ^= byte_val;
                for (int i = 0; i < 8; ++i) {
                    if (crc & 1)
                        crc = (crc >> 1) ^ reversed_poly;
                    else
                        crc >>= 1;
                }
        
            } else {
                
                // Process MSB first
                T byte_val = static_cast<T>(b);
                crc ^= (byte_val << (width - 8));
                for (int i = 0; i < 8; ++i) {
                    if (crc & msb_mask)
                        crc = (crc << 1) ^ poly;
                    else
                        crc <<= 1;
                }
            }
        }

        if constexpr (reflect_out)
            crc = reflect(crc, width);

        return crc ^ xor_out;
    }

    // TEMPLATE CLASS IMPLEMENTATION ===================================================================================

    // TEMPLATE CLASS PUBLIC ===========================================================================================

    // TEMPLATE CLASS PROTECTED ========================================================================================

    // TEMPLATE CLASS PRIVATE ==========================================================================================

}
