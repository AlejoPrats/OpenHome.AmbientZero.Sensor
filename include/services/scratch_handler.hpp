#pragma once
#include <cstdint>

/**
 * @brief Minimal interface for managing RP2040 scratch registers.
 *
 * The RP2040 provides four 32‑bit scratch registers that retain their
 * values across soft resets and deep‑sleep wakeups. This class exposes
 * a simple API for reading and writing those registers in a structured
 * way.
 *
 * The provisioning and measurement flows use these registers to store
 * small pieces of state that must survive resets (e.g., chunk counters,
 * sleep flags, or multi‑step operation markers).
 *
 * @note All operations are static. No instance of ScratchHandler exists.
 */
class ScratchHandler
{
public:
    /**
     * @brief Logical indices for the four scratch registers.
     *
     * These map directly to the hardware registers:
     *  - CHUNKS:      Number of completed chunks in a multi‑step send
     *  - REMAINDER:   Remaining bytes or steps
     *  - LAST_CHUNK:  Marker indicating the final chunk was sent
     *  - BOOT_FLAG:   Stores a BootFlag value controlling next boot mode
     *
     * @note These meanings are project‑specific conventions. The hardware
     *       itself does not assign semantics to the scratch registers.
     */
    enum Index : uint32_t
    {
        CHUNKS      = 0,
        REMAINDER   = 1,
        LAST_CHUNK  = 2,
        BOOT_FLAG   = 3
    };

    static void set(Index idx, uint32_t value);
    static uint32_t get(Index idx);
    static void reset_all();
};

/**
 * @brief Semantic boot‑mode values stored in the BOOT_FLAG scratch register.
 */
enum class BootFlag : uint32_t
{
    NONE        = 0,
    DEEP_SLEEP  = 1,
    OTA_PENDING = 2
};

/**
 * @brief Writes a BootFlag value into the BOOT_FLAG scratch register.
 */
inline void set_boot_flag(BootFlag flag)
{
    ScratchHandler::set(
        ScratchHandler::BOOT_FLAG,
        static_cast<uint32_t>(flag));
}

/**
 * @brief Reads the current BootFlag value from the BOOT_FLAG scratch register.
 *
 * If the stored value is invalid (e.g., after brownout), returns BootFlag::NONE.
 */
inline BootFlag get_boot_flag()
{
    uint32_t raw = ScratchHandler::get(ScratchHandler::BOOT_FLAG);

    switch (raw)
    {
        case static_cast<uint32_t>(BootFlag::NONE):
        case static_cast<uint32_t>(BootFlag::DEEP_SLEEP):
        case static_cast<uint32_t>(BootFlag::OTA_PENDING):
            return static_cast<BootFlag>(raw);

        default:
            return BootFlag::NONE;
    }
}
