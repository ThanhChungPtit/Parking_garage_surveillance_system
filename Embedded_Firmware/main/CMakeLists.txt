idf_component_register(SRCS "sensor_system.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver my_wifi my_mqtt LCD SERVO RFID SPI DHT)
