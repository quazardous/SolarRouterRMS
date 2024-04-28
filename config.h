
/**
 * @file config.h
 * @brief This file contains customizable defines for the RMS Router and the modules.
 * 
 * @details All cutomizable defines should be referenced here.
 * 
 * @author David Berlioz<berliozdavid@gmail.com>
 */
#pragma once

#ifndef RMS_HOSTNAME_PREFIX
#define RMS_HOSTNAME_PREFIX "RMS-ESP32-"
#endif

#ifndef RMS_ROUTER_NAME
#define RMS_ROUTER_NAME "My RMS Router"
#endif

#ifndef RMS_MOBILE_PROBE_NAME
#define RMS_MOBILE_PROBE_NAME "House Load"
#endif

#ifndef RMS_FIX_PROBE_NAME
#define RMS_FIX_PROBE_NAME "2nd Probe"
#endif

#ifndef RMS_TEMPERATURE_NAME
#define RMS_TEMPERATURE_NAME "Temperature"
#endif

#ifndef RMS_WIFI_SSID
#define RMS_WIFI_SSID ""
#endif

#ifndef RMS_WIFI_KEY
#define RMS_WIFI_KEY ""
#endif

// Allow Offshore mode. This will allow CORS for all requests.
#ifndef RMS_WEB_SERVER_ALLOW_OFFSHORE
#define RMS_WEB_SERVER_ALLOW_OFFSHORE 1
#endif