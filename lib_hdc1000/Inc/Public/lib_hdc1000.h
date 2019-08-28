/***************************************************************************//**
* @file    lib_hdc1000.h
* @version 1.0.0
*
* @brief
*
* @par Description
*
* @author
*
* @date
*
*******************************************************************************/
#ifndef LIBHDC1000_H
#define LIBHDC1000_H

/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include "hdc1000.h"

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/

// !!! DRDY requires external pull-up resistor to VDD !!!
// HDC1000 click module routes HDC1000 chip DRDY pin to module INT pin
// On Avnet Azure Sphere Starter Kit both click module sockets share INT pin
// signal and it is connected to GPIO2_PWM2 MT3620 pin

#define SK_MT3620_DRDY_GPIO 2


/*******************************************************************************
*   Function Declarations
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
///		Initialize HDC1000 sensor and associated resources
/// </summary>
/// <param name="i2c_fd">The file descriptor for the I2C interface</param>
/// <param name="i2c_addr">The address of the HDC1000 I2C device</param>
/// <param name="drdyn_pin">DRDYn signal GPIO pin number 
///							or -1 if not used</param>
/// <returns>Pointer to hdc1000_t data structure</returns>
hdc1000_t
*hdc1000_open(int i2c_fd, I2C_DeviceAddress i2c_addr, int drdyn_pin);

/// <summary>
///     Free HDC1000 associated resources
/// </summary>
/// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
void
hdc1000_close(hdc1000_t* __phdc);

#ifdef __cplusplus
}
#endif

#endif  // LIBHDC1000_H

/* [] END OF FILE */
