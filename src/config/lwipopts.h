#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Pull in the Pico W example defaults
#include "lwipopts_custom.h"

// --- Disable TLS / ALTCP ---
#undef LWIP_ALTCP
#define LWIP_ALTCP 0

#undef LWIP_ALTCP_TLS
#define LWIP_ALTCP_TLS 0

#undef LWIP_ALTCP_TLS_MBEDTLS
#define LWIP_ALTCP_TLS_MBEDTLS 0

// Optional: remove the TLS-specific TCP_WND override
#undef TCP_WND
#define TCP_WND (8 * TCP_MSS)

#endif
