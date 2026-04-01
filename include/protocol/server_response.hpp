#pragma once

#include <stddef.h>

// Forward declaration of the JSON parser types
struct JsonPair;
struct JsonValue;

/**
 * @brief Parsed representation of the backend's JSON response.
 *
 * This structure holds the minimal fields the device cares about when
 * receiving a response from the server after sending a measurement.
 *
 * Fields:
 *  - deviceId:      Null‑terminated string containing the assigned ID.
 *  - hasDeviceId:   True if the JSON contained a "deviceId" field.
 *  - isLightEnabled: True if the server requested the LED signaling mode.
 *
 * @note All fields are optional in the incoming JSON. Missing fields
 *       leave the corresponding flags set to false.
 */
struct ServerResponse {
    char deviceId[64];
    bool hasDeviceId;
    bool isLightEnabled;
};

/**
 * @brief Parses a raw JSON string into a ServerResponse structure.
 *
 * This function extracts only the fields relevant to the device:
 *  - "deviceId": string
 *  - "light": boolean
 *
 * Any other fields in the JSON are ignored. The parser is tolerant of
 * missing fields and malformed values; in such cases, the corresponding
 * flags in `out` remain false.
 *
 * @param json  Null‑terminated JSON string received from the server.
 * @param out   Output structure to populate.
 *
 * @return true if the JSON was syntactically valid enough to parse.
 */
bool parse_server_response(const char* json, ServerResponse& out);
