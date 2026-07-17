
#pragma once

#include <meta>


// FORWARD DECLARATIONS ================================================================================================

namespace PE::util {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    // TEMPLATE DECLARATION ============================================================================================

    template <typename E>
    requires std::is_enum_v<E>
    constexpr std::string_view enum_to_string(E value);


    // Convert a STRING to an optional enum value.
    template <typename E>
    requires std::is_enum_v<E>
    constexpr std::optional<E> string_to_enum(std::string_view str);


    template<std::unsigned_integral T>
    FORCE_INLINE constexpr void set_bytes(std::vector<std::byte>& array, T value) noexcept;


    template<typename T> requires(std::is_arithmetic_v<T> && sizeof(T) <= 8)
    FORCE_INLINE_R constexpr T get_bytes(const std::vector<std::byte>& array, const size_t startIndex);

    // CLASS DECLARATION ===============================================================================================

}

#include "util.inl"
