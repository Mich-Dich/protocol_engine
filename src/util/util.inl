#pragma once


// FORWARD DECLARATIONS ================================================================================================

namespace PE::util {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // INTERNAL FUNCTION DECLARATION ===================================================================================

    // INTERNAL FUNCTION IMPLEMENTATION ================================================================================

    // TEMPLATE IMPLEMENTATION =========================================================================================

    template <typename E>
    requires std::is_enum_v<E>
    constexpr std::string_view enum_to_string(E value) {

        constexpr std::size_t N = std::meta::enumerators_of(^^E).size();
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            std::string_view result = "<unnamed>";
            ((value == [:std::meta::enumerators_of(^^E)[Is]:]
                ? (result = std::meta::identifier_of(std::meta::enumerators_of(^^E)[Is]), true)
                : false) || ...);
            return result;
        }(std::make_index_sequence<N>{});
    }


    template <typename E>
    requires std::is_enum_v<E>
    constexpr std::optional<E> string_to_enum(std::string_view str) {

        constexpr std::size_t N = std::meta::enumerators_of(^^E).size();
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            std::optional<E> result;
            ((str == std::meta::identifier_of(std::meta::enumerators_of(^^E)[Is])
                ? (result = [:std::meta::enumerators_of(^^E)[Is]:], true)
                : false) || ...);
            return result;
        }(std::make_index_sequence<N>{});
    }

    
    template<std::unsigned_integral T>
    FORCE_INLINE constexpr void set_bytes(std::vector<std::byte>& array, T value) noexcept {

        static_assert(sizeof(T) <= 8, "Only up to 64-bit unsigned integers are supported");

        if constexpr (std::endian::native == std::endian::little) {

            const std::byte* bytes = reinterpret_cast<const u8*>(&value);
            for (size_t index = 0; index < sizeof(T); index++)
                array.push_back(bytes[index]);

        } else {

            /** Store bytes in reverse order */
            for (size_t index = sizeof(T); index > 0; index--)
                array.push_back(static_cast<std::byte>((value >> ((index - 1) * 8)) & 0xFF));
        }
    }


    template<typename T> requires(std::is_arithmetic_v<T> && sizeof(T) <= 8)
    FORCE_INLINE_R constexpr T get_bytes(const std::vector<std::byte>& array, const size_t startIndex) {

        static_assert(sizeof(T) <= 8, "Only up to 64-bit unsigned integers are supported");

        const auto minSize = sizeof(T);
        if (startIndex + minSize > array.size())
            throw std::out_of_range("Not enough bytes in vector");

        T value = 0;
        if constexpr (std::endian::native == std::endian::little)
            std::memcpy(&value, array.data() + startIndex, sizeof(T));      /** Direct copy for little endian systems */

        else {

            /** Byte reversal for big endian systems */
            for (size_t i = 0; i < sizeof(T); ++i)
                value |= static_cast<T>(array[startIndex + i]) << (i * 8);
        }
        return value;
    }

    // TEMPLATE CLASS IMPLEMENTATION ===================================================================================

    // TEMPLATE CLASS PUBLIC ===========================================================================================

    // TEMPLATE CLASS PROTECTED ========================================================================================

    // TEMPLATE CLASS PRIVATE ==========================================================================================

}
