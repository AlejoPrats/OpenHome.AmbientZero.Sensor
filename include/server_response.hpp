#pragma once

#include <stddef.h>

// Forward declaration of the JSON parser types
struct JsonPair;
struct JsonValue;

// The protocol model: what the server sends back
struct ServerResponse {
    char deviceId[64];
    bool hasDeviceId;
    bool isLightEnabled;
};

// Parse the raw JSON into a ServerResponse struct
bool parse_server_response(const char* json, ServerResponse& out);
