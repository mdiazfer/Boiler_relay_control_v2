/* 
    Definition of global parameters
    No user modification is required in here
*/
#include <Arduino.h>
#include "user_setup.h"

//Generic definitions
#define BUILD_TYPE_PRODUCTION   0
#define BUILD_TYPE_DEVELOPMENT  1
#define VERSION "1.1.0"
#define VERSION_CHAR_LENGTH 5

//Global board stuff
#define SERIAL_PORT_SPEED 115200 //9600
#define BOARD_TX  1 //GPIO pin TX - UART0
#define BOARD_RX  3 //GPIO pin RX - UART0
#define PIN_RL1 16 //GPIO pin Relay1 control
#define PIN_RL2 17 //GPIO pin Relay2 control
#define PIN_GAS_SENSOR_D0  19    //GPIO pin for GAS alerts - Digital Input - HIGH = OK, LOW=GAS_DETECTED
#define I2C_SDA 21 //I2C - SDA pin in the ESP board (SDA pin in the sensor)
#define I2C_SCL 22 //I2C - SCL pin in the ESP board (SCL pin in the sensor)
#define PIN_LED 23     //GPIO pin LED control
#define PIN_THERMOSTATE 26    //GPIO interrupt pin thermostate ON
#define PIN_GAS_SENSOR_A0  35    //GPIO pin for GAS samples - ADC1. Looks like pins in ADC2 can't be used for Analog if the WiFi module is used: https://www.luisllamas.es/en/esp32-adc/
#define EEPROM_SIZE 0x1000

#define DEVICE_NAME_PREFIX "boiler-relay-controlv2"
#ifndef DEBUG_MODE_ON
  #if TYPE_OF_BUILD==BUILD_TYPE_PRODUCTION
    #define DEBUG_MODE_ON false
  #else
    #define DEBUG_MODE_ON true
  #endif
#endif

//WiFi stuff
#define WIFI_ENABLED  true
#define MAX_CONNECTION_ATTEMPTS 10
#define NTP_SERVER_NAME_MAX_LENGTH 64
#ifndef NTP_SERVER
  #define NTP_SERVER  "time.google.com"
#endif
#define FQDN_TO_CHECK "google.com"
#define SERVER_TO_CHECK "connectivity.office.com"

#ifndef NTP_TZ_ENV_VARIABLE //Should be defined in user_setup.h
  #define NTP_TZ_ENV_VARIABLE "CET-1CEST,M3.5.0,M10.5.0/3"  //POSIX.1 format for Europe/Madrid TZ env variable
#endif
#define TZ_ENV_VARIABLE_MAX_LENGTH 57 //Including end null
#ifndef NTP_TZ_NAME //Should be defined in user_setup.h
  #define NTP_TZ_NAME "Europe/Madrid"  //POSIX.1 format for Europe/Madrid TZ env variable
#endif
#define TZ_ENV_NAME_MAX_LENGTH 30 //Including end null
#ifndef NTP_TZ_ENV_VARIABLE //Use GNUB Time Zone format if not POSI.1 one is provided with
  // --> Not longer used in the code as POSIX.1 format is preferred. Kept it just for documentation
  #define GMT_OFFSET_SEC 3600                 
  #define DAYLIGHT_OFFSET_SEC 7200 //3600 for CEST
#endif
#define UPLOAD_SAMPLES_ENABLED true
#define UPLOAD_SAMPLES_FROM_SITE "home"

#if BUILD_ENV_NAME==BUILD_TYPE_DEVELOPMENT
  #define SERVER_UPLOAD_SAMPLES  "10.88.50.5"
#endif
#if BUILD_ENV_NAME==BUILD_TYPE_PRODUCTION
  #ifndef SERVER_UPLOAD_SAMPLES
    #define SERVER_UPLOAD_SAMPLES "195.201.42.50"
  #else
    #define SERVER_UPLOAD_SAMPLES  "10.88.50.5"
  #endif
#endif
#define SERVER_UPLOAD_PORT  80
//GET /lar-to/?device=boiler-temp-relay&local_ip_address=192.168.100.192&relay_status=0&counts=-1359 HTTP/1.0
#define GET_REQUEST_TO_UPLOAD_SAMPLES  "GET /lar-to/?"
#define WIFI_100_RSSI -60  //RSSI > -60 dBm Excellent - Consider 100% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_075_RSSI -70  //RSSI > -70 dBm Very Good - Consider 75% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_050_RSSI -80  //RSSI > -80 dBm Good - Consider 50% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_025_RSSI -90  //RSSI > -90 dBm Low - Consider 25% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_000_RSSI -100 //RSSI < -100 dBm No Signal - Lower values mean no SSID visibiliy, 0% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_MAX_SSID_LENGTH  33 //32 CHAR + NULL
#define WIFI_MAX_PSSW_LENGTH  64 //63 CHAR + NULL
#define WIFI_MAX_SITE_LENGTH  11 //10 CHAR + NULL

//WEB SERVER Stuff
#define WEBSERVER_ENABLED true
#define WEBSERVER_MIN_HEAP_SIZE  15000 //95000  //Based on guess. Below this, the web server will be reset
#define WEBSERVER_SEND_DELAY 800  //milliseconds - Delay to allow stop BLE Advertisings before sending HTTP Answer
#define WEBSERVER_PORT 80
#define WEBSERVER_CHECK_PERIOD 240000  //milliseconds - Check webServer responsiveness every 4 min
#define WEBSERVER_CSSSTYLES_PAGE "/styles.css"
#define WEBSERVER_CSSNAVBAR_PAGE "/tswnavbar.css"
#define WEBSERVER_LOGO_ICON "/The_IoT_Factory.png"
#define WEBSERVER_INDEX_PAGE "/index.html"
#define WEBSERVER_GRAPHS_PAGE "/graphs.html"
#define WEBSERVER_STATS_PAGE "/stats.html"
#define WEBSERVER_INFO_PAGE "/info.html"
#define WEBSERVER_BASICCONFIG_PAGE "/basic.html"
#define WEBSERVER_CLOUDCONFIG_PAGE "/cloud.html"
#define WEBSERVER_BLUETOOTHCONFIG_PAGE "/bluetooth.html"
#define WEBSERVER_MAINTENANCE_PAGE "/maintenance.html"
#define WEBSERVER_CONTAINER_PAGE "/container.html"
#define WEBSERVER_TEST_PAGE "/test.html"
#define WEBSERVER_CONSOLE_PAGE "/console.html"
//#define WEBSERVER_APINDEX_PAGE "/apindex.html"
//#define WEBSERVER_APCONTAINER_PAGE "/apcontainer.html"
#define WEBSERVER_UPLOADFILE_PAGE "/maintenance_upload_firmware"
#define WEBSERVER_FAVICON_ICON "/favicon.ico"
//#define WEBSERVER_JQUERY_JS "/jquery.min.js"
#define WEBSERVER_FILE_EXTENSION ".bin"
#define WEBSERVER_DEFAULTCONF_PAGE "/maintenance_default_values"
#define WEBSERVER_DEVICERESET_PAGE "/maintenance_device_reset"
#define WEBSERVER_COUNTERRESET_PAGE "/reset_counters"
#define WEBSERVER_SAMPLES_EVENT "/sampleEvents"
#define WEBSERVER_SAMPLES_PAGE "/samples"
#define WEBSERVER_GAUGESCRIPT_PAGE "/gauge.min.js"
#define WEBSERVER_INDEX_SCRIPT_PAGE "/index_script.js"
#define WEBSERVER_STATS_SCRIPT_PAGE "/stats_script.js"
#define WEBSERVER_INFO_SCRIPT_PAGE "/info_script.js"
#define WEBSERVER_BASIC_SCRIPT_PAGE "/basic_script.js"
#define WEBSERVER_CLOUD_SCRIPT_PAGE "/cloud_script.js"
#define WEBSERVER_MAINTENANCE_SCRIPT_PAGE "/maintenance_script.js"
#define WEBSOCKET_CONSOLE_URI "/wsconsole"
#define COOKIE_SIZE sizeof(DEVICE_NAME_PREFIX)+44 //54 //32 Chars + null
#define CLOUD_SERVICES_URL  "http://the-iotfactory.com/lar-boil/"

//Error stuff
#define NO_ERROR                      0x00000000  //00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
#define ERROR_SENSOR_TEMP_HUM_SETUP   0x00000002  //00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000010 
#define ERROR_WIFI_SETUP              0x00000010  //00000000 00000000 00000000 00000000 00000000 00000000 00000000 00010000
#define ERROR_SSID_CONNECTION         0x00000040  //00000000 00000000 00000000 00000000 00000000 00000000 00000000 01000000
#define ERROR_NTP_SERVER              0x00000080  //00000000 00000000 00000000 00000000 00000000 00000000 00000000 10000000
#define ERROR_CLOUD_SERVER            0x00000100  //00000000 00000000 00000000 00000000 00000000 00000000 00000001 00000000
#define ERROR_BREAK_WEB_SETUP         0x00008000  //00000000 00000000 00000000 00000000 00000000 00000000 10000000 00000000 
#define ERROR_SPIFFS_SETUP            0x00010000  //00000000 00000000 00000000 00000000 00000000 00000001 00000000 00000000 
#define ERROR_WEB_SERVER              0x00020000  //00000000 00000000 00000000 00000000 00000000 00000010 00000000 00000000 
#define ERROR_MQTT_SERVER             0x00040000  //00000000 00000000 00000000 00000000 00000000 00000100 00000000 00000000 
#define ERROR_EEPROM_VARIABLES_INIT   0x00080000  //00000000 00000000 00000000 00000000 00000000 00001000 00000000 00000000 
#define ERROR_WEB_SOCKET              0x00100000  //00000000 00000000 00000000 00000000 00000000 00010000 00000000 00000000 
#define ERROR_UPLOAD_FILE_NOERROR           0x00  //00000000
#define ERROR_UPLOAD_FILE_EXTENSION         0x01  //00000001
#define ERROR_UPLOAD_FILE_UPDATE_BEGIN      0x02  //00000010
#define ERROR_UPLOAD_FILE_UPDATE_WRITE      0x03  //00000011
#define ERROR_UPLOAD_FILE_UPDATE_END        0x04  //00000100
#define ERROR_UPLOAD_FILE_POST_PRM_NAME     0x05  //00000101
#define ERROR_UPLOAD_FILE_POST_PRM_NUMBER   0x06  //00000110
#define ERROR_UPLOAD_FILE_MD5_CHECKSUM      0x07  //00000111
#define ERROR_UPLOAD_FILE_SIZE              0x08  //00001000
#define ERROR_UPLOAD_FILE_NOFILE            0x09  //00001001
#define ERROR_UPLOAD_FILE_NOAUTH            0x0A  //00001010
#define ERROR_UPLOAD_FILE_WRONG_FILE_NAME   0x0B  //00001011
#define ERROR_UPLOAD_FILE_NOFLASHSPACE_LEFT 0x0C  //00001100
#define ERROR_UPLOAD_FILE_NOSPIFFSPACE_LEFT 0x0D  //00001101
#define ERROR_UPLOAD_FILE_NOBIN_FILE        0x0E  //00001110
#define ERROR_UPLOAD_FILE_NOSPIFFS_FILE     0x0F  //00001111
#define ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE 0x10  //00010000
#define ERROR_UPLOAD_FILE_NOCOOKIE          0x11  //00010001
#define ERROR_UPLOAD_FILE_BADCOOKIE_FORMAT  0x12  //00010010
#define ERROR_UPLOAD_FILE_UPLOAD_ONGOING    0x13  //00010011
#define ERROR_UPLOAD_FILE_BADCOOKIE         0x14  //00010100
#define ERROR_WEBSERVER_NO_RESPOND          0X20  //00100000
#define ERROR_BLE_NO_LOAD                   0X21  //00100001
#define ERROR_BLE_UNLOAD                    0x22  //00100010
#define ERROR_MIN_HEAP                      0x23  //00100011
#define ERROR_NO_CONNECTIVITY               0x24  //00101000

//Timers and Global stuff
#define INITIAL_BOOTIME X  //milliseconds - Time to bootup as per board measurements
#define BOOT_LOGS_TIME 120000  //milliseconds - 2 min - Time to save the logs after boot time
#define uS_TO_S_FACTOR  1000000
#define ONE_SECOND_PERIOD 1000  //Millisenconds - 1 second
#define TIME_COUNTERS_EEPROM_UPDATE_PERIOD 3600000  //Millisenconds - 1 hour
#define HTTP_ANSWER_TIMEOUT 7000  //Millisenconds
#define NTP_KO_CHECK_PERIOD  60000 //Milliseconds. 1 minute
#define NTP_CHECK_TIMEOUT     5000  //Millisecons. Should have NTP anser within 2 sc.
#define WIFI_RECONNECT_PERIOD  120000 //milliseconds - 2 min - less than UPLOAD_SAMPLES_PERIOD for quicker icon refresh
#define HEAP_SAMPLE_PERIOD 20000 //Millisecons - Period to get heap size readings
#define TOFF 3000 //Time all relays are off since starting the sequence in millisenconds
#define TRL1_ON 1000  //Time Relay-1 ON in millisenconds
#define GAP_OFF 2000  //Time between ON Relays in millisenconds
#define TRL2_ON  500  //Time Relay-2 ON in millisenconds
#define THERMOSTATE_INTERRUPT_DELAY  300 //Milliseconds - Time to stabilize the interrupt
#define MQTT_CHECK_PERIOD  180000 //Millisecons - Interval between checks on the MQTT server
#define SAMPLE_PERIOD  20000 //Millisecons - Interval between gas/temp/hum samples if there is boiler activity (neither boiler nor heater)
#define SAMPLE_LONG_PERIOD  300000 //Millisecons - Interval between gas/temp/hum samples if no boiler activity (neither boiler nor heater)
#define CONNECTIVITY_CHECK_PERIOD 240000 //Millisecons - Interval between connectiviy checks
#define ADC_SAMPLES 100
#define HA_ADVST_WINDOW 15000 //Milliseconds - Time gap to send MQTT HA messages since bootup to make sure all the topics are processed - v0.9.7

//Binary header stuff
//https://github.com/espressif/esp-idf/blob/8fbb63c2a701c22ccf4ce249f43aded73e134a34/components/bootloader_support/include/esp_image_format.h#L58
#define BINARY_HD_MAGIC_NUMBER       0xE9 //Magic number as per https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/firmware-image-format.html
#define BINARY_HD_MAGIC_NUMBER_ADDR  0x00 //Address
#define BINARY_HD_MAGIC_WORD         0xABCD5432 //Magic word as per //https://github.com/espressif/esp-idf/blob/8fbb63c2a701c22ccf4ce249f43aded73e134a34/components/bootloader_support/include/esp_image_format.h#L58
#define BINARY_HD_MAGIC_WORD_ADDR    0x20 //Address - 32 = 0x20

//SPIFFS stuff
#define SPIFFS_CFG_LOG_BLOCK_SZ  4096 //Bytes - Size of Logical SPIFFS Blocks
#define SPIFFS_CFG_LOG_PAGE_SZ  256 //Bytes - Size of Logical SPIFFS Pages
#define SPIFFS_MAGIC_ADDR SPIFFS_CFG_LOG_PAGE_SZ-4 //Bytes - Address for the magic number to confirm it's SPIFFS partition

//WEB and MQTT credentials
#ifndef WEB_USER_CREDENTIAL
  #define WEB_USER_CREDENTIAL "admin"
#endif
#ifndef WEB_PW_CREDENTIAL
  #define WEB_PW_CREDENTIAL "adminCO2"
#endif
#define WEB_USER_CREDENTIAL_LENGTH 11 //10+null=11B
#define WEB_PW_CREDENTIAL_LENGTH 11 //10+null=11B
#ifndef MQTT_USER_CREDENTIAL
  #define MQTT_USER_CREDENTIAL "admin"
#endif
#ifndef MQTT_PW_CREDENTIAL
  #define MQTT_PW_CREDENTIAL "adminCO2"
#endif
#define MQTT_USER_CREDENTIAL_LENGTH 11 //10+null=11B
#define MQTT_PW_CREDENTIAL_LENGTH 11 //10+null=11B

//MQTT stuff
#define MQTTSERVER_ENABLED true
#define SECURE_MQTT_ENABLED false
#ifndef MQTT_SERVER
  #define MQTT_SERVER "10.88.50.5"
#endif
#define MQTTSERVER_PORT 1883
#define MQTT_SERVER_NAME_MAX_LENGTH NTP_SERVER_NAME_MAX_LENGTH
#define MQTT_TOPIC_NAME_MAX_LENGTH 201 //200+null=201 B
#ifndef MQTT_TOPIC_PREFIX
  #define MQTT_TOPIC_PREFIX "the-iot-factory/"
#endif
#define MQTT_TOPIC_SUBSCRIPTION "the-iot-factory/info"
#define MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION "cmnd/RELAY"
#define MQTT_HA_SENSOR_TOPIC_PREFIX "homeassistant/sensor"
#define MQTT_HA_BINARY_SENSOR_TOPIC_PREFIX "homeassistant/binary_sensor"
#define MQTT_HA_BUTTON_TOPIC_PREFIX "homeassistant/button"
#define MQTT_HA_SWITCH_TOPIC_PREFIX "homeassistant/switch"
#define MQTT_HA_B_AND_LWT_TOPIC_PREFIX "homeassistant/status"
#define MQTT_POWER_MEASURE_ENABLED false
#ifndef MQTT_POWER_TOPIC
  #define MQTT_POWER_TOPIC "tele/tasmota_2EDC69/SENSOR"
#endif

//POWER stuff
#define BOILER_STATUS_ON_POWER_THRESHOLD 50  //Watts - Power threshold to decide whether the boiler is active (not necessarily burning gas)
#define BOILER_FLAME_ON_POWER_THRESHOLD 235  //Watts - Power threshold to decide whether the boiler is burning gas or not due to either hot water or heater - Based on readings

//BLE stuff
#define BLE_ENABLED  false
#define BLE_MIN_HEAP_SIZE  95000  //Based on tests
#define ABSULUTE_MIN_MAX_HEAP_BLOCK_THRESHOLD  5000  //Bellow this threshold, heap overflow risk. SoftReset is needed. 
#define ABSULUTE_MIN_HEAP_THRESHOLD  7000  //Bellow this threshold, heap overflow risk. SoftReset is needed. 
#define MIN_HEAP_SEEN_THRESHOLD        700  //Bellow this threshold, heap overflow risk. SoftReset is needed. 
#define BLE_BEACON_UUID           "F7826DA6-4FA2-4E98-8024-BC5B71E0893E"  //Kontakt proximity
#define BLE_BEACON_UUID_REV       "3E89E071-5BBC-2480-984E-A24FA66D82F7" //Kontakt proximity reverse
#define BLE_BEACON_UUID_LENGH     37  //36+null=33 B - Including '-'
#define BLE_BEACON_MANUFACTURER   0x4c00
#define BLE_BEACON_RSSI           0xC5
#define BLE_BEACON_FLAGS          0x06 //0x06 - Simulating Kontak TAG - 0x1A Real
#define BLE_SERVICE_UUID      "9504ECB3-997C-4367-95FA-DA48461648CA"
#define BLE_CHARACT_CO2_UUID      "055232CF-43A3-4373-92B8-090B630EA136"
#define BLE_CHARACT_TEM_UUID      "D914B4BB-D7E7-4C81-9017-C9486FDEC3FA"
#define BLE_CHARACT_HUM_UUID      "0EE3E2CC-4F7E-4E87-BEB7-C765BF8A75A6"

//MQ5 Sensor stuff
//Calibration coeficients as per graph in https://www.rxelectronics.sg/datasheet/a7/101020056.pdf
#define R0_CALIBRATED  3.42 //R0 value of MQ5 Gas sensor - Using readAnalogMilliVolts()
#define H2_A  -0.000114286    // A coeficient for H2 curve
#define H2_B  1.822857143     // B coeficient for H2 curve
#define LPG_A  -0.0000581633  // A coeficient for LPG curve - TOWN GAS, NATURAL GAS
#define LPG_B  0.711632653    // B coeficient for LPG curve - TOWN GAS, NATURAL GAS
#define CH4_A  -0.0000765306  // A coeficient for CH4 curve - METANE
#define CH4_B  0.965306122    // B coeficient for CH4 curve - METANE
#define CO_A  -0.000183673    // A coeficient for CO curve - MONOXIDE CARBONE
#define CO_B  4.036734694     // B coeficient for CO curve - MONOXIDE CARBONE
#define ALCOHOL_A  -0.000244898 // A coeficient for ALCOHOL curve
#define ALCOHOL_B  3.848979592  // B coeficient for ALCOHOL curve

//SI7021 Sensor stuff (Temp & Hum)
#define TEMP_HUM_SENSOR  "SI7021"  //SI7021, SHT21 and HTU21 are all equivalent
#define TEMP_HUM_SENSOR_TYPE  "SI7021"
