/**
 * m5power.c
 *
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#include "m5power.h"

static const char *TAG = "m5power";

#define AXP192_INIT_REG_COUNT 12
char * axp192_init_register_list_error_messages[AXP192_INIT_REG_COUNT] = {
    "OLED_VPP enable                        ",
    "Enable LDO2 & LDO3, LED & TFT 3.3V     ",
    "Enable ADCs                            ",
    "Enable Charging                        ",
    "Enable Coulomb counter                 ",
    "Enable DC-DC1, OLED_VDD, 5B V_EXT      ",
    "Enable PEK                             ",
    "Enable GPIO0                           ",
    "Enable USB thru mode                   ",
    "Battery charging High Temperature alarm",
    "Backup battery charging control        ",
    "Enable 3.0V? What is the Voff function?" };

#define AXP192_INIT_REGISTER_LCD_BACKLIGHT_LEVEL_REGISTER_INDEX 3
#define AXP192_INIT_REGISTER_LCD_BACKLIGHT_REGISTER_INDEX 11

uint8_t axp192_init_register_default_list[AXP192_INIT_REG_COUNT * 2] = {
    // OLED_VPP enable
    // 0x10 EXTEN & DC-DC2 output control
    // BIT2: EXTEN Switch control. 0 shut down. 1 turn on.
    // BIT0: DC-DC2 Switch control. 0 shut down. 1 turn on.
    EXTEN_DCDC2_SWITCH_CONTROL_REG, AXP192_10H, // 1

    // Enable LDO2 & LDO3, LED & TFT 3.3V
    // 0x28 LDO2/3 Output Voltage Setting
    // BIT7-4: 1.8-3.3V, 100mV/step
    // BIT3-0: 1.8-3.3V, 100mV/step
    LDO2_LDO3_VOLTAGE_SETTING_REG, AXP192_28H, //2

    // Enable ADCs
    // 0x82 ADC Enable 1. 0 shut down. 1 turn on.
    // BIT7: Battery voltage ADC enable
    // BIT6: Battery current ADC enable
    // BIT5: ACIN voltage ADC enable
    // BIT4: ACIN electric current ADC enable
    // BIT3: VBUS voltage ADC enable
    // BIT2: VBUS electric current ADC enable
    // BIT1: APS voltage ADC enable
    // BIT0: TS pin ADC enable function
    ADC_ENABLE_1_REG, AXP192_82H, //3

    // Enable Charging
    // 0x33 Charging Control 1
    // BIT7: Enable control bit, outer and inner channel.
    // BIT6-5: Target voltage. 00 4.1V, 01 4.15V, 10 4.2V, 11 4.36V
    // BIT4: Current setting at the end of charge. 0 current is less than 10% whem the end of charging set value. 1 15%.
    // BIT 3-0: Current setting internal passage.   0000 100mA, 0001 190mA, 0010 280mA, 0011 360mA
    //                                              0100 450mA, 0101 550mA, 0110 630mA, 0111 700mA
    //                                              1000 780mA, 1001 880mA, 1010 960mA, 1011 1000mA
    //                                              1100 1080mA, 1101 1160mA, 1110 1240mA, 1111 1320mA
    CHARGE_CONTROL_1_REG, AXP192_33H, //4

    // Enable Coulomb counter
    // 0xB8 Coulomb gauge control
    // BIT7: Switching control coulomb meter
    // BIT6: meter pause control. 1 pause metering. 0 resume.
    // BIT5: clear measurement
    COULOMB_COUNTER_CONTROL_REG, AXP192_B8H, //5

    // Enable DC-DC1, OLED_VDD, 5B V_EXT
    // 0x12 Power supply output control
    // BIT6: EXTEN switch control
    // BIT4: DC-DC2 switch control
    // BIT3: LDO3 switch control
    // BIT2: LDO2 switch control // Handles M5StickC LCD Backlight
    // BIT1: DC-DC3 switch control
    // BIT0: DC-DC1 switch control
    DCDC1_DCDC3_LDO2_LDO3_SWITCH_CONTROL_REG, AXP192_12H, // 0x4D, //6

    // Enable PEK
    // 0x36 PEK key parameters
    // BIT7-6: boot time settings. 00 128ms, 01 512ms, 10 1s, 11 2s.
    // BIT5-4: long time setting key. 00 1s, 01 1.5s, 10 2s, 11 2.5s.
    // BIT3: Automatic shutdown function ???
    // BIT2: PWROK signal delay after power-up complete. 0 32ms, 1 64ms.
    // BIT1-0: Long set off. 00 4s, 01 6s, 10 8s, 11 10s.
    PEK_SETTING_REG, AXP192_36H, // 0x0C, //7

    // Enable GPIO0
    // 0x90 GPIO0 feature
    // BIT2-0: 000 NMOS Open-drain output, 001 Universal input function, 010 Low noise LDO, 011 Retention.
    //         100 ADC entry, 101 output low, 11x floating
    GPIO0_CONTROL_REG, AXP192_90H, //8

    // Enable USB thru mode
    // 0x30 VBUS-IPSOUT path management
    // BIT7: 0 N_VBUSEN pin selection. 1 VBUS-IPSOUT input selection regardles of N_VBUSEN status.
    // BIT6: VBUS Vhold limiting control. 0 no limit. 1 limit.
    // BIT5-3: Vhold. 000 4.0V, 001 4.1V, 010 4.2V, 011 4.3V, 100 4.4V, 101 4.5V, 110 4.6V, 111 4.7V.
    // BIT1: VBUS limiting control enable. 0 shutdown. 1 enable.
    // BIT0: VBUS limit control current. 0 500mA, 1 100mA.
    VBUS_IPSOUT_PATH_SETTING_REG, AXP192_30H, //9

    // Battery charging High Temperature alarm
    // 0x39 VHTF-CHARGE: Default: 0x1F
    // BIT7-0: When charging the battery temperature threshold setting, N
    //         N * 10H , when N = 1FH ,correspond 0.397V ; Voltage may correspond to 0V ~ 3.264V
    //
    BATTERY_CHARGING_HIGH_TEMPERATURE_ALARM_REG, AXP192_39H, //10

    // Backup battery charging control
    // 0x35 BACKUP_BATTERY_CHARGING_CONTROL_REG Default: 0x22
    // BIT7: A spare battery enable control; 0: shut down; 1: turn on.
    // BIT6-5: Backup battery charging target voltage setting; 00: 3.1V ; 01: 3.0V ; 10: 3.0V ; 11: 2.5V.
    // BIT4-2: Reservations can not be changed
    // BIT1-0: Backup battery charging current setting: 00: 50uA ; 01: 100uA ; 10: 200uA ; 11: 400uA
    BACKUP_BATTERY_CHARGING_CONTROL_REG, AXP192_35H, //11

    // Enable 3.0V ??? What is the Voff function????
    // 0x31 Voff voltage setting
    // BIT3: Sleep mode PWRON press wakeup enable settings. 0 short press to wake up  ?????
    // BIT2-0: Voff setup. 000 2.6V, 001 2.7V, 010 2.8V, 011 2.9V, 100 3.0V, 101 3.1V, 110 3.2V, 111 3.3V
    VOFF_SHUTDOWN_VOLTAGE_SETTING_REG, AXP192_31H, //12
};

esp_err_t m5power_init(m5power_config_t *config)
{
    esp_err_t e;
    uint8_t error_count = 0;
    uint8_t i;

    if (config->enable_lcd_backlight == true)
    {
        axp192_init_register_default_list[AXP192_INIT_REGISTER_LCD_BACKLIGHT_REGISTER_INDEX] |= BIT2;
    }
    axp192_init_register_default_list[AXP192_INIT_REGISTER_LCD_BACKLIGHT_LEVEL_REGISTER_INDEX] = (0x80 | (config->lcd_backlight_level << 4)) | 0x0F;

    for (i = 0; i < AXP192_INIT_REG_COUNT * 2; i += 2)
    {
        e = m5power_register_write(axp192_init_register_default_list[ i ], axp192_init_register_default_list[ i + 1 ]);
        if (e != ESP_OK)
        {
            ESP_LOGE(TAG, "error %s", axp192_init_register_list_error_messages[ i / 2 ]);
            error_count++;
        }
        else
        {
            ESP_LOGI(TAG, "%s 0x%02X", axp192_init_register_list_error_messages[ i / 2 ], axp192_init_register_default_list[ i + 1 ]);
        }
        
    }

    if (error_count == 0)
    {
        ESP_LOGD(TAG, "Power manager initialized");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "%d errors found while initializing power manager", error_count);
        return ESP_FAIL;
    }
}

esp_err_t m5power_register_read(uint8_t register_address, uint8_t *register_content)
{
    esp_err_t e;

    // Read register content
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Error creating I2C link");
        return ESP_ERR_NO_MEM;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, register_address, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, register_content, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(I2C_NUM_0, cmd, 250 / portTICK_PERIOD_MS);
    if (e == ESP_OK)
    {
        ESP_LOGD(TAG, "Register %#04x content: %#04x", register_address, *register_content);
    }
    else
    {
        ESP_LOGE(TAG, "Error reading register %#04x: %s", register_address, esp_err_to_name(e));
        return ESP_FAIL;
    }
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}

esp_err_t m5power_register_write(uint8_t register_address, uint8_t register_content)
{
    esp_err_t e;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Error creating I2C link");
        return ESP_ERR_NO_MEM;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, register_address, true);
    i2c_master_write_byte(cmd, register_content, true);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    if (e == ESP_OK)
    {
        ESP_LOGD(TAG, "Register %#04x set to %#04x", register_address, register_content);
    }
    else
    {
        ESP_LOGE(TAG, "Error setting register %#04x set to %#04x: %s", register_address, register_content, esp_err_to_name(e));
        return ESP_FAIL;
    }
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}

esp_err_t m5power_register_set_bits(uint8_t register_address, uint8_t bits_to_set)
{
    esp_err_t e;
    uint8_t register_content;

    e = m5power_register_read(register_address, &register_content);
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    register_content |= bits_to_set;

    e = m5power_register_write(register_address, register_content);
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t m5power_register_clear_bits(uint8_t register_address, uint8_t bits_to_clear)
{
    esp_err_t e;
    uint8_t register_content;

    e = m5power_register_read(register_address, &register_content);
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    register_content &= ~bits_to_clear;

    e = m5power_register_write(register_address, register_content);
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t m5power_get_vbat(uint16_t *vbat)
{
    esp_err_t e;
    *vbat = 0;
    uint8_t read1, read2;

    e = m5power_register_read(0x78, &read1); // battery voltage LSB buff
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }
    e = m5power_register_read(0x79, &read2); // battery voltage MSB buff
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    *vbat = ((read1 << 4) + read2); // V

    ESP_LOGD(TAG, "VBat: %u", *vbat);

    return ESP_OK;
}

esp_err_t m5power_get_vaps(uint16_t *vaps)
{
    esp_err_t e;
    *vaps = 0;
    uint8_t read1, read2;

    e = m5power_register_read(0x7E, &read1); // APS voltage LSB buff
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }
    e = m5power_register_read(0x7F, &read2); // APS voltage MSB buff
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    *vaps = ((read1 << 4) + read2); // V

    ESP_LOGD(TAG, "VAPS: %u", *vaps);

    return ESP_OK;
}

esp_err_t m5power_set_sleep(void)
{
    esp_err_t e;
    uint8_t read1;

    e = m5power_register_read(VOFF_SHUTDOWN_VOLTAGE_SETTING_REG, &read1); // VOFF_SHUTDOWN_VOLTAGE_SETTING_REG
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    read1 = (1 << 3) | read1;
    e = m5power_register_write(VOFF_SHUTDOWN_VOLTAGE_SETTING_REG, read1); // VOFF_SHUTDOWN_VOLTAGE_SETTING_REG
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    e = m5power_register_write(DCDC1_DCDC3_LDO2_LDO3_SWITCH_CONTROL_REG, 0x01); // DCDC1_DCDC3_LDO2_LDO3_SWITCH_CONTROL_REG
    if (e != ESP_OK)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}