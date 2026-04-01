#include "services/scratch_handler.hpp"
#include "hardware/structs/watchdog.h"

void ScratchHandler::set(Index idx, uint32_t value) {
    watchdog_hw->scratch[static_cast<uint32_t>(idx)] = value;
}

uint32_t ScratchHandler::get(Index idx) {
    return watchdog_hw->scratch[static_cast<uint32_t>(idx)];
}

void ScratchHandler::resetAll() {
    set(CHUNKS, 0);
    set(REMAINDER, 0);
    set(LAST_CHUNK, 0);
    set(SHOULD_SLEEP, 0);
}
