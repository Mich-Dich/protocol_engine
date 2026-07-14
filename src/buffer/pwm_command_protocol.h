
#pragma once



// FORWARD DECLARATIONS ================================================================================================

namespace PE::PCP {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    enum class error_code : u16 {

        OK                      = 0x0000,
        INVALID_COMMAND         = 0x0001,
        INVALID_PARAMETER       = 0x0002,
        CHANNEL_UNAVAILABLE     = 0x0003,
        CRC_MISMATCH            = 0x0004,
        NOT_ALLOWED_IN_STATE    = 0x0005,
        GENERAL_FAILURE         = 0xFFFF

    };

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    class slave {
    public:

        slave(const u64 ID);
    
        std::optional<error_code> proccess_frame(const std::vector<std::byte>& frame);

    private:

        void onSetPWM(const u8 channel, const u16 value);
        void onStopActions(const u8 channel);
        void onStartLerp(const u8 channel, const u16 target, const u16 durationMs);
        void onStartOscillation(const u8 channel, const u16 value0, const u16 value1, const u16 durationMs, const u16 holdMs, const u16 cycleCount);
        void onStopOscillation(const u8 channel);
        void onEnterSafeState();
        void onExitSafeState();

    };

    // TEMPLATE DECLARATION ============================================================================================

    // CLASS DECLARATION ===============================================================================================

}
