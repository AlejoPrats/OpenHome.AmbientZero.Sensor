#include "network/wifi_scan.hpp"

// Current scan results
WifiScan::NetworkInfo WifiScan::networks[WifiScan::MAX_NETWORKS];
int WifiScan::networkCount = 0;

// Previous scan results
static WifiScan::NetworkInfo previousNetworks[WifiScan::MAX_NETWORKS];
static int previousCount = 0;
static int scansRemaining = 1;
static WifiScan::NetworkInfo masterList[WifiScan::MAX_NETWORKS];
static int masterCount = 0;


bool WifiScan::scanInProgress = false;
absolute_time_t WifiScan::scanDeadline;

async_at_time_worker_t WifiScan::worker;

void WifiScan::start(int scans) {
    scansRemaining = scans;
    masterCount = 0;

    networkCount = 0;
    scanInProgress = false;

    worker.do_work = workerFn;
    worker.user_data = nullptr;

    async_context_add_at_time_worker_in_ms(
        cyw43_arch_async_context(),
        &worker,
        200   // small initial delay
    );
}

void WifiScan::poll() {
    async_context_poll(cyw43_arch_async_context());

    if (scanInProgress) {
        if (networkCount >= MAX_NETWORKS) {
            finishScan();
        } else if (absolute_time_diff_us(get_absolute_time(), scanDeadline) < 0) {
            finishScan();
        }
    }
}

void WifiScan::workerFn(async_context_t* context, async_at_time_worker_t* w) {
    if (scanInProgress) return;

    networkCount = 0;
    scanInProgress = true;
    scanDeadline = make_timeout_time_ms(SCAN_TIMEOUT_MS);

    cyw43_wifi_scan_options_t opts = {0};

    int err = cyw43_wifi_scan(&cyw43_state, &opts, nullptr, scanCallback);
    if (err != 0) {
        scanInProgress = false;
    }
}

int WifiScan::scanCallback(void* env, const cyw43_ev_scan_result_t* result) {
    if (!scanInProgress) return 0;

    if (!result) {
        finishScan();
        return 0;
    }

    // Extract SSID
    char ssidBuf[33];
    memset(ssidBuf, 0, sizeof(ssidBuf));
    memcpy(ssidBuf, result->ssid, result->ssid_len);

    if (ssidBuf[0] == '\0') {
        strcpy(ssidBuf, "[Hidden SSID]");
    }

    // Determine security
    bool secure = (result->auth_mode != CYW43_AUTH_OPEN);

    // Check if already stored
    for (int i = 0; i < networkCount; ++i) {
        if (strcmp(networks[i].ssid, ssidBuf) == 0) {
            return 0;
        }
    }

    // Store new unique network
    if (networkCount < MAX_NETWORKS) {
        strcpy(networks[networkCount].ssid, ssidBuf);
        networks[networkCount].secure = secure;
        networkCount++;
    }

    return 0;
}


// ------------------------------------------------------------
// DIFF LOGIC: Only detect NEW SSIDs (ignore disappeared ones)
// ------------------------------------------------------------
static void computeDiff() {
    int newCount = 0;

    for (int i = 0; i < WifiScan::networkCount; ++i) {
        bool found = false;

        for (int j = 0; j < previousCount; ++j) {
            if (strcmp(WifiScan::networks[i].ssid, previousNetworks[j].ssid) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            newCount++;
        }
    }

    // Update previous list
    previousCount = WifiScan::networkCount;
    for (int i = 0; i < WifiScan::networkCount; ++i) {
        previousNetworks[i] = WifiScan::networks[i];
    }
}

static void mergeIntoMaster() {
    for (int i = 0; i < WifiScan::networkCount; ++i) {
        bool found = false;

        for (int j = 0; j < masterCount; ++j) {
            if (strcmp(WifiScan::networks[i].ssid, masterList[j].ssid) == 0) {
                found = true;
                break;
            }
        }

        if (!found && masterCount < WifiScan::MAX_NETWORKS) {
            masterList[masterCount] = WifiScan::networks[i];
            masterCount++;
        }
    }
}

void WifiScan::finishScan() {
    if (!scanInProgress) return;

    scanInProgress = false;

    // Merge this scan into the master list
    mergeIntoMaster();

    scansRemaining--;

    if (scansRemaining > 0) {
        // Schedule next scan
        async_context_add_at_time_worker_in_ms(
            cyw43_arch_async_context(),
            &worker,
            SCAN_TIMEOUT_MS - 100
        );
        return;
    }
}

bool WifiScan::isFinished() {
    return (!scanInProgress && scansRemaining == 0);
}

