/**
 * m5stickc_led.h
 *
 * (C) 2019 - Timothee Cruse <timothee.cruse@gmail.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5STICKC_LED_H_
#define _M5STICKC_LED_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_system.h"
#include "esp_log.h"

#define M5STICKC_LED_ON 0
#define M5STICKC_LED_OFF 1

#define M5STICKC_LED_DEFAULT_STATE M5STICKC_LED_OFF
#define M5STICKC_LED_GPIO          GPIO_NUM_10

/**
 * @brief   Initialize led
 * *
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t M5StickCLedInit();

/**
 * @brief   Check if led is on
 *
 * @return  false not on
 *          true otherwise on
 */
bool M5StickCLedIsOn(void);

/**
 * @brief   Set led.
 *
 * @param   state led state to set
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t M5StickCLedSet(bool state);

/**
 * @brief   Toggle led.
 *
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t M5StickCLedToggle(void);

#ifdef __cplusplus
}
#endif

#endif // _M5STICKC_LED_H_