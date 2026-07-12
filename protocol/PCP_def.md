
# PWM Command Protocol (PCP)

This protocol allows a master to control multiple PWM channels on a slave MCU.
Communication uses a **two‑level packet structure**:

- **Outer frame** – device addressing, byte boundaries, and error detection.
- **Inner packet** – command ID, target channel(s), and command‑specific data.

## Outer Frame (Low‑Level Transport)

| Byte offset | Field    | Size | Description                                                                   |
|-------------|----------|------|-------------------------------------------------------------------------------|
| 0           | Start    | 1    | `0xAA`                                                                        |
| 1           | Device ID| 1    | Target device ID (`0x00` = master, `0xFF` = broadcast)                                         |
| 2           | Length   | 1    | Number of bytes that follow – the **entire inner packet** (see below)         |
| 3 … n+2     | Payload  | n    | The inner packet (command ID + channel bitmask + data)                        |
| n+3 … n+4   | CRC16    | 2    | CRC‑16/XMODEM over the **Length** byte and the whole **Payload** (little‑endian) |

- **No escaping** – a `0xAA` inside the inner packet or CRC does not cause false synchronisation because `Length` tells the receiver exactly where the CRC ends.
- CRC polynomial: `0x1021` (XMODEM).
- Total frame size = `1 (Start) + 1 (Device ID) + 1 (Length) + Length + 2 (CRC)`.

## Inner Packet (Command Layer)

The outer frame’s **Payload** contains the following fixed header followed by variable data:

| Byte offset | Field        | Size | Description                                                                 |
|-------------|--------------|------|-----------------------------------------------------------------------------|
| 0           | Command ID   | 1    | Defines the command (see list below)                                        |
| 1‑2         | pwm_channel  | 2    | Bitmask of channels the command applies to (little‑endian). Set to `0x0000` if the command is system‑wide or irrelevant. |
| 3 … m+2     | Data         | m    | Command‑specific parameters. Size = `Length - 3` bytes.                     |

The inner packet **always** starts with a 1‑byte command ID and a 2‑byte channel bitmask.
The rest of the data is parsed according to the specific command.

## Command Definitions

### Master‑to‑Slave Commands

#### `enable_save_mode` (0x01)  
**Response:** `acknowledge` (0x0D)  

Enables or disables the keep‑alive safety mechanism.  

After power‑up, keep‑alive is **disabled** (no timeout).  
When `enable_save_mode` is enabled, the MCU starts a timer that is reset by **any valid command**.  

If the timer expires, the MCU immediately enters **safe state**:  
- Stop all lerps and oscillations.  
- Set every PWM output to its **safe value** (default 0 if never set with `set_safe_value`).  

The MCU remains in a safe state until it receives a new `enable_save_mode` (re‑enable) or a hardware reset.

| Field                 | Type      | Notes                                |
|-----------------------|-----------|--------------------------------------|
| use_keep_alive_mode   | bool (u8) | `1` = enable, `0` = disable          |
| heartbeat_ms          | u16       | Maximum silence interval in milliseconds (ignored if disabled) |

---

#### `set_safe_value` (0x02)  
**Response:** `acknowledge`  

Defines the PWM value that the selected channels shall adopt when the system enters the **safe state** (triggered by keep‑alive timeout, for example).  
If a channel has never received this command, its safe value defaults to `0x0000` (output off).

| Field | Type | Notes                             |
|-------|------|-----------------------------------|
| value | u16  | PWM value to use during safe state |

---

#### `ping` (0x04)  
**Response:** `acknowledge`  

Master tells the MCU it is still alive and checks whether the MCU is responsive.  
Does not modify any PWM state.

**Data fields:** none  

---

#### `set_default_value` (0x06)  
**Response:** `acknowledge`  

Sets the default value of the selected PWM channels.  
This value is used as a return point by commands such as `oscillation_stop` when `return_to_default` is true.

| Field         | Type | Notes                       |
|---------------|------|-----------------------------|
| default_value | u16  | Default return‑point value  |

---

#### `query_current_state` (0x07)  
**Response:** `current_state_response` (0x0F)  

The master requests the current operational state of the channels whose bits are set in `pwm_channel`.  
The response contains per‑channel details (current value, active command, etc.).

**Data fields:** none  

---

#### `set_absolute_value` (0x08)  
**Response:** `acknowledge`  

Instructs the selected channels to move to a target PWM value.  

**Streaming behaviour:**  
When a new `set_absolute_value` arrives for a channel that is already transitioning (lerp), the **active target is immediately replaced**. The transition continues smoothly from the *current actual position* to the new target over the specified duration. This gives real‑time “streaming” without any queue.

| Field | Type | Notes                                  |
|-------|------|----------------------------------------|
| value | u16  | Target PWM value                       |
| flags | u8   | Bit 0: `duration_ms` follows           |

**Optional fields:**

| Field        | Type | Default | Notes |
|--------------|------|---------|-------|
| duration_ms  | u16  | 0       | Lerp time in milliseconds (if absent, the change is immediate) |

---

#### `oscillation` (0x0A)  
**Response:** `acknowledge`  

Starts an endless oscillation between two PWM values on the selected channels.  
The oscillation runs **forever** until explicitly stopped by `oscillation_stop` or `emergency_stop`.  

If the optional `cycle_count` is provided, the oscillation automatically stops after that many cycles without requiring a stop command.

| Field   | Type | Notes                                         |
|---------|------|-----------------------------------------------|
| value_0 | u16  | First PWM value (position)                    |
| value_1 | u16  | Second PWM value (position)                   |
| flags   | u8   | Bit 0: `duration_ms`, Bit 1: `hold_time_ms`, Bit 2: `cycle_count` |

**Optional fields:**

| Field        | Type | Default | Notes |
|--------------|------|---------|-------|
| duration_ms  | u16  | 0       | Lerp time between values                           |
| hold_time_ms | u16  | 0       | Dwell time at each value before moving to the next |
| cycle_count  | u16  | 0       | Number of full cycles before auto‑stop             |

---

#### `oscillation_stop` (0x0B)  
**Response:** `acknowledge`  

Stops any active oscillation on the selected channels.  
After stopping, the channel behaviour is determined by `return_to_default`:

- `1` → PWM goes to the channel’s default value (set by `set_default_value`, or 0 if never set).
- `0` → PWM stays at its current position.

| Field             | Type      | Notes                                 |
|-------------------|-----------|---------------------------------------|
| return_to_default | bool (u8) | `1` = go to default; `0` = stay       |

---

#### `emergency_stop` (0x0C)  
**Response:** `acknowledge`  

Immediately halts **all** ongoing timed actions (lerp, oscillation) on the selected channels.  
The PWM output freezes at its current value. No return to default occurs.

**Data fields:** none  

---

### Slave‑to‑Master Responses

Responses also use the same inner‑packet structure, with the `pwm_channel` field set to `0x0000` when the message is not channel-specific.

#### `acknowledge` (0x0D)  
The MCU confirms that a command was **received and parsed without errors**.  
For long‑running commands (like `oscillation`), this means the command was accepted and started, not that the action has completed.

| Field       | Type | Notes                      |
|-------------|------|----------------------------|
| command_id  | u16  | ID of the acknowledged command |

---

#### `error` (0x0E)  
The MCU reports that the last command could not be processed correctly.

| Field       | Type | Notes                          |
|-------------|------|--------------------------------|
| command_id  | u16  | ID of the command that failed  |
| error_code  | u16  | Error code (see table below)   |

---

#### `current_state_response` (0x0F)  
Sent in reply to `query_current_state`.  
The response reports the current state of every channel whose bit was set in the request’s `pwm_channel`.

| Field       | Type     | Notes                                                    |
|-------------|----------|----------------------------------------------------------|
| error_code  | u16      | Overall error code (0 on success)                        |
| [channel_data] | repeated | One block per set bit in the request’s `pwm_channel`, processed LSB first |

**Per‑channel data block:**

| Field           | Type | Notes                                                     |
|-----------------|------|-----------------------------------------------------------|
| current_value   | u16  | Current PWM output value                                  |
| default_value   | u16  | Stored default value (as set by `set_default_value`)      |
| current_command | u16  | ID of the command currently active on this channel (`0x00` if idle) |

If `current_command == 0x0A` (oscillation), the following additional fields are appended:

| Field   | Type | Notes                       |
|---------|------|-----------------------------|
| value_0 | u16  | First oscillation value     |
| value_1 | u16  | Second oscillation value    |

---

## Error Codes

| Code   | Meaning                                     |
|--------|---------------------------------------------|
| 0x0001 | Invalid command ID                          |
| 0x0002 | Invalid parameter (e.g., value out of range)|
| 0x0003 | Channel(s) not available                    |
| 0x0004 | CRC mismatch                                |
| 0x0005 | Command not allowed in current state        |
| 0xFFFF | General failure                             |

---

## Keep‑Alive Behaviour (Summary)

- After power‑up, keep‑alive is **disabled** (no timeout).
- `enable_save_mode` with `use_keep_alive_mode = true` enables the timer; **any valid command** resets it.
- If the timer expires, the MCU enters **safe state**:
  - Stop all timed actions.
  - Set each PWM channel to its safe value (default `0x0000`).
- The MCU leaves safe state only when it receives a new `enable_save_mode` (enable) or a hardware reset.










<!-- 

commands in code:


// add const input parameter for each parameter in command def (optional parameter should be with default values)
PCP::error_code function_name(/*parameter...*/);

-->
