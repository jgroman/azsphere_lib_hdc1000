/***************************************************************************//**
* @file    hdc1000.h
* @version 1.0.0
*
* @brief Header file for TI HDC1000 sensor.
*
* @par Description
*    .
* @author
*
* @date
*
*******************************************************************************/

#ifndef __HDC1000_H__
#define __HDC1000_H__

/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include <stdint.h>
#include <stdlib.h>

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/
#define HDC1000_I2C_ADDR				0x40

#define HDC1000_REG_TEMP				0x00
#define HDC1000_REG_HUMI				0x01
#define	HDC1000_REG_CONFIG				0x02
#define HDC1000_REG_SERID_1				0xFB
#define HDC1000_REG_SERID_2				0xFC
#define HDC1000_REG_SERID_3				0xFD
#define HDC1000_REG_MFID				0xFE
#define HDC1000_REG_DEVID				0xFF

#define HDC1000_CFG_RST					0x80
#define	HDC1000_CFG_HEAT_ON				0x20
#define	HDC1000_CFG_HEAT_OFF			0x00
#define	HDC1000_CFG_BOTH_TEMP_HUMI		0x10
#define	HDC1000_CFG_SINGLE_MEASUR		0x00
#define	HDC1000_CFG_TEMP_HUMI_14BIT		0x00
#define	HDC1000_CFG_TEMP_11BIT			0x40
#define HDC1000_CFG_HUMI_11BIT			0x01
#define	HDC1000_CFG_HUMI_8BIT			0x02

#define HDC1000_MSG_I2C_READ_BYTE		0
#define HDC1000_MSG_I2C_READ_BYTES		1
#define HDC1000_MSG_I2C_WRITE_BYTE		2

#define HDC1000_MSG_DELAY_MILLI			10

#define HDC1000_MSG_GPIO_MODE_INPUT		20
#define HDC1000_MSG_GPIO_GET_VALUE		21



/*******************************************************************************
*   Global Variables and Constant Declarations with Applicable Initializations
*******************************************************************************/

/*******************************************************************************
*   Function Declarations
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct hdc1000_struct hdc1000_t;

typedef int(*hdc1000_msg_cb)(hdc1000_t *p_hdc, uint8_t msg, 
    uint8_t arg_int, void *arg_ptr);

struct hdc1000_struct {
    uint8_t i2c_addr;
    int drdyn_pin;
    hdc1000_msg_cb platform_cb;
};

hdc1000_t 
*hdc1000_init(uint8_t ad, int dp, hdc1000_msg_cb platform_cb);

void 
hdc1000_shutdown(hdc1000_t *p_hdc);
	
void 
hdc1000_set_config(hdc1000_t *p_hdc, uint8_t r, uint8_t m,
	uint8_t res, uint8_t h);

uint16_t 
hdc1000_get_config(hdc1000_t *p_hdc);

uint16_t 
hdc1000_get_temp_raw(hdc1000_t *p_hdc);

double 
hdc1000_get_temp(hdc1000_t *p_hdc);

uint16_t 
hdc1000_get_humi_raw(hdc1000_t *p_hdc);

double 
hdc1000_get_humi(hdc1000_t *p_hdc);

uint16_t 
hdc1000_get_dev_id(hdc1000_t *p_hdc);

uint16_t 
hdc1000_get_mf_id(hdc1000_t *p_hdc);

uint8_t 
hdc1000_get_battery_status(hdc1000_t *p_hdc);

#ifdef __cplusplus
}
#endif

#endif // __HDC1000_H__
/* [] END OF FILE */
