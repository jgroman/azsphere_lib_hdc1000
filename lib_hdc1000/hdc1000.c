/***************************************************************************//**
* @file    hdc1000.c
* @version 1.0.0
*
* @brief .
*
* @par Target device
*    HDC1000
*
* @par Related hardware
*    Avnet Azure Sphere MT3620 Starter Kit
*
* @author
*
* @date
*
*******************************************************************************/
#include "hdc1000.h"

#include <unistd.h>

/*******************************************************************************
* Forward declarations of private functions
*******************************************************************************/

static void 
hdc1000_set_reg_addr(hdc1000_t* p_hdc, uint8_t reg_addr);

static uint16_t
hdc1000_get_register(hdc1000_t* p_hdc);

static int
hdc1000_delay(hdc1000_t* p_hdc, uint8_t msg, uint8_t arg);

static int 
hdc1000_gpio(hdc1000_t* p_hdc, uint8_t msg, uint8_t arg_int, void *arg_ptr);

static int 
hdc1000_i2c_write(hdc1000_t* p_hdc, uint8_t arg);

static int 
hdc1000_i2c_read_bytes(hdc1000_t* p_hdc, uint8_t* buffer, uint8_t length);

/*******************************************************************************
* Public functions
*******************************************************************************/

/// <summary>
///		Initialize HDC1000 and allocate resources
/// <para>Memory resources are allocated from heap, to properly free
/// allocated memory use hdc1000_shutdown()</para>
/// </summary>
/// <param name="i2c_addr">HDC1000 I2C address</param>
/// <param name="drdyn_pin">DRDYn pin number or -1 if not used</param>
/// <param name="i2c_cb">Hardware dependent I2C functions callback</param>
/// <param name="gd_cb">Hardware dependent GPIO and delay functions cb</param>
/// <returns>Pointer to hdc1000_t data structure</returns>
hdc1000_t 
*hdc1000_init(uint8_t i2c_addr, int drdyn_pin, hdc1000_msg_cb platform_cb) 
{

	hdc1000_t *p_hdc = (hdc1000_t *)malloc(sizeof(hdc1000_t));
    if (p_hdc == NULL) 
    {
        return NULL;
    }

	p_hdc->i2c_addr = i2c_addr;
	if (0 == i2c_addr) 
    {
		p_hdc->i2c_addr = HDC1000_I2C_ADDR;
	}

	p_hdc->drdyn_pin = drdyn_pin;
	p_hdc->platform_cb = platform_cb;

	// If using DRDYn pin configure GPIO as Input
	if (drdyn_pin > -1) 
    {
		// Set MT3620 GPIO pin to Input mode
		hdc1000_gpio(p_hdc, HDC1000_MSG_GPIO_MODE_INPUT, 
			(uint8_t)drdyn_pin, NULL);
	}

	return p_hdc;
}

/// <summary>
///		Shutdown HDC1000 and free allocated resources
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
///
void 
hdc1000_shutdown(hdc1000_t *p_hdc) 
{
	free(p_hdc);
}

/// <summary>
///		Set HDC1000 Configuration
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <param name="reset"></param>
/// <param name="mode"></param>
/// <param name="resolution"></param>
/// <param name="heater"></param>
///
void 
hdc1000_set_config(hdc1000_t *p_hdc,
						uint8_t reset, uint8_t mode, 
						uint8_t resolution, uint8_t heater)
{
	uint8_t config = mode | resolution | heater | reset;

	hdc1000_i2c_write(p_hdc, HDC1000_REG_CONFIG);
	hdc1000_i2c_write(p_hdc, config);
	hdc1000_i2c_write(p_hdc, 0);
}

/// <summary>
///		Get Configuration
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Configuration register value</returns>
///
uint16_t 
hdc1000_get_config(hdc1000_t* p_hdc) 
{
	hdc1000_set_reg_addr(p_hdc, HDC1000_REG_CONFIG);
	return hdc1000_get_register(p_hdc);
}

/// <summary>
///		Get Temperature register
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Temperature register value</returns>
///
uint16_t 
hdc1000_get_temp_raw(hdc1000_t* p_hdc) 
{
	hdc1000_set_reg_addr(p_hdc, HDC1000_REG_TEMP);
	return hdc1000_get_register(p_hdc);
}

/// <summary>
///		Get Temperature
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Temperature in degrees Celsius</returns>
///
double 
hdc1000_get_temp(hdc1000_t* p_hdc) 
{
	double temp = (double) hdc1000_get_temp_raw(p_hdc);
	return ((temp / 65536.0) * 165.0) - 40.0;
}

/// <summary>
///		Get Humidity register
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Humidity register value</returns>
///
uint16_t 
hdc1000_get_humi_raw(hdc1000_t* p_hdc) 
{
	hdc1000_set_reg_addr(p_hdc, HDC1000_REG_HUMI);
	return hdc1000_get_register(p_hdc);
}

/// <summary>
///		Get Relative Humidity
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Relative Humidity in %RH</returns>
///
double 
hdc1000_get_humi(hdc1000_t* p_hdc) 
{
	double temp = (double)hdc1000_get_humi_raw(p_hdc);
	return (temp / 65536.0) * 100.0;
}

/// <summary>
///		Get manufacturer ID (= 0x5449)
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>HDC1000 Manufacturer ID</returns>
///
uint16_t 
hdc1000_get_mf_id(hdc1000_t* p_hdc) 
{
	hdc1000_set_reg_addr(p_hdc, HDC1000_REG_MFID);
	return hdc1000_get_register(p_hdc);
}

///<summary>
///   Get device ID (= 0x1000)
///</summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>HDC1000 Device ID</returns>
///
uint16_t 
hdc1000_get_dev_id(hdc1000_t* p_hdc) 
{
	hdc1000_set_reg_addr(p_hdc, HDC1000_REG_DEVID);
	return hdc1000_get_register(p_hdc);
}

/// <summary>
///		Get device battery status (BTST)
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Battery Voltage Status: 0 - over 2.8V, 1 - under 2.8V</returns>
///
uint8_t 
hdc1000_get_battery_status(hdc1000_t* p_hdc) 
{
	uint16_t config = hdc1000_get_config(p_hdc);

	if (config & 0x800) 
    {
		return 1;
	}
	return 0;
}

/*******************************************************************************
* Private functions
*******************************************************************************/

/// <summary>
///		Set register address for subsequent access
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <param name="reg_addr">Address of register to be accessed</param>
static void
hdc1000_set_reg_addr(hdc1000_t *p_hdc, uint8_t reg_addr) 
{
	uint8_t drdyn_state = 1;

	hdc1000_i2c_write(p_hdc, reg_addr);

	if (p_hdc->drdyn_pin > -1) 
    {
		// Using DRDYn to ack
		// After measurement is complete DRDYn is set to LOW
		while (drdyn_state == 1) 
        {
			hdc1000_gpio(p_hdc, HDC1000_MSG_GPIO_GET_VALUE, 0, &drdyn_state);
		}
	}
	else 
    {
		hdc1000_delay(p_hdc, HDC1000_MSG_DELAY_MILLI, 20);
	}
}

/// <summary>
///		Gets register value
///	<para>Register address has to be set by hdc1000_set_reg_addr() first</para>
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
/// <returns>Register value</returns>
/// 
static uint16_t 
hdc1000_get_register(hdc1000_t* p_hdc) 
{
	uint8_t bytes[2];

	hdc1000_i2c_read_bytes(p_hdc, bytes, 2);
	return (uint16_t)((bytes[0] << 8) + bytes[1]);
}

///
///
static int 
hdc1000_delay(hdc1000_t* p_hdc, uint8_t msg, uint8_t arg) 
{
	return (*p_hdc->platform_cb)(p_hdc, msg, arg, NULL);
}

///
///
static int 
hdc1000_gpio(hdc1000_t* p_hdc, uint8_t msg, uint8_t arg_int, void *arg_ptr) 
{
	return (*p_hdc->platform_cb)(p_hdc, msg, arg_int, arg_ptr);
}

///
///
int 
hdc1000_i2c_write(hdc1000_t* p_hdc, uint8_t arg) 
{
	return (*p_hdc->platform_cb)(p_hdc, HDC1000_MSG_I2C_WRITE_BYTE, arg, NULL);
}

///
///
static int
hdc1000_i2c_read_bytes(hdc1000_t *p_hdc, uint8_t *buffer, uint8_t length) 
{
	return (*p_hdc->platform_cb)(p_hdc, HDC1000_MSG_I2C_READ_BYTES, length,
		buffer);
}

/* [] END OF FILE */
