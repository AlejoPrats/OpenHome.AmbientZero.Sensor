#pragma once
#include <cstdint>

class ScratchHandler {
public:
    enum Index : uint32_t {
        CHUNKS = 0,
        REMAINDER = 1,
        LAST_CHUNK = 2,
        SHOULD_SLEEP = 3
    };

    static void set(Index idx, uint32_t value);
    static uint32_t get(Index idx);
    static void resetAll();
};
