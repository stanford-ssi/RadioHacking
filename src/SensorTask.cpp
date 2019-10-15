#include "SensorTask.hpp"

TaskHandle_t SensorTask::taskHandle = NULL;
StaticTask_t SensorTask::xTaskBuffer;
StackType_t SensorTask::xStack[stackSize];

SensorTask::SensorTask()
{
    SensorTask::taskHandle = xTaskCreateStatic(activity,                  //static function to run
                                               "Sensors",                 //task name
                                               stackSize,                 //stack depth (words!)
                                               NULL,                      //parameters
                                               1,                         //priority
                                               SensorTask::xStack,        //stack object
                                               &SensorTask::xTaskBuffer); //TCB object
}

TaskHandle_t SensorTask::getTaskHandle()
{
    return taskHandle;
}

void SensorTask::activity(void *ptr)
{
    sys.tasks.logger.log("Initializing Sensors");

    int rc;
    char str[100];

    vTaskDelay(2); //but why...

    sys.sensors.adxl1.init();
    sys.sensors.adxl1.startMeasuring();

    vTaskDelay(2); //but why...

    rc = sys.sensors.pres1.init();

    if (rc != true)
    {
        sys.tasks.logger.log("Error Starting BMP388");
    }

    gpio_set_pin_level(ACCEL_CS_1, false);
    vTaskDelay(1);
    gpio_set_pin_level(ACCEL_CS_1, true);
    vTaskDelay(1);

    rc = sys.sensors.imu1.accel->begin();

    if (rc != 1)
    {
        snprintf(str, sizeof(str), "Error Starting BMI088Accel: %i", rc);
        sys.tasks.logger.log(str);
    }

    vTaskDelay(2); //but why...

    rc = sys.sensors.imu1.gyro->begin();

    if (rc != 1)
    {
        snprintf(str, sizeof(str), "Error Starting BMI088Gyro: %i", rc);
        sys.tasks.logger.log(str);
    }

    TickType_t lastSensorTime = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&lastSensorTime, 10);

        gpio_set_pin_level(SENSOR_LED, true);

        StaticJsonDocument<1000> sensor_json;

        SensorData data;

        data.adxl375_data = sys.sensors.adxl1.readSensor();
        data.bmp388_data = sys.sensors.pres1.readSensor();
        vTaskDelay(2); //but why...
        data.bmi088accel_data = sys.sensors.imu1.accel->readSensor();
        vTaskDelay(2); //but why...
        data.bmi088gyro_data = sys.sensors.imu1.gyro->readSensor();

        data.tick = xTaskGetTickCount();

        sensor_json["tick"] = data.tick;

        JsonObject bmp388_json = sensor_json.createNestedObject("bmp");
        JsonObject bmi088_json = sensor_json.createNestedObject("bmi");
        JsonObject adxl375_json = sensor_json.createNestedObject("adxl");

        bmp388_json["p"] = data.bmp388_data.pressure;
        bmp388_json["t"] = data.bmp388_data.temperature;

        JsonArray bmi_accel_json = bmi088_json.createNestedArray("a");
        bmi_accel_json.add(data.bmi088accel_data.x);
        bmi_accel_json.add(data.bmi088accel_data.y);
        bmi_accel_json.add(data.bmi088accel_data.z);

        JsonArray bmi_gyro_json = bmi088_json.createNestedArray("g");
        bmi_gyro_json.add(data.bmi088gyro_data.x);
        bmi_gyro_json.add(data.bmi088gyro_data.y);
        bmi_gyro_json.add(data.bmi088gyro_data.z);

        JsonArray adxl_accel_json = adxl375_json.createNestedArray("a");
        adxl_accel_json.add(data.adxl375_data.x);
        adxl_accel_json.add(data.adxl375_data.y);
        adxl_accel_json.add(data.adxl375_data.z);

        sys.tasks.logger.logJSON(sensor_json, "sensor");

        gpio_set_pin_level(SENSOR_LED, false);
    }
}