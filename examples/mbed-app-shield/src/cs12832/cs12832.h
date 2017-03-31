#pragma once

void cs12832_init(int bus, int lcd_a0_pin, int lcd_reset_pin);
void cs12832_fill(unsigned int value);