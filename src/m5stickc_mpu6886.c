/**
 * m5stickc_mpu6886.c
 *
 * (C) 2019 - Timothee Cruse (timothee.cruse@gmail.com)
 * This code is licensed under the MIT License.
 */

#include "m5stickc_mpu6886.h"
#include "m5stickc_power.h"
#include "wire.h"
#include "MahonyAHRS.h"

static const char * TAG = "m5stickc_mpu6886";

esp_err_t M5StickCMPU6886_Read_NBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *read_Buffer);
esp_err_t M5StickCMPU6886_Write_NBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *write_Buffer);


enum Gscale Gyscale = GFS_2000DPS;
enum Ascale Acscale = AFS_8G;

#define MPU6886_INIT_REG_COUNT 13
uint8_t mpu6886_init_register_default_list[MPU6886_INIT_REG_COUNT * 2] = {
    MPU6886_PWR_MGMT_1,     0x00,
    MPU6886_PWR_MGMT_1,     (0x01 << 7),
    MPU6886_PWR_MGMT_1,     (0x01 << 0),
    MPU6886_ACCEL_CONFIG,   0x10,
    MPU6886_GYRO_CONFIG,    0x18,
    MPU6886_CONFIG,         0x01,
    MPU6886_SMPLRT_DIV,     0x05,
    MPU6886_INT_ENABLE,     0x00,
    MPU6886_ACCEL_CONFIG2,  0x00,
    MPU6886_USER_CTRL,      0x00,
    MPU6886_FIFO_EN,        0x00,
    MPU6886_INT_PIN_CFG,    0x22,
    MPU6886_INT_ENABLE,     0x01
};


esp_err_t M5StickCMPU6886Init()
{
    esp_err_t e;

    uint8_t tempdata;
    uint8_t error_count = 0;
    uint8_t i;

    e = M5StickCMPU6886_Read_NBytes(MPU6886_WHOAMI, 1, &tempdata);
    if (e != ESP_OK || tempdata != 0x19)
    {
        ESP_LOGE(TAG, "error reading MPU6886_WHOAMI");
        return ESP_FAIL;
    }

    for (i = 0; i < MPU6886_INIT_REG_COUNT * 2; i += 2)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        e = M5StickCMPU6886_Write_NBytes(mpu6886_init_register_default_list[ i ], 1, mpu6886_init_register_default_list + i + 1 );
        if (e != ESP_OK)
        {
            ESP_LOGE(TAG, "error writing 0x%02X register", mpu6886_init_register_default_list[ i ]);
            error_count++;
        }
        else
        {
            ESP_LOGI(TAG, "0x%02X 0x%02X", mpu6886_init_register_default_list[ i ], mpu6886_init_register_default_list[ i + 1 ]);
        }
        
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    M5StickCMPU6886GetGres();
    M5StickCMPU6886GetAres();

    if (error_count == 0)
    {
        ESP_LOGD(TAG, "MPU6886 initialized");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "%d errors found while initializing MPU6886", error_count);
        return ESP_FAIL;
    }
}

esp_err_t M5StickCMPU6886GetAccelAdc(int16_t *ax, int16_t *ay, int16_t *az)
{
    esp_err_t e;
    uint8_t buf[6];
    e = M5StickCMPU6886_Read_NBytes(MPU6886_ACCEL_XOUT_H, 6, buf);

    *ax = ((int16_t)buf[0] << 8) | buf[1];
    *ay = ((int16_t)buf[2] << 8) | buf[3];
    *az = ((int16_t)buf[4] << 8) | buf[5];

    return e;
}
esp_err_t M5StickCMPU6886GetGyroAdc(int16_t *gx, int16_t *gy, int16_t *gz)
{
    esp_err_t e;
    uint8_t buf[6];
    e = M5StickCMPU6886_Read_NBytes(MPU6886_GYRO_XOUT_H, 6, buf);

    *gx = ((uint16_t)buf[0] << 8) | buf[1];
    *gy = ((uint16_t)buf[2] << 8) | buf[3];
    *gz = ((uint16_t)buf[4] << 8) | buf[5];

    return e;
}
esp_err_t M5StickCMPU6886GetTempAdc(int16_t *t)
{
    esp_err_t e;
    uint8_t buf[2];
    e = M5StickCMPU6886_Read_NBytes(MPU6886_TEMP_OUT_H, 2, buf);

    *t = ((uint16_t)buf[0] << 8) | buf[1];

    return e;
}

void M5StickCMPU6886GetGres()
{
    switch (Gyscale)
    {
        // Possible gyro scales (and their register bit settings) are:
    case GFS_250DPS:
        gRes = 250.0 / 32768.0;
        break;
    case GFS_500DPS:
        gRes = 500.0 / 32768.0;
        break;
    case GFS_1000DPS:
        gRes = 1000.0 / 32768.0;
        break;
    case GFS_2000DPS:
        gRes = 2000.0 / 32768.0;
        break;
    }
}

void M5StickCMPU6886GetAres()
{
    switch (Acscale)
    {
        // Possible accelerometer scales (and their register bit settings) are:
        // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case AFS_2G:
        aRes = 2.0 / 32768.0;
        break;
    case AFS_4G:
        aRes = 4.0 / 32768.0;
        break;
    case AFS_8G:
        aRes = 8.0 / 32768.0;
        break;
    case AFS_16G:
        aRes = 16.0 / 32768.0;
        break;
    }
}

esp_err_t MPU6886setFIFOEnable(bool enableflag)
{
    esp_err_t e;
    uint8_t regdata = 0;

    if (enableflag)
    {
        regdata = 0x0C; // MPU6886_FIFO_ENABLE = 0x0C
        e = M5StickCMPU6886_Write_NBytes(MPU6886_FIFO_ENABLE, 1, &regdata);
        if (e != ESP_OK) {
            return e;
        }
        regdata = 0x40; // MPU6886_USER_CTRL = 0x40
        e = M5StickCMPU6886_Write_NBytes(MPU6886_USER_CTRL, 1, &regdata);        
        if (e != ESP_OK) {
            return e;
        }
    }
    else
    {
        regdata = 0x00;
        e = M5StickCMPU6886_Write_NBytes(MPU6886_FIFO_ENABLE, 1, &regdata);
        if (e != ESP_OK) {
            return e;
        }
        regdata = 0x00;
        e = M5StickCMPU6886_Write_NBytes(MPU6886_USER_CTRL, 1, &regdata);
        if (e != ESP_OK) {
            return e;
        }
    }

    return e;
}

esp_err_t M5StickCMPU6886ReadFIFO(uint8_t * rd_data)
{
    return M5StickCMPU6886_Read_NBytes(MPU6886_FIFO_R_W, 1, rd_data);
}

esp_err_t M5StickCMPU6886ReadFIFOBuff(uint8_t * rd_data , uint16_t length)
{
    return M5StickCMPU6886_Read_NBytes(MPU6886_FIFO_R_W, length, rd_data);
}

esp_err_t M5StickCMPU6886ReadFIFOCount(uint16_t * rd_data)
{
    esp_err_t e;
    uint8_t temp[2];
    e = M5StickCMPU6886_Read_NBytes(MPU6886_FIFO_CONUTH, 2, temp);
    *rd_data = temp[0];
    *rd_data <<= 8;
    *rd_data |= temp[1];
    return e;
}

esp_err_t M5StickCMPU6886SetGyroFsr(enum Gscale scale)
{
    //return IIC_Write_Byte(MPU_GYRO_CFG_REG,scale<<3);//设置陀螺仪满量程范围
    unsigned char regdata;
    regdata = (scale << 3);
    esp_err_t e = M5StickCMPU6886_Write_NBytes(MPU6886_GYRO_CONFIG, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    Gyscale = scale;
    M5StickCMPU6886GetGres();

    return e;
}

esp_err_t M5StickCMPU6886SetAccelFsr(enum Ascale scale)
{
    unsigned char regdata;
    regdata = (scale << 3);
    esp_err_t e = M5StickCMPU6886_Write_NBytes(MPU6886_ACCEL_CONFIG, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    
    Acscale = scale;
    M5StickCMPU6886GetAres();

    return e;
}

esp_err_t M5StickCMPU6886GetAccelData(float *ax, float *ay, float *az)
{
    int16_t accX = 0;
    int16_t accY = 0;
    int16_t accZ = 0;
    esp_err_t e = M5StickCMPU6886GetAccelAdc(&accX, &accY, &accZ);

    *ax = (float)accX * aRes;
    *ay = (float)accY * aRes;
    *az = (float)accZ * aRes;

    return e;
}

esp_err_t M5StickCMPU6886GetGyroData(float *gx, float *gy, float *gz)
{
    int16_t gyroX = 0;
    int16_t gyroY = 0;
    int16_t gyroZ = 0;
    esp_err_t e = M5StickCMPU6886GetGyroAdc(&gyroX, &gyroY, &gyroZ);

    *gx = (float)gyroX * gRes;
    *gy = (float)gyroY * gRes;
    *gz = (float)gyroZ * gRes;

    return e;
}

esp_err_t M5StickCMPU6886GetTempData(float *t)
{
    int16_t temp = 0;
    esp_err_t e = M5StickCMPU6886GetTempAdc(&temp);

    *t = (float)temp / 326.8 + 25.0;

    return e;
}

esp_err_t M5StickCMPU6886GetAhrsData(float *pitch, float *roll, float *yaw)
{
    esp_err_t e;

    float accX = 0; 
    float accY = 0;
    float accZ = 0;

    float gyroX = 0;
    float gyroY = 0;
    float gyroZ = 0;


    e = M5StickCMPU6886GetGyroData(&gyroX, &gyroY, &gyroZ);
    if (e != ESP_OK) {
        return e;
    }
    e = M5StickCMPU6886GetAccelData(&accX, &accY, &accZ);
    if (e != ESP_OK) {
        return e;
    }

    float deg_to_rad = 0.017453292519943295769236907684886f;
    MahonyAHRSupdateIMU(gyroX * deg_to_rad, gyroY * deg_to_rad, gyroZ * deg_to_rad, accX, accY, accZ, pitch, roll, yaw);

    return e;
}

esp_err_t M5StickCMPU6886_Read_NBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *read_Buffer)
{
    return I2CReadBuff(&wire0, MPU6886_ADDRESS, start_Addr, read_Buffer, number_Bytes);
}

esp_err_t M5StickCMPU6886_Write_NBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *write_Buffer)
{
    return I2CWriteBuff(&wire0, MPU6886_ADDRESS, start_Addr, write_Buffer, number_Bytes);
}
