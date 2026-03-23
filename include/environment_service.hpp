// environment_service.hpp
#pragma once
#include "ahtx.hpp"

struct EnvironmentData {
    float temperature;
    float humidity;
    bool ok;
};

class EnvironmentService {
public:
    EnvironmentService(AHT10& sensor);

    bool init();
    EnvironmentData read();

private:
    AHT10& sensor;
    bool initialized = false;
};
