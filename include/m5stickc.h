/**
 * m5stickc.h - ESP-IDF component to work with M5StickC
 *
 * Include this header file to use the component.
 *
 * (C) 2019 - Timothee Cruse <timothee.cruse@gmail.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5STICKC_H_
#define _M5STICKC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "wire.h"
#include "m5stickc_power.h"
#include "m5stickc_button.h"
#include "m5stickc_display.h"
#include "m5stickc_event.h"
#include "m5stickc_led.h"
#include "m5stickc_mpu6886.h"

typedef struct {
    m5stickc_power_config_t power;
} m5stickc_config_t;

#define M5STICKC_CONFIG_DEFAULT() { \
    .power = M5POWER_CONFIG_DEFAULT() \
    };

/**
 * @brief   Initialize M5StickC
 *
 *          Initializes power management, display and buttons.
 *
 * @return  ESP_OK success
 *          ESP_FAIL errors found
 */
esp_err_t M5StickCInit(m5stickc_config_t * config);

#ifdef __cplusplus
}
#endif

#endif // _M5STICKC_H_
