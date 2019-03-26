#include <atmel_start.h>
#include "ADXL375.hpp"
#include "BMI088.hpp"
#include "ArduinoJson.h"
#include "sdtester.c"
#include "printf.h"
#include "BMP3xx.hpp"
#include "SPITest.hpp"
#include "Squib.hpp"
#include "Battery.hpp"

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	gpio_set_pin_level(LED1, true);
	gpio_set_pin_level(LED2, true);
	gpio_set_pin_level(LED3, true);
	gpio_set_pin_level(LED4, true);

	delay_ms(1000);

	gpio_set_pin_level(LED1, false);
	gpio_set_pin_level(LED2, false);
	gpio_set_pin_level(LED3, false);
	gpio_set_pin_level(LED4, false);


	spi_m_sync_disable(&SPI_SENSOR);
	spi_m_sync_set_mode(&SPI_SENSOR, SPI_MODE_3);
	spi_m_sync_enable(&SPI_SENSOR);

	spi_m_sync_disable(&SPI_SQUIB);
	spi_m_sync_set_mode(&SPI_SQUIB, SPI_MODE_3);
	spi_m_sync_enable(&SPI_SQUIB);

	ADXL375 adxl375(&SPI_SENSOR,ADXL_CS_1);
	adxl375.init();
	adxl375.startMeasuring();

	BMI088Accel bmi088accel(&SPI_SENSOR,ACCEL_CS_1);
	bmi088accel.begin();

	BMI088Gyro bmi088gyro(&SPI_SENSOR,GYRO_CS_1);
	bmi088gyro.begin();

	BMP3xx bmp388(&SPI_SENSOR,BMP_CS_1);
	bmp388.begin();

	Battery battery(&ADC_0);

	delay_ms(1000);

	Squib squib(&SPI_SQUIB, SQUIB_CS);

	uint8_t leds[] = {LED1,LED2,LED3,LED4};

	uint8_t led = 0;

	bool fire = false;

	

	while (true)
	{
		gpio_toggle_pin_level(leds[led]);
		led++;
		led = led % 4;
		
		/*
		for(int k=0;k<100;k++){
			for(int i=0;i<10000;i++){}
			gpio_toggle_pin_level(BUZZER);
		}
		*/
		if(fire){
			squib.fire(CMD_FIRE_1A2A1B2B);
		}else{
			squib.fire(CMD_FIRE_NO_SQUIBS);
		}
		fire = !fire;

		//printf_("SD Test!");	
		//sdtester();

		squib.getStatus();
		Squib::Status status =  squib.status;
		BMI088Accel::Data accel = bmi088accel.readSensor();
		BMI088Gyro::Data gyro = bmi088gyro.readSensor();
		ADXL375::Data accelHigh = adxl375.readSensor();
		BMP3xx::Data pressure = bmp388.readSensor();
		Battery::cell_voltage_t cells = battery.readVoltage();

		DynamicJsonDocument doc(1024);

		/* JsonObject bmi088_json = doc.createNestedObject("BMI088");
		bmi088_json["temp"] = accel.temp;
		bmi088_json["time"] = accel.time;

		JsonObject bmi_accel_json = bmi088_json.createNestedObject("Accel");
		bmi_accel_json["x"] = accel.x;
		bmi_accel_json["y"] = accel.y;
		bmi_accel_json["z"] = accel.z;

		JsonObject bmi_gyro_json = bmi088_json.createNestedObject("Gryo");
		bmi_gyro_json["x"] = gyro.x;
		bmi_gyro_json["y"] = gyro.y;
		bmi_gyro_json["z"] = gyro.z;

		JsonObject adxl375_json = doc.createNestedObject("ADXL375");
		adxl375_json["x"] = accelHigh.x;
		adxl375_json["y"] = accelHigh.y;
		adxl375_json["z"] = accelHigh.z; */

		/*JsonObject bmp388_json = doc.createNestedObject("BMP388");
		bmp388_json["pres"] = pressure.pressure;
		bmp388_json["temp"] = pressure.temperature;*/

		/*JsonObject squib_json = doc.createNestedObject("Squib");
		squib_json["fen1"] = status.Squib_StatFen1;
		squib_json["fen2"] = status.Squib_StatFen2;
		squib_json["1A_res"] = status.Squib_Stat1AResistance;
		squib_json["1B_res"] = status.Squib_Stat1BResistance;
		squib_json["2A_res"] = status.Squib_Stat2AResistance;
		squib_json["2B_res"] = status.Squib_Stat2BResistance;*/

		JsonObject battery_json = doc.createNestedObject("Battery");
		battery_json["cellA"] = cells.cellA;
		battery_json["cellB"] = cells.cellB;
		battery_json["total"] = cells.total;

		char string[1000];
		serializeJson(doc,string,sizeof(string));

		printf_("%s\n",string);
		//delay_ms(50);
	}
}