#pragma once

class AltimeterTask;

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <hal_rtos.h>
#include "support/driver_init.h"
#include "printf.h"
#include "support/atmel_start_pins.h"
#include "ADXL375.hpp"
#include "BMP3xx.hpp"
#include "BMI088.hpp"
#include "main.hpp"
#include "ArduinoJson.h"

#define ALT_LED LED1

class AltimeterTask
{
private:
    static const size_t stackSize = 2000;

    static TaskHandle_t taskHandle;
    static StaticTask_t xTaskBuffer;
    static StackType_t xStack[stackSize];

    static void activity(void *p);

public:
    AltimeterTask();
    TaskHandle_t getTaskHandle();
};