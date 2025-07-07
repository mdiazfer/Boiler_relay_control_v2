/* 
  Definition of user configuration parameters
*/

//WiFi stuff
#ifndef WIFI_SSID_CREDENTIALS
  #define WIFI_SSID_CREDENTIALS "madnetot"
  #define WIFI_PW_CREDENTIALS "FC4B4C9F6E601E56451B8"
  #define WIFI_SITE "home"
  #define WIFI_SITE_ALLOW true
  //#define WIFI_SSID_CREDENTIALS "iPhonix"
  //#define WIFI_PW_CREDENTIALS "ALEwifiALE"
  //#define WIFI_SITE "home"
  //#define WIFI_SITE_ALLOW true
  #define WIFI_SSID_CREDENTIALS_BK1 "EBC_DEMO"
  #define WIFI_PW_CREDENTIALS_BK1 "al31tal1a@"
  #define WIFI_SITE_BK1 "office"
  #define WIFI_SITE_ALLOW_BK1 false
  //#define WIFI_SSID_CREDENTIALS_BK1 "ALE"
  //#define WIFI_PW_CREDENTIALS_BK1 "ALE@Madrid-1"
  //#define WIFI_SITE_BK1 "office"
  //#define WIFI_SITE_ALLOW_BK1 false
  #define WIFI_SSID_CREDENTIALS_BK2 "wlansix"
  #define WIFI_PW_CREDENTIALS_BK2 "madwep6wepmad"
  #define WIFI_SITE_BK2 "pare. home"
  #define WIFI_SITE_ALLOW_BK2 false
#endif

//HTTP Cloud server
#define SERVER_UPLOAD_SAMPLES  "10.88.50.5"

//NTP servers
#define NTP_SERVER  "10.88.50.5"
#define NTP_SERVER2  "time.google.com"
#define NTP_SERVER3  "pool.ntp.org"
#define NTP_SERVER4  "time.apple.com"
//#define NTP_SERVER4  "time.windows.com"

//HTTP server admin credentias
#define WEB_USER_CREDENTIAL "admin"
#define WEB_PW_CREDENTIAL "adminCO2"

//MQTT server
#define MQTT_USER_CREDENTIAL "admin"
#define MQTT_PW_CREDENTIAL "adminCO2"
#define MQTT_SERVER "10.88.50.5"

//Syslog server
#define SYSLOG_SERVER "10.88.50.5"
#define SYSLOG_SERVER_UDP_PORT 514

//Time zone stuff
#define NTP_TZ_ENV_VARIABLE "CET-1CEST,M3.5.0,M10.5.0/3"  //POSIX.1 format for Europe/Madrid TZ env variable
#define NTP_TZ_NAME "Europe/Madrid"  //POSIX.1 format for Europe/Madrid TZ env variable