#ifndef __DHT_H_
#define __DHT_H_

#include "rom/ets_sys.h"
#include "esp_err.h" 
#define DHT11_PIN 25

int get_signal_level(int us_time, int level) ;

esp_err_t read_dht11(float *temp, float *hum) ;




#endif