#pragma once

#define SENSOR_LED LED2

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <hal_rtos.h>

class SensorTask
{
private:
    static const size_t stackSize = 2000;

    static TaskHandle_t taskHandle;
    static StaticTask_t xTaskBuffer;
    static StackType_t xStack[stackSize];

    static void activity(void *p);

public:
    SensorTask();
    TaskHandle_t getTaskHandle();
};

#include "main.hpp"
#include "driver_init.h"
#include "printf.h"
#include "atmel_start_pins.h"
#include "periph/ADXL375/ADXL375.hpp"
#include "periph/BMP388/BMP388.hpp"
#include "periph/BMI088/BMI088.hpp"
#include "ArduinoJson.h"
#include "AltFilterTask.h"
#include "AltFilter.hpp"