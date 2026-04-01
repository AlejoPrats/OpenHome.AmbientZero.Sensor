#pragma once
#include <stddef.h>

// ------------------------------------------------------------
// Basic key/value slice extracted from JSON
// ------------------------------------------------------------

/**
 * @brief Represents a top‑level "key": value pair extracted from JSON.
 *
 * The parser does not allocate or copy memory. Each JsonPair contains:
 * - key:        pointer to the key string inside the original JSON
 * - valueStart: pointer to the beginning of the value
 * - valueLength: length of the value slice
 *
 * The value is not interpreted here; classification happens later.
 */
struct JsonPair
{
    const char *key;
    const char *valueStart;
    size_t valueLength;
};

// ------------------------------------------------------------
// Value type classification
// ------------------------------------------------------------

/**
 * @brief Supported JSON value types.
 *
 * This parser handles only the minimal subset required by the project:
 * - STRING:   "text"
 * - BOOLEAN:  true / false
 * - NUMBER:   integer or float
 * - NULLTYPE: null
 * - UNKNOWN:  unrecognized or malformed value
 */
enum class JsonType
{
    STRING,
    BOOLEAN,
    NUMBER,
    NULLTYPE,
    UNKNOWN
};

/**
 * @brief Represents a classified JSON value.
 *
 * Contains the detected type and a slice pointing to the raw value
 * inside the original JSON buffer.
 */
struct JsonValue
{
    JsonType type;
    const char *start;
    size_t length;
};

// ------------------------------------------------------------
// API declarations
// ------------------------------------------------------------

/**
 * @brief Extracts top‑level "key": value pairs from a JSON object.
 *
 * This function performs a shallow parse of a JSON object and fills
 * the provided output array with key/value slices. It does not
 * allocate memory or modify the input buffer.
 *
 * @param json      Null‑terminated JSON string.
 * @param out       Output array of JsonPair structures.
 * @param maxPairs  Maximum number of pairs to write into `out`.
 *
 * @return Number of pairs successfully extracted.
 *
 * @note Only top‑level pairs are parsed. Nested objects/arrays are
 *       treated as opaque value slices.
 */
size_t parse_json_pairs(const char *json, JsonPair *out, size_t maxPairs);

/**
 * @brief Looks up a key in a parsed pair list and classifies its value.
 *
 * @param pairs   Array of JsonPair entries.
 * @param count   Number of entries in the array.
 * @param key     Key to search for.
 * @param out     Output JsonValue containing type and slice.
 *
 * @return true if the key was found and the value was classified.
 */
bool json_get(const JsonPair *pairs, size_t count,
              const char *key, JsonValue &out);

/**
 * @brief Extracts a string value into a user‑provided buffer.
 *
 * @param v        Classified JSON value.
 * @param out      Destination buffer.
 * @param maxLen   Maximum number of bytes to write (including null).
 *
 * @return true if the value is a valid JSON string and fits in the buffer.
 */
bool getString(const JsonValue &v, char *out, size_t maxLen);

/**
 * @brief Converts a JSON boolean value into a native bool.
 *
 * @return true if the value is a valid boolean.
 */
bool getBool(const JsonValue &v, bool &out);

/**
 * @brief Converts a JSON number into an integer.
 *
 * @return true if the value is a valid number and fits in int range.
 */
bool getInt(const JsonValue &v, int &out);
