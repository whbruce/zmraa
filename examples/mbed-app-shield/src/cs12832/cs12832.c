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
#include "cs12832.h"
#include "font_7.h"

static mraa_spi_context spi;
static mraa_gpio_context gpio_lcd_a0;
static mraa_gpio_context gpio_lcd_reset;
static unsigned char buffer[512];
static unsigned int orientation;
static unsigned int char_x;
static unsigned int char_y;
static unsigned char buffer[512];
static unsigned int contrast;
static unsigned int auto_up;
static unsigned char* font;
static unsigned int draw_mode;


static void wr_cmd(unsigned char cmd)
{
    mraa_gpio_write(gpio_lcd_a0, 0);
    mraa_spi_cs_enable(spi);
    mraa_spi_write(spi, cmd);
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

void c12832_set_font(unsigned char* f)
{
    font = f;
}



// reset and init the lcd controller
void cs12832_init(int bus, int lcd_a0_pin, int lcd_reset_pin)
{
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

    c12832_locate(0, 0);
    c12832_set_font((unsigned char*)small_7);  // standard font
    memset(buffer, 0x00, 512);
    copy_to_lcd();
}

void cs12832_fill(unsigned char value)
{
    memset(buffer, value, 512);
    copy_to_lcd();
}

int c12832_width()
{
    if (orientation == 0 || orientation == 2) return 32;
    else return 128;
}

int c12832_height()
{
    if (orientation == 0 || orientation == 2) return 128;
    else return 32;
}


void c12832_invert(unsigned int o)
{
    if(o == 0) wr_cmd(0xA6);
    else wr_cmd(0xA7);
}


void c12832_set_contrast(unsigned int o)
{
    contrast = o;
    wr_cmd(0x81);      //  set volume
    wr_cmd(o & 0x3F);
}

unsigned int c12832_get_contrast(void)
{
    return(contrast);
}

void c12832_cls(void)
{
    memset(buffer,0x00,512);  // clear display buffer
    copy_to_lcd();
}

void c12832_pixel(int x, int y, int color)
{
    // first check parameter
    if(x > 128 || y > 32 || x < 0 || y < 0) return;

    if(draw_mode == NORMAL) {
        if(color == 0)
            buffer[x + ((y/8) * 128)] &= ~(1 << (y%8));  // erase pixel
        else
            buffer[x + ((y/8) * 128)] |= (1 << (y%8));   // set pixel
    } else { // XOR mode
        if(color == 1)
            buffer[x + ((y/8) * 128)] ^= (1 << (y%8));   // xor pixel
    }
}

void c12832_line(int x0, int y0, int x1, int y1, int color)
{
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    //  if (dx == 0) {        /* vertical line */
    //      if (y1 > y0) vline(x0,y0,y1,color);
    //      else vline(x0,y1,y0,color);
    //      return;
    //  }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    //  if (dy == 0) {        /* horizontal line */
    //      if (x1 > x0) hline(x0,x1,y0,color);
    //      else  hline(x1,x0,y0,color);
    //      return;
    //  }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            c12832_pixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        c12832_pixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            c12832_pixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        c12832_pixel(x0, y0, color);
    }
    if(auto_up) copy_to_lcd();
}

void c12832_rect(int x0, int y0, int x1, int y1, int color)
{

    if (x1 > x0) c12832_line(x0,y0,x1,y0,color);
    else  c12832_line(x1,y0,x0,y0,color);

    if (y1 > y0) c12832_line(x0,y0,x0,y1,color);
    else c12832_line(x0,y1,x0,y0,color);

    if (x1 > x0) c12832_line(x0,y1,x1,y1,color);
    else  c12832_line(x1,y1,x0,y1,color);

    if (y1 > y0) c12832_line(x1,y0,x1,y1,color);
    else c12832_line(x1,y1,x1,y0,color);

    if(auto_up) copy_to_lcd();
}

void c12832_fillrect(int x0, int y0, int x1, int y1, int color)
{
    int l,c,i;
    if(x0 > x1) {
        i = x0;
        x0 = x1;
        x1 = i;
    }

    if(y0 > y1) {
        i = y0;
        y0 = y1;
        y1 = i;
    }

    for(l = x0; l<= x1; l ++) {
        for(c = y0; c<= y1; c++) {
            c12832_pixel(l,c,color);
        }
    }
    if(auto_up) copy_to_lcd();
}



void c12832_circle(int x0, int y0, int r, int color)
{

    int draw_x0, draw_y0;
    int draw_x1, draw_y1;
    int draw_x2, draw_y2;
    int draw_x3, draw_y3;
    int draw_x4, draw_y4;
    int draw_x5, draw_y5;
    int draw_x6, draw_y6;
    int draw_x7, draw_y7;
    int xx, yy;
    int di;
    //WindowMax();
    if (r == 0) {       /* no radius */
        return;
    }

    draw_x0 = draw_x1 = x0;
    draw_y0 = draw_y1 = y0 + r;
    if (draw_y0 < c12832_height()) {
        c12832_pixel(draw_x0, draw_y0, color);     /* 90 degree */
    }

    draw_x2 = draw_x3 = x0;
    draw_y2 = draw_y3 = y0 - r;
    if (draw_y2 >= 0) {
        c12832_pixel(draw_x2, draw_y2, color);    /* 270 degree */
    }

    draw_x4 = draw_x6 = x0 + r;
    draw_y4 = draw_y6 = y0;
    if (draw_x4 < c12832_width()) {
        c12832_pixel(draw_x4, draw_y4, color);     /* 0 degree */
    }

    draw_x5 = draw_x7 = x0 - r;
    draw_y5 = draw_y7 = y0;
    if (draw_x5>=0) {
        c12832_pixel(draw_x5, draw_y5, color);     /* 180 degree */
    }

    if (r == 1) {
        return;
    }

    di = 3 - 2*r;
    xx = 0;
    yy = r;
    while (xx < yy) {

        if (di < 0) {
            di += 4*xx + 6;
        } else {
            di += 4*(xx - yy) + 10;
            yy--;
            draw_y0--;
            draw_y1--;
            draw_y2++;
            draw_y3++;
            draw_x4--;
            draw_x5++;
            draw_x6--;
            draw_x7++;
        }
        xx++;
        draw_x0++;
        draw_x1--;
        draw_x2++;
        draw_x3--;
        draw_y4++;
        draw_y5++;
        draw_y6--;
        draw_y7--;

        if ( (draw_x0 <= c12832_width()) && (draw_y0>=0) ) {
            c12832_pixel(draw_x0, draw_y0, color);
        }

        if ( (draw_x1 >= 0) && (draw_y1 >= 0) ) {
            c12832_pixel(draw_x1, draw_y1, color);
        }

        if ( (draw_x2 <= c12832_width()) && (draw_y2 <= c12832_height()) ) {
            c12832_pixel(draw_x2, draw_y2, color);
        }

        if ( (draw_x3 >=0 ) && (draw_y3 <= c12832_height()) ) {
            c12832_pixel(draw_x3, draw_y3, color);
        }

        if ( (draw_x4 <= c12832_width()) && (draw_y4 >= 0) ) {
            c12832_pixel(draw_x4, draw_y4, color);
        }

        if ( (draw_x5 >= 0) && (draw_y5 >= 0) ) {
            c12832_pixel(draw_x5, draw_y5, color);
        }
        if ( (draw_x6 <=c12832_width()) && (draw_y6 <= c12832_height()) ) {
            c12832_pixel(draw_x6, draw_y6, color);
        }
        if ( (draw_x7 >= 0) && (draw_y7 <= c12832_height()) ) {
            c12832_pixel(draw_x7, draw_y7, color);
        }
    }
    if(auto_up) copy_to_lcd();
}

void c12832_fillcircle(int x, int y, int r, int color)
{
    int i,up;
    up = auto_up;
    auto_up = 0;   // off
    for (i = 0; i <= r; i++)
        c12832_circle(x,y,i,color);
    auto_up = up;
    if(auto_up) copy_to_lcd();
}

void c12832_setmode(int mode)
{
    draw_mode = mode;
}

void c12832_locate(int x, int y)
{
    char_x = x;
    char_y = y;
}



int c12832_columns()
{
    return c12832_width() / font[1];
}



int c12832_rows()
{
    return c12832_height() / font[2];
}


void c12832_character(int x, int y, int c)
{
    unsigned int hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line

    if (char_x + hor > c12832_width()) {
        char_x = 0;
        char_y = char_y + vert;
        if (char_y >= c12832_height() - font[2]) {
            char_y = 0;
        }
    }

    zeichen = &font[((c -32) * offset) + 4]; // start of char bitmap
    w = zeichen[0];                          // width of actual char
    // construct the char into the buffer
    for (j=0; j<vert; j++) {  //  vert line
        for (i=0; i<hor; i++) {   //  horz line
            z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00) {
                c12832_pixel(x+i,y+j,0);
            } else {
                c12832_pixel(x+i,y+j,1);
            }

        }
    }

    char_x += w;
}

int c12832_putc(char value)
{
    if (value == '\n') {    // new line
        char_x = 0;
        char_y = char_y + font[2];
        if (char_y >= c12832_height() - font[2]) {
            char_y = 0;
        }
    } else {
        c12832_character(char_x, char_y, value);
        if(auto_up) copy_to_lcd();
    }
    return value;
}

void c12832_puts(char* str)
{
    while (*str)
       c12832_putc(*str++);
 }

void c12832_set_auto_up(unsigned int up)
{
    if(up ) auto_up = 1;
    else auto_up = 0;
}

unsigned int c12832_get_auto_up(void)
{
    return (auto_up);
}

void c12832_print_bm(Bitmap bm, int x, int y)
{
    int h,v,b;
    char d;

    for(v=0; v < bm.ySize; v++) {   // lines
        for(h=0; h < bm.xSize; h++) { // pixel
            if(h + x > 127) break;
            if(v + y > 31) break;
            d = bm.data[bm.Byte_in_Line * v + ((h & 0xF8) >> 3)];
            b = 0x80 >> (h & 0x07);
            if((d & b) == 0) {
                c12832_pixel(x+h,y+v,0);
            } else {
                c12832_pixel(x+h,y+v,1);
            }
        }
    }

}


