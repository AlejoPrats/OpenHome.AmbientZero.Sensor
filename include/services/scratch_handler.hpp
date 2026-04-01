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
class ScratchHandler {
public:
    /**
     * @brief Logical indices for the four scratch registers.
     *
     * These map directly to the hardware registers:
     *  - CHUNKS:       Number of completed chunks in a multi‑step send
     *  - REMAINDER:    Remaining bytes or steps
     *  - LAST_CHUNK:   Marker indicating the final chunk was sent
     *  - SHOULD_SLEEP: Flag used to trigger deep sleep after reboot
     *
     * @note These meanings are project‑specific conventions. The hardware
     *       itself does not assign semantics to the scratch registers.
     */
    enum Index : uint32_t {
        CHUNKS = 0,
        REMAINDER = 1,
        LAST_CHUNK = 2,
        SHOULD_SLEEP = 3
    };

    /**
     * @brief Writes a 32‑bit value to the specified scratch register.
     */
    static void set(Index idx, uint32_t value);

    /**
     * @brief Reads the value stored in the specified scratch register.
     */
    static uint32_t get(Index idx);

    /**
     * @brief Clears all scratch registers to zero.
     *
     * Useful when starting a new flow or after completing a multi‑step
     * operation that no longer requires persisted state.
     */
    static void resetAll();
};
