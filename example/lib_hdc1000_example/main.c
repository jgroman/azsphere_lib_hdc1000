#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>
#include <applibs/i2c.h>

// Import project hardware abstraction from project property "Target Hardware Definition Directory"
#include <hw/project_hardware.h>

#include "lib_hdc1000.h"

// Support functions.
static void TerminationHandler(int signalNumber);
static int InitPeripheralsAndHandlers(void);
static void ClosePeripheralsAndHandlers(void);

// File descriptors - initialized to invalid value
static int i2cFd = -1;

// Termination state
static volatile sig_atomic_t terminationRequired = false;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    terminationRequired = true;
}

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static int InitPeripheralsAndHandlers(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    // Init I2C
    i2cFd = I2CMaster_Open(PROJECT_ISU2_I2C);
    if (i2cFd < 0) {
        Log_Debug("ERROR: I2CMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    int result = I2CMaster_SetBusSpeed(i2cFd, I2C_BUS_SPEED_STANDARD);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetBusSpeed: errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    result = I2CMaster_SetTimeout(i2cFd, 100);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetTimeout: errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    result = I2CMaster_SetDefaultTargetAddress(i2cFd, 0x3C);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetDefaultTargetAddress: errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    close(i2cFd);
}

/// <summary>
///     HDC1000
/// </summary>
void hdc1000Main(void) {

    uint16_t data16;
    double ddata;
    hdc1000_t* hdc;

    Log_Debug("Open HDC\n");
    hdc = hdc1000_open(i2cFd, HDC1000_I2C_ADDR, -1);

    data16 = hdc1000_get_dev_id(hdc);
    Log_Debug("Device ID: 0x%04x\n", data16);

    data16 = hdc1000_get_mf_id(hdc);
    Log_Debug("Manufacturer ID: 0x%04x\n", data16);

    data16 = hdc1000_get_config(hdc);
    Log_Debug("Config: 0x%04x\n", data16);

    ddata = hdc1000_get_temp(hdc);
    Log_Debug("Temperature [degC]: %f\n", ddata);

    ddata = hdc1000_get_humi(hdc);
    Log_Debug("Humidity [percRH]: %f\n", ddata);

    Log_Debug("Close HDC\n");
    hdc1000_close(hdc);
}

/// <summary>
///     Application entry point
/// </summary>
int main(void) {
    Log_Debug("\n*** Starting ***\n");

    if (InitPeripheralsAndHandlers() != 0) {
        terminationRequired = true;
    }

    if (!terminationRequired) {
        hdc1000Main();
    }

    Log_Debug("*** Terminating ***\n");
    ClosePeripheralsAndHandlers();
    return 0;
}
