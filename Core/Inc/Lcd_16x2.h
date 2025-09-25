/*
 * Lcd_16x2.h
 *
 *  Created on: Feb 21, 2024
 *      Author: maulin
 */

#ifndef INC_LCD_16X2_H_
#define INC_LCD_16X2_H_

#define LCD_I2C_ADDRESS 0x3C // change this according to ur setup

char lcd_write_command(uint8_t command);
void lcd_write_data(uint8_t data) ;
void lcd_clear();
void lcd_display_string(const char *string);
void lcd_initialize();
void lcd_set_cursor(uint8_t row, uint8_t  col);
void lcd_float_print(float value, int precision);

#endif /* INC_LCD_16X2_H_ */
