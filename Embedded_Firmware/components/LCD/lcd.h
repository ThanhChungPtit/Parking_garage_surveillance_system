#ifndef __LCD_H
#define __LCD_H
#include "driver/i2c.h"
#include <stdio.h>
#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BACKLIGHT 0x08


#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME   0x02
#define LCD_ENTRY_MODE    0x06
#define LCD_DISPLAY_ON    0x0C
#define LCD_FUNCTION_SET  0x28

typedef struct {
    int LCD_SDA ;
    int LCD_SCL ;
    i2c_port_t I2C_MASTER_NUM ;
    int I2C_MASTER_FREQ_HZ ;
    int LCD_ADDRESS ;
} LCD_Typedef ;

void lcd_send(LCD_Typedef *LCD,uint8_t data) ;

void lcd_send_data(LCD_Typedef *LCD,uint8_t data) ;
void lcd_send_cmd(LCD_Typedef *LCD,uint8_t cmd) ;
void lcd_send_string(LCD_Typedef *LCD,const char *str) ;
void lcd_set_cursor(LCD_Typedef* LCD ,uint8_t row, uint8_t col) ;
void lcd_clear(LCD_Typedef* LCD) ;
void lcd_init(LCD_Typedef *LCD) ;
void i2c_master_init(LCD_Typedef *LCD,int SDA_PIN,int SCL_PIN,i2c_port_t I2C_NUM,int I2C_FREQ_HZ,int LCD_ADDRESS) ;



#endif 