#pragma once
#include <cstdint>

/**
 * @brief Provides cooperative, chunked sleeping for long delays.
 *
 * This class implements a sleep helper that breaks long delays into
 * smaller chunks, allowing the system to remain responsive and avoid
 * watchdog resets. It is intended for use in flows where blocking is
 * acceptable but must not freeze the system for extended periods.
 *
 * @note This function does not yield to any scheduler. It simply
 *       performs repeated short sleeps internally.
 */
class Sleeper
{
public:
    /**
     * @brief Sleeps for the specified duration in milliseconds.
     *
     * The delay is internally divided into smaller chunks to prevent
     * long blocking intervals. The exact chunk size is implementation‑
     * defined and chosen to balance responsiveness and simplicity.
     *
     * @param ms  Total duration to sleep.
     */
    void sleepChunkMs(uint32_t ms);
};
