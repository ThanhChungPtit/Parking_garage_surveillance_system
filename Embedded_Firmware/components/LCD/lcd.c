#include "lcd.h"


void lcd_send(LCD_Typedef *LCD,uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create() ;
    i2c_master_start(cmd) ;//start signal
    i2c_master_write_byte(cmd,(LCD->LCD_ADDRESS<<1 )|(I2C_MASTER_WRITE),true) ;
    i2c_master_write_byte(cmd,data,true) ;
    i2c_master_stop(cmd) ;
    i2c_master_cmd_begin(LCD->I2C_MASTER_NUM,cmd,pdMS_TO_TICKS(50)) ;
    i2c_cmd_link_delete(cmd) ;
}

/*void lcd_send_data(LCD_Typedef *LCD,uint8_t data) {
    uint8_t high=data & 0xF0 ;
    uint8_t low=(data << 4) & 0xF0 ;
    lcd_send(LCD,high | LCD_BACKLIGHT | LCD_EN | LCD_RS) ;
    lcd_send(LCD,high|LCD_BACKLIGHT|LCD_RS) ;
    lcd_send(LCD,low|LCD_BACKLIGHT|LCD_EN|LCD_RS) ;
    lcd_send(LCD,low|LCD_BACKLIGHT|LCD_RS) ;
    vTaskDelay(pdMS_TO_TICKS(10)) ;
}

void lcd_send_cmd(LCD_Typedef *LCD,uint8_t cmd) {
    uint8_t high=cmd & 0xF0 ;
    uint8_t low=(cmd << 4) & 0xF0 ;
    lcd_send(LCD,high | LCD_BACKLIGHT | LCD_EN) ;
    lcd_send(LCD,high|LCD_BACKLIGHT) ;
    lcd_send(LCD,low|LCD_BACKLIGHT|LCD_EN) ;
    lcd_send(LCD,low|LCD_BACKLIGHT) ;
    vTaskDelay(pdMS_TO_TICKS(10)) ;
}*/
void lcd_send_cmd(LCD_Typedef *LCD, uint8_t cmd) {
    uint8_t high = cmd & 0xF0;
    uint8_t low = (cmd << 4) & 0xF0;
    
    // Đóng gói 4 trạng thái của chân EN vào 1 mảng duy nhất
    uint8_t data_arr[4];
    data_arr[0] = high | LCD_BACKLIGHT | LCD_EN;
    data_arr[1] = high | LCD_BACKLIGHT;
    data_arr[2] = low | LCD_BACKLIGHT | LCD_EN;
    data_arr[3] = low | LCD_BACKLIGHT;

    // Gửi toàn bộ 1 lần qua I2C (Bắn 1 phát ăn ngay, không bị Task khác cắt ngang)
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (LCD->LCD_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, data_arr, 4, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(LCD->I2C_MASTER_NUM, handle, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(handle);
    
    vTaskDelay(pdMS_TO_TICKS(2)); // Chỉ delay một chút cho các lệnh hệ thống
}

void lcd_send_data(LCD_Typedef *LCD, uint8_t data) {
    uint8_t high = data & 0xF0;
    uint8_t low = (data << 4) & 0xF0;
    
    // có thêm cờ LCD_RS để báo đây là Data (chữ viết)
    uint8_t data_arr[4];
    data_arr[0] = high | LCD_BACKLIGHT | LCD_EN | LCD_RS;
    data_arr[1] = high | LCD_BACKLIGHT | LCD_RS;
    data_arr[2] = low | LCD_BACKLIGHT | LCD_EN | LCD_RS;
    data_arr[3] = low | LCD_BACKLIGHT | LCD_RS;

    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (LCD->LCD_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, data_arr, 4, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(LCD->I2C_MASTER_NUM, handle, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(handle);
}
void lcd_send_string(LCD_Typedef *LCD,const char *str) {
    while(*str) {
        lcd_send_data(LCD,*str) ;
        str++ ;
    }
}
void lcd_init(LCD_Typedef *LCD) {

    vTaskDelay(pdMS_TO_TICKS(50)) ;
    lcd_send_cmd(LCD,0x30) ;
    vTaskDelay(pdMS_TO_TICKS(5)) ;
    lcd_send_cmd(LCD,0x30) ;
    vTaskDelay(pdMS_TO_TICKS(5)) ;
    lcd_send_cmd(LCD,0x30) ;
    vTaskDelay(pdMS_TO_TICKS(5)) ;
    lcd_send_cmd(LCD,0x20) ;
    vTaskDelay(pdMS_TO_TICKS(5)) ;
    lcd_send_cmd(LCD,0x28) ;//4 bit-mode
    vTaskDelay(pdMS_TO_TICKS(10)) ;
    lcd_send_cmd(LCD,LCD_FUNCTION_SET) ;
    lcd_send_cmd(LCD,LCD_DISPLAY_ON) ;
    lcd_send_cmd(LCD,LCD_CLEAR_DISPLAY) ;
    vTaskDelay(pdMS_TO_TICKS(5)) ;
    lcd_send_cmd(LCD,LCD_ENTRY_MODE) ;//entry mode
}
void lcd_set_cursor(LCD_Typedef* LCD ,uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_send_cmd(LCD,addr);
}
void lcd_clear(LCD_Typedef* LCD) {
    lcd_send_cmd(LCD,LCD_CLEAR_DISPLAY) ;
    vTaskDelay(pdMS_TO_TICKS(5)) ;
}

void i2c_master_init(LCD_Typedef *LCD,int SDA_PIN,int SCL_PIN,i2c_port_t I2C_NUM,int I2C_FREQ_HZ,int LCD_ADDRESS)
{
    //Init LCD
    LCD->LCD_SDA=SDA_PIN ;
    LCD->LCD_SCL=SCL_PIN ;
    LCD->I2C_MASTER_NUM=I2C_NUM ;
    LCD->I2C_MASTER_FREQ_HZ=I2C_FREQ_HZ ;
    LCD->LCD_ADDRESS=LCD_ADDRESS ;//(0x27) 

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = LCD->LCD_SDA,
        .scl_io_num = LCD->LCD_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = LCD->I2C_MASTER_FREQ_HZ,
    };
    (i2c_param_config(LCD->I2C_MASTER_NUM, &conf));
    (i2c_driver_install(LCD->I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}
