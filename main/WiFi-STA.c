//
// Created by Michele Mocellin on 09/05/2020.
//

#include "WiFi-STA.h"
#include "esp_setup.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"



#include "lwip/err.h"
#include "lwip/sys.h"



/* FreeRTOS event group to signal when we are connected*/
//static EventGroupHandle_t s_wifi_event_group;

extern EventGroupHandle_t connectionEventBits;
EventBits_t bits = 0;

static const char* TAG = "WIFI STA";
int apReconnectionRetries = 0;

static void displayBits(unsigned int value) {
    unsigned int displayMask = 1 << 31;

    printf("DEC to BIN %u = ", value);

    for(unsigned int c = 1; c <= 32; ++c) {
        putchar(value & displayMask ? '1' : '0');
        //printf("%s", value & displayMask ? "1" : "0");
        value <<= 1;

        if(c % 8 == 0) {
            printf(" ");
        }
    }
    puts("");
}

static void reconnectWifi(void){

    if(apReconnectionRetries < ESP_MAXIMUM_RETRY) {

        puts("Retry reconnection to AP...");
        int err = esp_wifi_connect();
        apReconnectionRetries++;

        if(err != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    } else {
        //TODO sistemare opzioni di riconessione
    }

}

static void wifiEvent(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {

    if(eventBase == WIFI_EVENT) {
        switch(eventId) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();

            break;

            case WIFI_EVENT_STA_DISCONNECTED:
                reconnectWifi();
                xEventGroupSetBits(connectionEventBits, WIFI_FAIL_BIT);
            break;
        }
    }
}

static void ipEvent(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {

    ip_event_got_ip_t *event = (ip_event_got_ip_t *) eventData;
    const tcpip_adapter_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "WiFi Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "WiFiIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "WiFiMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "WiFiGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    xEventGroupSetBits(connectionEventBits, WIFI_CONNECTED_BIT);
}

void wifi_init_sta() {

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = ESP_WIFI_SSID,
                    .password = ESP_WIFI_PASS
            },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_LOGI(stationTAG, "WiFi setup finished.");
}

void wifiStart(void) {

    puts("Starting Wi-Fi");

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEvent, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ipEvent, NULL));

    int err = esp_wifi_start();

    if(err == ESP_OK) {

        puts("Wi-Fi started");

    } else {

        printf("Wi-Fi did not start. Error: %d\n", err);
    }

    bits = xEventGroupWaitBits(connectionEventBits,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           portMAX_DELAY);

    displayBits(bits);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(stationTAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(stationTAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(stationTAG, "UNEXPECTED EVENT");
    }
}

void wifiStop(void) {

    puts("Stopping Wi-Fi...");

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ipEvent));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEvent));

    esp_wifi_disconnect();
    int err = esp_wifi_stop();

    if(err == ESP_OK) {

        puts("Wi-Fi stopped");
        xEventGroupSetBits(connectionEventBits, WIFI_DISCONNECTED_BIT);
    } else {
        printf("Wi-Fi did not stop. Error: %d\n", err);
        xEventGroupSetBits(connectionEventBits, WIFI_FAIL_BIT);
    }



    bits = xEventGroupWaitBits(connectionEventBits,
                                           WIFI_DISCONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           portMAX_DELAY);

    displayBits(bits);

    if (bits & WIFI_DISCONNECTED_BIT) {
        ESP_LOGI(stationTAG, "Disconnected from ap SSID:%s",
                 ESP_WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(stationTAG, "Failed to disconnect from SSID:%s",
                 ESP_WIFI_SSID);
    } else {
        ESP_LOGE(stationTAG, "UNEXPECTED EVENT");
    }
}