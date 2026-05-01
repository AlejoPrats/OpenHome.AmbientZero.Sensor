#include "protocol/server_response.hpp"
#include "protocol/json_parser.hpp"
#include <string.h>
#include "stdio.h"

bool parse_server_response(const char *json, ServerResponse &out)
{
    // Reset output
    out.deviceId[0] = '\0';
    out.hasDeviceId = false;
    out.isLightEnabled = false;
    out.shouldUpdate = false;

    // Parse JSON into key/value pairs
    JsonPair pairs[8];
    size_t count = parse_json_pairs(json, pairs, 8);

    JsonValue v;

    // -------------------------
    // DeviceId
    // -------------------------
    if (json_get(pairs, count, "DeviceId", v))
    {
        if (v.type == JsonType::NULLTYPE)
        {
            out.deviceId[0] = '\0';
            out.hasDeviceId = false;
        }
        else
        {
            if (get_string(v, out.deviceId, sizeof(out.deviceId)))
            {
                out.hasDeviceId = true;
            }
        }
    }

    // -------------------------
    // IsLightEnabled
    // -------------------------
    if (json_get(pairs, count, "IsLightEnabled", v))
    {
        bool b;
        if (get_bool(v, b))
        {
            out.isLightEnabled = b;
        }
    }

    // -------------------------
    // ShouldUpdate
    // -------------------------
    if (json_get(pairs, count, "ShouldUpdate", v))
    {
        bool b;
        if (get_bool(v, b))
        {
            out.shouldUpdate = b;
        }
    }

    return true;
}
