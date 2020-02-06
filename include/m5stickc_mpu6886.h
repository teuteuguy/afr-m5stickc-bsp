/**
 * m5stickc_mpu6886.h
 *
 * (C) 2019 - Timothee Cruse <timothee.cruse@gmail.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5STICKC_MPU6886_H_
#define _M5STICKC_MPU6886_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_system.h"
#include "esp_log.h"

#define MPU6886_ADDRESS           0x68 
#define MPU6886_WHOAMI            0x75
#define MPU6886_ACCEL_INTEL_CTRL  0x69
#define MPU6886_SMPLRT_DIV        0x19
#define MPU6886_INT_PIN_CFG       0x37
#define MPU6886_INT_ENABLE        0x38
#define MPU6886_ACCEL_XOUT_H      0x3B
#define MPU6886_ACCEL_XOUT_L      0x3C
#define MPU6886_ACCEL_YOUT_H      0x3D
#define MPU6886_ACCEL_YOUT_L      0x3E
#define MPU6886_ACCEL_ZOUT_H      0x3F
#define MPU6886_ACCEL_ZOUT_L      0x40

#define MPU6886_TEMP_OUT_H        0x41
#define MPU6886_TEMP_OUT_L        0x42

#define MPU6886_GYRO_XOUT_H       0x43
#define MPU6886_GYRO_XOUT_L       0x44
#define MPU6886_GYRO_YOUT_H       0x45
#define MPU6886_GYRO_YOUT_L       0x46
#define MPU6886_GYRO_ZOUT_H       0x47
#define MPU6886_GYRO_ZOUT_L       0x48

#define MPU6886_USER_CTRL         0x6A
#define MPU6886_PWR_MGMT_1        0x6B
#define MPU6886_PWR_MGMT_2        0x6C
#define MPU6886_CONFIG            0x1A
#define MPU6886_GYRO_CONFIG       0x1B
#define MPU6886_ACCEL_CONFIG      0x1C
#define MPU6886_ACCEL_CONFIG2     0x1D
#define MPU6886_FIFO_EN           0x23

#define MPU6886_FIFO_ENABLE		  0x23
#define MPU6886_FIFO_CONUTH		  0x72
#define MPU6886_FIFO_CONUTL		  0x73
#define MPU6886_FIFO_R_W		  0x74

//#define G (9.8)
#define RtA     57.324841
#define AtR    	0.0174533	
#define Gyro_Gr	0.0010653

enum Ascale {
	AFS_2G = 0,
	AFS_4G,
	AFS_8G,
	AFS_16G
};

enum Gscale {
	GFS_250DPS = 0,
	GFS_500DPS,
	GFS_1000DPS,
	GFS_2000DPS
};

extern enum Gscale Gyscale;
extern enum Ascale Acscale;

/**
 * @brief   Initialize MPU6886
 * *
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t M5StickCMPU6886Init( void );

esp_err_t M5StickCMPU6886GetAccelAdc( int16_t* ax, int16_t* ay, int16_t* az );
esp_err_t M5StickCMPU6886GetGyroAdc( int16_t* gx, int16_t* gy, int16_t* gz );
esp_err_t M5StickCMPU6886GetTempAdc( int16_t *t );

esp_err_t M5StickCMPU6886GetAccelData( float* ax, float* ay, float* az );
esp_err_t M5StickCMPU6886GetGyroData( float* gx, float* gy, float* gz );
esp_err_t M5StickCMPU6886GetTempData( float *t );

esp_err_t M5StickCMPU6886SetGyroFsr( enum Gscale scale );
esp_err_t M5StickCMPU6886SetAccelFsr( enum Ascale scale );

void M5StickCMPU6886GetAhrsData( float *pitch, float *roll, float *yaw );
esp_err_t M5StickCMPU6886SetFIFOEnable( bool );
esp_err_t M5StickCMPU6886ReadFIFO( uint8_t * rd_data );
esp_err_t M5StickCMPU6886ReadFIFOCount( uint16_t * rd_data );
esp_err_t M5StickCMPU6886ReadFIFOBuff( uint8_t * rd_data , uint16_t length );

float aRes, gRes;

void M5StickCMPU6886GetGres( void );
void M5StickCMPU6886GetAres( void );

#ifdef __cplusplus
}
#endif

#endif // _M5STICKC_MPU6886_H_