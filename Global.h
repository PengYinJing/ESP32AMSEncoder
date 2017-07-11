/*
 * Global.h
 * 
 * Global header for controlling AS5x47 
 * 
 * Created by: 
 *       K.C.Y
 * Date:
 *       2017/06/30
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* MEMS Microphone GPIO */
#define MIC_CLK_IO_NUM            22
#define MIC_DATA_IO_NUM           21

/* AS5x47 GPIO */
#define AS5X47_A_IO_NUM           12
#define AS5X47_B_IO_NUM           13
#define AS5X47_I_IO_NUM           14

#define AS5X47_SPI_CS_IO_NUM      4
#define AS5X47_SPI_SCK_IO_NUM     18
#define AS5X47_SPI_MOSI_IO_NUM    23
#define AS5X47_SPI_MISO_IO_NUM    19

/* Preference Keys */
#define PREF_SECTION_WIFI     "wifi"
#define PREF_KEY_SSID         "ssid"
#define PREF_KEY_PASSWORD     "password"
#define PREF_KEY_ORG          "org"
#define PREF_KEY_DEV_TYPE     "devtype"
#define PREF_KEY_DEV_ID       "devid"
#define PREF_KEY_TOKEN        "token"
#define PREF_KEY_FWVERSION    "version"
#define PREF_KEY_REQ_ID       "req"

#define TEST_MQTT_MODE      0
#define TEST_MQTT_SERVER    "192.168.1.123"
#define TEST_MQTT_CLIENT_ID "ESP32-MQTT-CLIENT"

#endif

