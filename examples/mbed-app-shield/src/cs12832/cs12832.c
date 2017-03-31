/*
 * Author: Henry Bruce <henry.bruce@intel.com>
 * Copyright (c) 2016 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include "mraa.h"

// #define GPIO_SPI_CS                     10

static mraa_spi_context spi;
// static mraa_gpio_context gpio_spi_cs;
static mraa_gpio_context gpio_lcd_a0;
static mraa_gpio_context gpio_lcd_reset;
static unsigned char buffer[512];


static void wr_cmd(unsigned char cmd)
{
    mraa_gpio_write(gpio_lcd_a0, 0);
    // mraa_gpio_write(gpio_spi_cs, 0);
    mraa_spi_cs_enable(spi);
    mraa_spi_write(spi, cmd);
    // mraa_gpio_write(gpio_spi_cs, 1);
    mraa_spi_cs_disable(spi);
}


// write data to lcd controller
static void wr_dat(unsigned char dat)
{
    mraa_gpio_write(gpio_lcd_a0, 1);
    mraa_spi_cs_enable(spi);
    mraa_spi_write(spi, dat);
    mraa_spi_cs_enable(spi);
}


static void copy_to_lcd(void)
{

    int i=0;

    //page 0
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB0);      // set page address  0
    // _A0 = 1;
    mraa_gpio_write(gpio_lcd_a0, 1);
    for(i=0; i<128; i++) {
        wr_dat(buffer[i]);
    }

    // page 1
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB1);      // set page address  1
    // _A0 = 1;
    mraa_gpio_write(gpio_lcd_a0, 1);
    for(i=128; i<256; i++) {
        wr_dat(buffer[i]);
    }

    //page 2
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB2);      // set page address  2
    // _A0 = 1;
    mraa_gpio_write(gpio_lcd_a0, 1);
    for(i=256; i<384; i++) {
        wr_dat(buffer[i]);
    }

    //page 3
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB3);      // set page address  3
    // _A0 = 1;
    mraa_gpio_write(gpio_lcd_a0, 1);

    // _CS = 0;

    for(i=384; i<512; i++) {
        wr_dat(buffer[i]);
    }

}


// reset and init the lcd controller
void cs12832_init(int bus, int lcd_a0_pin, int lcd_reset_pin)
{
/*
    gpio_spi_cs = mraa_gpio_init(GPIO_SPI_CS);
    if (gpio_spi_cs == NULL) {
        printf("Could not open GPIO_SPI_CS gpio\n");
        return;
    }
*/
    gpio_lcd_a0 = mraa_gpio_init(lcd_a0_pin);
    if (gpio_lcd_a0 == NULL) {
        printf("Could not open GPIO_SPI_CS gpio\n");
        return;
    }

    gpio_lcd_reset = mraa_gpio_init(lcd_reset_pin);
    if (gpio_lcd_reset == NULL) {
        printf("Could not open GPIO_SPI_RESET gpio\n");
        return;
    }

    // mraa_gpio_dir(gpio_spi_cs, MRAA_GPIO_OUT);
    mraa_gpio_dir(gpio_lcd_a0, MRAA_GPIO_OUT);
    mraa_gpio_dir(gpio_lcd_reset, MRAA_GPIO_OUT);

    spi = mraa_spi_init(bus);
    if (spi == NULL) {
        printf("Could not open SPI\n");
        return;
    }

    if (mraa_spi_mode(spi, MRAA_SPI_MODE3) != MRAA_SUCCESS) {
        printf("mraa_spi_mode failed\n");
        return;

    }

    if (mraa_spi_bit_per_word(spi, 8) != MRAA_SUCCESS) {
        printf("mraa_spi_bit_per_word failed\n");
        return;
    }

    if (mraa_spi_frequency(spi, 5000000) != MRAA_SUCCESS) {
        printf("mraa_spi_frequency failed\n");
        return;
    }

    mraa_gpio_write(gpio_lcd_a0, 0);
    mraa_gpio_write(gpio_lcd_reset, 0);
    k_sleep(1);
    mraa_gpio_write(gpio_lcd_reset, 1);
    k_sleep(5);
    wr_cmd(0xAE);   //  display off
    wr_cmd(0xA2);   //  bias voltage

    wr_cmd(0xA0);
    wr_cmd(0xC8);   //  colum normal

    wr_cmd(0x22);   //  voltage resistor ratio
    wr_cmd(0x2F);   //  power on
    //wr_cmd(0xA4);   //  LCD display ram
    wr_cmd(0x40);   // start line = 0
    wr_cmd(0xAF);     // display ON

    wr_cmd(0x81);   //  set contrast
    wr_cmd(0x17);   //  set contrast

    wr_cmd(0xA6);     // display normal


    memset(buffer, 0x00, 512);
    copy_to_lcd();
}

void cs12832_fill(unsigned char value)
{
    memset(buffer, value, 512);
    copy_to_lcd();
}
