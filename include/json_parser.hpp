#pragma once
#include <stddef.h>

// ------------------------------------------------------------
// Basic key/value slice extracted from JSON
// ------------------------------------------------------------
struct JsonPair {
    const char* key;
    const char* valueStart;
    size_t valueLength;
};

// ------------------------------------------------------------
// Value type classification
// ------------------------------------------------------------
enum class JsonType {
    STRING,
    BOOLEAN,
    NUMBER,
    NULLTYPE,
    UNKNOWN
};

struct JsonValue {
    JsonType type;
    const char* start;
    size_t length;
};

// ------------------------------------------------------------
// API declarations
// ------------------------------------------------------------

// Extract top-level "key": value pairs
size_t parse_json_pairs(const char* json, JsonPair* out, size_t maxPairs);

// Lookup a key and classify its value
bool json_get(const JsonPair* pairs, size_t count,
              const char* key, JsonValue& out);

// Typed getters
bool getString(const JsonValue& v, char* out, size_t maxLen);
bool getBool(const JsonValue& v, bool& out);
bool getInt(const JsonValue& v, int& out);
