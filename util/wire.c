#include "wire.h"

static const char * TAG = "wire";

esp_err_t I2CWireInit( wire_t* wire )
{
	i2c_config_t conf;
	esp_err_t err;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = wire->io_sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = wire->io_scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = wire->i2c_speed;
    err = i2c_param_config(wire->i2cnum, &conf);
	if( err != ESP_OK ) return err;
    err = i2c_driver_install(wire->i2cnum, 
							conf.mode,
							0,	//I2C_MASTER_RX_BUF_DISABLE
							0,  //I2C_MASTER_TX_BUF_DISABLE 
							0);
	return err;
}

esp_err_t I2CReadByte( wire_t* wire, uint8_t device_addr, uint8_t reg_addr, uint8_t* rd_content  )
{
	return I2CReadBuff( wire, device_addr, reg_addr, rd_content, 1);
}

esp_err_t I2CReadBuff( wire_t* wire, uint8_t device_addr, uint8_t reg_addr, uint8_t* rd_buff, uint8_t length )
{
	esp_err_t e;

    // Read register content
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Error creating I2C link");
        return ESP_ERR_NO_MEM;
    }
	if( length == 0 )
	{
		return -1;
	}

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
	if (length > 1) 
	{
        i2c_master_read(cmd, rd_buff, length - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, rd_buff + length - 1, NACK_VAL);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(wire->i2cnum, cmd, 250 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return e;
}

esp_err_t I2CWriteByte( wire_t* wire, uint8_t device_addr, uint8_t reg_addr, uint8_t Data)
{
	esp_err_t e;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Error creating I2C link");
        return ESP_ERR_NO_MEM;
    }

	i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, Data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(wire->i2cnum, cmd, 10 / portTICK_RATE_MS);
	if (e == ESP_OK)
    {
        ESP_LOGD(TAG, "Register %#04x set to %#04x", reg_addr, Data);
    }
    else
    {
        ESP_LOGE(TAG, "Error setting register %#04x set to %#04x: %s", reg_addr, Data, esp_err_to_name(e));
    }
    i2c_cmd_link_delete(cmd);
    return e;
}

esp_err_t I2CWriteBuff( wire_t* wire, uint8_t device_addr, uint8_t reg_addr, uint8_t* wd_buff, uint8_t length)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	esp_err_t e;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write(cmd, wd_buff, length, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(wire->i2cnum, cmd, 10 / portTICK_RATE_MS);
	if (e == ESP_OK)
    {
        ESP_LOGD(TAG, "Wrote %u Bytes starting from Register %#04x", length, reg_addr);
    }
    else
    {
        ESP_LOGE(TAG, "Error writing %u bytes at register %#04x: %s", length, reg_addr, esp_err_to_name(e));
    }
    i2c_cmd_link_delete(cmd);
    return e;
}

// esp_err_t I2CBeginTransmission( wire_t* wire, uint8_t device_addr, uint8_t reg_addr )
// {
// 	i2c_cmd_handle_t cmd;

// 	cmd = i2c_cmd_link_create();
// 	ESP_ERROR_CHECK(i2c_master_start(cmd));
// 	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, 1));
// 	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg_addr, 1));
// 	ESP_ERROR_CHECK(i2c_master_stop(cmd));
// 	ESP_ERROR_CHECK(i2c_master_cmd_begin(wire->i2cnum, cmd, 1000/portTICK_PERIOD_MS));
// 	i2c_cmd_link_delete(cmd);

// 	return 0;
// }

wire_t wire0={
	.i2cnum = 0,
	.io_scl = 22,
	.io_sda = 21,
	.i2c_speed = 10000
};

wire_t wire1=
{
	.i2cnum = 1,
	.io_scl = 32,
	.io_sda = 32,
	.i2c_speed = 10000
};