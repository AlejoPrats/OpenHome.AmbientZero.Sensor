#pragma once

/**
 * @brief Initializes all low‑level hardware subsystems.
 *
 * This function performs the minimal set of hardware bring‑up steps
 * required before any drivers, services, or domain flows can operate.
 * Typical responsibilities include:
 * - initializing the RP2040 peripherals
 * - preparing GPIOs and board‑level hardware
 * - enabling the CYW43 WiFi chip (when required)
 *
 * @note This function centralizes all hardware initialization to keep
 *       the rest of the system deterministic and free of side effects.
 *       Higher‑level modules must never perform their own hardware
 *       bring‑up.
 *
 * @warning This function must be called exactly once at boot, before
 *          any drivers or flows are used.
 */
void init_hardware();
