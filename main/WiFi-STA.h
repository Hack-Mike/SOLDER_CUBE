//
// Created by Michele Mocellin on 09/05/2020.
//

#ifndef MY_IOT_WIFI_STA_H
#define MY_IOT_WIFI_STA_H

#endif //MY_IOT_WIFI_STA_H

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define ESP_WIFI_SSID      "NETGEAR71"
#define ESP_WIFI_PASS      "joyouschair932"
#define ESP_MAXIMUM_RETRY  10

static const char *stationTAG = "WI-FI STATION: ";

void wifi_init_sta(void);

void wifiStop(void);

void wifiStart(void);