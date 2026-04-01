#include "protocol/json_parser.hpp"
#include <string.h>
#include <stdlib.h>

// ------------------------------------------------------------
// Tiny JSON key/value extractor (top-level only)
// ------------------------------------------------------------
size_t parse_json_pairs(const char *json, JsonPair *out, size_t maxPairs)
{
    size_t count = 0;
    const char *p = json;

    while (*p && count < maxPairs)
    {
        const char *keyStart = strchr(p, '\"');
        if (!keyStart)
            break;

        const char *keyEnd = strchr(keyStart + 1, '\"');
        if (!keyEnd)
            break;

        const char *colon = strchr(keyEnd, ':');
        if (!colon)
            break;

        const char *v = colon + 1;

        while (*v == ' ' || *v == '\t' || *v == '\n' || *v == '\r')
            v++;

        const char *valueStart = nullptr;
        size_t valueLength = 0;

        if (*v == '\"')
        {
            valueStart = v + 1;
            const char *end = strchr(valueStart, '\"');
            if (!end)
                break;
            valueLength = end - valueStart;
            p = end + 1;
        }
        else if (strncmp(v, "true", 4) == 0)
        {
            valueStart = v;
            valueLength = 4;
            p = v + 4;
        }
        else if (strncmp(v, "false", 5) == 0)
        {
            valueStart = v;
            valueLength = 5;
            p = v + 5;
        }
        else if (strncmp(v, "null", 4) == 0)
        {
            valueStart = v;
            valueLength = 4;
            p = v + 4;
        }
        else
        {
            valueStart = v;
            const char *end = v;
            while (*end && *end != ',' && *end != '}' &&
                   *end != ' ' && *end != '\n' && *end != '\r')
                end++;
            valueLength = end - v;
            p = end;
        }

        out[count++] = {
            keyStart + 1,
            valueStart,
            valueLength};
    }

    return count;
}

// ------------------------------------------------------------
// Lookup a key and classify its value
// ------------------------------------------------------------
bool json_get(const JsonPair *pairs, size_t count,
              const char *key, JsonValue &out)
{
    size_t keyLen = strlen(key);

    for (size_t i = 0; i < count; i++)
    {
        if (strncmp(pairs[i].key, key, keyLen) == 0)
        {
            out.start = pairs[i].valueStart;
            out.length = pairs[i].valueLength;

            if (out.length == 4 && strncmp(out.start, "null", 4) == 0)
            {
                out.type = JsonType::NULLTYPE;
            }
            else if (out.length == 4 && strncmp(out.start, "true", 4) == 0)
            {
                out.type = JsonType::BOOLEAN;
            }
            else if (out.length == 5 && strncmp(out.start, "false", 5) == 0)
            {
                out.type = JsonType::BOOLEAN;
            }
            else if (out.length > 0 && out.start[-1] == '\"')
            {
                out.type = JsonType::STRING;
            }
            else if ((*out.start >= '0' && *out.start <= '9') || *out.start == '-')
            {
                out.type = JsonType::NUMBER;
            }
            else
            {
                out.type = JsonType::UNKNOWN;
            }

            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------
// Typed getters
// ------------------------------------------------------------
bool getString(const JsonValue &v, char *out, size_t maxLen)
{
    if (v.type != JsonType::STRING)
        return false;

    if (v.length >= maxLen)
        return false;

    memcpy(out, v.start, v.length);
    out[v.length] = '\0';
    return true;
}

bool getBool(const JsonValue &v, bool &out)
{
    if (v.type != JsonType::BOOLEAN)
        return false;

    if (v.length == 4 && strncmp(v.start, "true", 4) == 0)
    {
        out = true;
        return true;
    }
    if (v.length == 5 && strncmp(v.start, "false", 5) == 0)
    {
        out = false;
        return true;
    }

    return false;
}

bool getInt(const JsonValue &v, int &out)
{
    if (v.type != JsonType::NUMBER)
        return false;

    out = atoi(v.start);
    return true;
}
