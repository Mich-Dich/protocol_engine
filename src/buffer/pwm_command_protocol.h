
#pragma once



// FORWARD DECLARATIONS ================================================================================================

namespace PE::PCP {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    enum class error_code : u16 {

        ok                      = 0x0000,
        invalid_command         = 0x0001,
        invalid_parameter       = 0x0002,
        channel_unavailable     = 0x0003,
        crc_mismatch            = 0x0004,
        not_allowed_in_state    = 0x0005,
        general_failure         = 0xFFFF
    };

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    class slave {
    public:

        slave(const u64 ID);
    
        error_code process_frame(const std::vector<std::byte>& rawFrame);

    private:

        error_code is_usable_frame(const std::vector<std::byte>& raw_frame, const u8 length);
        error_code test_crc(const std::vector<std::byte>& raw_frame, const std::vector<std::byte>& payload, const u8 length);


        // messages that can come in
        error_code enable_save_mode(const bool master_to_slave, const u64 heartbeat_ms);
        error_code set_safe_value(const u16 safe_value);
        error_code ping();
        error_code set_default_value(const u16 default_value);
        error_code query_current_state(const bool master_to_slave, const u64 heartbeat_ms);
        error_code set_absolute_value(const u16 target_value, const u8 flags, const u16 duration_ms = 0);
        error_code oscillation(const u16 value_0, const u16 value_1, const u8 flags, 
            const u16 duration_ms = 0, const u16 hold_time_ms = 0, const u16 cycle_count = 0);
        error_code oscillation_stop(const bool return_to_default);
        error_code emergency_stop();

        // messages the slave can send out
        error_code acknowledge(const u16 ack_command_id);
        error_code error(const u16 failed_command_id, const u16 error_code);

        u64         m_device_id{};

    };

    // TEMPLATE DECLARATION ============================================================================================

    // CLASS DECLARATION ===============================================================================================

}
