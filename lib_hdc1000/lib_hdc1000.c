/*
 * Driver HDC1000 digital temperature and humidity sensor used with Azure Sphere
 * developer kit.
 *
 * This source code file is a platform dependent part of HDC1000 driver.
 *
 * ---------------------------------------------------------------------------
 *
 * The BSD License (3-clause license)
 *
 * Copyright (c) 2019 Jaroslav Groman (https://github.com/jgroman)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 /***************************************************************************//**
 * @file    lib_hdc1000.c
 * @version 1.0.0
 *
 * @brief Definitions for HDC1000 platform dependent wrapper.
 *
 * @par Target device
 *    HDC1000
 *
 * @author Jaroslav Groman
 *
 * @date
 *
 *******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/i2c.h>
#include <applibs/gpio.h>

#include "lib_hdc1000.h"

 /*******************************************************************************
 * Forward declarations of private functions
 *******************************************************************************/

 /// <summary>
 ///     Platform dependent implementation of the delay and gpio callback
 /// </summary>
 /// <param name="p_hdc">Pointer to hdc1000_t data struct</param>
 /// <returns></returns>
static int
hdc1000_platform_cb(hdc1000_t *p_hdc, uint8_t msg, uint8_t arg_int,
    void *arg_ptr);

/*******************************************************************************
* Global variables
*******************************************************************************/

static int i2cFd = -1;      // Global I2C file descriptor
static int gpioFd = -1;     // Global GPIO file descriptor

/*******************************************************************************
* Function definitions
*******************************************************************************/

hdc1000_t
*hdc1000_open(int i2c_fd, I2C_DeviceAddress i2c_addr, int drdyn_pin)
{
    i2cFd = i2c_fd;
    return hdc1000_init((uint8_t)i2c_addr, drdyn_pin, hdc1000_platform_cb);
}

void
hdc1000_close(hdc1000_t *p_hdc)
{
    if (gpioFd != -1)
    {
        close(gpioFd);
    }

    hdc1000_shutdown(p_hdc);
}

static int
hdc1000_platform_cb(hdc1000_t *p_hdc, uint8_t msg, uint8_t arg_int,
    void *arg_ptr)
{

    struct timespec sleepTime;
    ssize_t result = 0;

    switch (msg)
    {
    case HDC1000_MSG_I2C_READ_BYTES:
        // Read arg_int bytes from I2C address and store it to arg_ptr
        result = I2CMaster_Read(i2cFd, p_hdc->i2c_addr, arg_ptr, arg_int);
        if (result == -1)
        {
            Log_Debug("ERROR: I2CMaster_Read: errno=%d (%s)\n", errno,
                strerror(errno));
            return -1;
        }
        break;

    case HDC1000_MSG_I2C_WRITE_BYTE:
        // Write 1 byte from arg_int to I2C address
        result = I2CMaster_Write(i2cFd, p_hdc->i2c_addr, &arg_int, 1);
        if (result == -1)
        {
            Log_Debug("ERROR: I2CMaster_Write: errno=%d (%s)\n", errno,
                strerror(errno));
            return -1;
        }
        break;

    case HDC1000_MSG_DELAY_MILLI:
        // Perform delay for arg_int milliseconds
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 1000000 * arg_int;
        result = nanosleep(&sleepTime, NULL);
        break;

    case HDC1000_MSG_GPIO_MODE_INPUT:
        // Open a GPIO as an input
        // arg_int should contain DRDYn signal GPIO pin number
        gpioFd = GPIO_OpenAsInput((GPIO_Id)arg_int);
        if (gpioFd == -1)
        {
            Log_Debug("ERROR: GPIO_OpenAsInput: errno=%d (%s)\n", errno,
                strerror(errno));
            result = -1;
        }
        break;

    case HDC1000_MSG_GPIO_GET_VALUE:
        // Gets the current value of a GPIO
        result = GPIO_GetValue(gpioFd, arg_ptr);
        if (result == -1)
        {
            Log_Debug("ERROR: GPIO_GetValue: errno=%d (%s)\n", errno,
                strerror(errno));
        }
        break;

    default:
        break;
    }

    return 1;
}

/* [] END OF FILE */
