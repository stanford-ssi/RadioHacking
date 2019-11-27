/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef DRIVER_INIT_INCLUDED
#define DRIVER_INIT_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_atomic.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_sleep.h>

#include <hal_adc_sync.h>

#include <hal_ext_irq.h>

#include <hal_spi_m_sync.h>

#include <hal_usart_sync.h>

#include <hal_spi_m_os.h>

#include <hal_i2c_m_sync.h>

#include <hal_mci_sync.h>

#include <hal_delay.h>

#include "hal_usb_device.h"

extern struct adc_sync_descriptor   ADC_0;

extern struct spi_m_os_descriptor SPI_SENSOR;

extern struct usart_sync_descriptor USART_ESP;

extern struct spi_m_os_descriptor SPI_SQUIB;

extern struct i2c_m_sync_desc I2C_BUS2;

extern struct mci_sync_desc MCI_0;

void ADC_0_PORT_init(void);
void ADC_0_CLOCK_init(void);
void ADC_0_init(void);

void SPI_SENSOR_PORT_init(void);
void SPI_SENSOR_CLOCK_init(void);
void SPI_SENSOR_init(void);

void USART_ESP_PORT_init(void);
void USART_ESP_CLOCK_init(void);
void USART_ESP_init(void);

void SPI_SQUIB_PORT_init(void);
void SPI_SQUIB_CLOCK_init(void);
void SPI_SQUIB_init(void);

void I2C_BUS2_CLOCK_init(void);
void I2C_BUS2_init(void);
void I2C_BUS2_PORT_init(void);

void MCI_0_PORT_init(void);
void MCI_0_CLOCK_init(void);
void MCI_0_init(void);

void delay_driver_init(void);

void USB_0_CLOCK_init(void);
void USB_0_init(void);

/**
 * \brief Perform system initialization, initialize pins and clocks for
 * peripherals
 */
void system_init(void);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_INIT_INCLUDED