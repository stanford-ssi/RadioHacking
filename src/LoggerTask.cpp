#include "LoggerTask.hpp"
#include "main.hpp"

TaskHandle_t LoggerTask::taskHandle = NULL;
StaticTask_t LoggerTask::xTaskBuffer;
StackType_t LoggerTask::xStack[stackSize];

MessageBufferHandle_t LoggerTask::bufferHandle;
StaticMessageBuffer_t LoggerTask::messageBufferStruct;
uint8_t LoggerTask::ucStorageBuffer[bufferSize];

char LoggerTask::lineBuffer[10000];
char LoggerTask::inputLineBuffer[1000];

bool LoggerTask::loggingEnabled = false;
bool LoggerTask::shitlEnabled = false;

FATFS LoggerTask::fs;
FIL LoggerTask::file_object;
FIL LoggerTask::shitl_file_object;

LoggerTask::LoggerTask()
{
    LoggerTask::taskHandle = xTaskCreateStatic(activity,                  //static function to run
                                               "Logger",                  //task name
                                               stackSize,                 //stack depth (words!)
                                               NULL,                      //parameters
                                               1,                        //priority
                                               LoggerTask::xStack,        //stack object
                                               &LoggerTask::xTaskBuffer); //TCB object

    LoggerTask::bufferHandle = xMessageBufferCreateStatic(bufferSize, LoggerTask::ucStorageBuffer, &LoggerTask::messageBufferStruct);
}

TaskHandle_t LoggerTask::getTaskHandle()
{
    return taskHandle;
}

void LoggerTask::log(const char *message)
{
    vPortEnterCritical();
    xMessageBufferSend(bufferHandle, message, strlen(message) + 1, 0);
    vPortExitCritical();
}

void LoggerTask::logJSON(JsonDocument & jsonDoc, const char* id){
    jsonDoc["id"] = id;
    jsonDoc["stack"] = uxTaskGetStackHighWaterMark(NULL);//TODO: Check this for capacity... (dangerous!)

    if (jsonDoc.getMember("tick") == NULL){
        jsonDoc["tick"] = xTaskGetTickCount();
    }

    //jsonDoc["la"] = xMessageBufferSpaceAvailable(bufferHandle);
    
    size_t len = measureJson(jsonDoc);
    char str[len+5]; //plenty of room!
    serializeJson(jsonDoc, str, sizeof(str));
    log(str);
}

void LoggerTask::format()
{
    FRESULT res;
    printf("-I- Format disk %d\n\r", 0);
    printf("-I- Please wait a moment during formatting...\n\r");
    //res = f_mkfs("0", FM_EXFAT, 512);
    printf("-I- Disk format finished !\n\r");
    if (res != FR_OK)
    {
        printf("-E- f_mkfs pb: 0x%X\n\r", res);
    }
}

void LoggerTask::activity(void *ptr)
{
    gpio_set_pin_level(DISK_LED, true);
    FRESULT res;

    //Clear file system object
    memset(&fs, 0, sizeof(FATFS));

    res = f_mount(&fs, "", 1);
    if (res != FR_OK)
    {
        loggingEnabled = false;
        printf("WARN-%s-%u: 0x%X\n\r", __FILE__, __LINE__, res);
    }
    else
    {
        loggingEnabled = true;
        printf("Mounted SD card, enabling logging\n");
    }

    char file_name[20];

    if (loggingEnabled)
    {
        int lognum = 0;
        while (true)
        {
            snprintf(file_name, sizeof(file_name), "log%u.txt", lognum);
            res = f_stat(file_name, NULL);

            if (res == FR_NO_FILE)
            {
                break;
            }

            if (res != FR_OK)
            {
                printf("WARN-%s-%u: 0x%X\n\r", __FILE__, __LINE__, res);
                printf("->Error stat-ing file: %s\n", file_name);
                loggingEnabled = false;
                break;
            }

            lognum++;
        }
    }

    if (loggingEnabled)
    {
        printf("Logging to file: %s\n", file_name);

        res = f_open(&file_object, file_name, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            printf("WARN-%s-%u: 0x%X\n\r", __FILE__, __LINE__, res);
            loggingEnabled = false;
        }

        //SHITL-----

        printf("SHITL from file: shitl.txt\n");

        res = f_open(&shitl_file_object, "shitl.txt", FA_READ);
        if (res == FR_OK)
        {
            shitlEnabled = true;
        }
        else
        {
            shitlEnabled = false;
            printf("WARN-%s-%u: 0x%X\n\r", __FILE__, __LINE__, res);
        }

    }

    gpio_set_pin_level(DISK_LED, false);

    char* p = lineBuffer;
    TickType_t timeout = 0;
    while (true)
    {
        //Step 1: read in all the logs
        if (xMessageBufferReceive(bufferHandle, p, 1000, shitlEnabled ? 0 : NEVER) > 0)
        {
            p = lineBuffer + strlen(lineBuffer);

            p[0] = '\n';
            p++;
            p[0] = '\0';

            if(p - lineBuffer > 8999 || xTaskGetTickCount() > timeout ){ //we need to write!
                //Step 2: Write to USB
                writeUSB(lineBuffer);
                    
                //Step 3: Write to SD card
                if (loggingEnabled){
                    writeSD(lineBuffer);
                }

                //reset buffer
                lineBuffer[0] = '\0';
                p = lineBuffer;
                timeout = xTaskGetTickCount() + 1000;
            }
        
        }else{
            readSHITL();
        }
    }
}

void LoggerTask::readSHITL(){

    gpio_set_pin_level(SENSOR_LED, true);
    //read in next line
    f_gets(inputLineBuffer, sizeof(inputLineBuffer), &shitl_file_object);

    StaticJsonDocument<1024> sensor_json;

    SensorData data;

    if(deserializeJson(sensor_json, inputLineBuffer) != DeserializationError::Ok){
        printf("Parsing Error!\n");
    }else{
        //need try catch?
        data.adxl1_data.y = 1;
        data.bmp1_data.pressure = 1;
        data.bmp2_data.pressure = 1;

        sys.tasks.filter.queueSensorData(data);
    }

    gpio_set_pin_level(SENSOR_LED, false);
}

void LoggerTask::writeUSB(char * buf){
    char endl = '\n';
    uint32_t len = strlen(lineBuffer);
    for(uint32_t i = 0; i < len; i++){
        write_byte(0, &lineBuffer[i], 1);
    }
    write_byte(0, &endl , 1);
}

void LoggerTask::writeSD(char* buf){
    gpio_set_pin_level(DISK_LED, true);

    FRESULT res;
    UINT writen;
    res = f_write(&file_object, lineBuffer, strlen(lineBuffer), &writen);

    if (res != FR_OK)
    {
        printf("WARN-%s-%u: 0x%X\n\r", __FILE__, __LINE__, res);
    }

    res = f_sync(&file_object); //the file is still saved every for each sector, which is pretty fast... (false!)
    if (res != FR_OK)
    {
        printf("WARN-%s-%u: 0x%X\n\r", __FILE__, __LINE__, res);
    }
    
    gpio_set_pin_level(DISK_LED, false);
}