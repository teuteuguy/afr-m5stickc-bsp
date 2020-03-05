/**
 * m5stickc_event.h
 *
 * (C) 2020 - Timothee Cruse <timothee.cruse@gmail.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5EVENT_H_
#define _M5EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_event.h"

extern esp_event_loop_handle_t m5stickc_event_loop;   /*!< Event loop for M5 device-specific events */

esp_err_t M5StickCEventInit();

#ifdef __cplusplus
}
#endif

#endif // _M5EVENT_H_
