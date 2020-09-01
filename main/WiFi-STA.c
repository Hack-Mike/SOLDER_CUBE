//
// Created by Michele Mocellin on 09/05/2020.
//

#include "WiFi-STA.h"

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
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1



static int s_retry_num = 0;

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

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(stationTAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(stationTAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(stationTAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);


    }
}

void wifi_init_sta() {
    s_wifi_event_group = xEventGroupCreate();

    //tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = ESP_WIFI_SSID,
                    .password = ESP_WIFI_PASS
            },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

    uint8_t macAdd[8] = {0};
    int err = esp_wifi_get_mac(WIFI_MODE_STA, *macAdd);

    printf("Errore mac address: %d\n", err);

    printf("MAC: ");
    for(int i = 0; i < 8; i++) {
        printf("%hhu ", macAdd[i]);
    }

    puts("");

    //ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(stationTAG, "wifi_init_sta finished.");


}

void wifiStop(void) {

    puts("Stopping Wi-Fi...");
    esp_wifi_disconnect();
    int err = esp_wifi_stop();

    if(err == ESP_OK) {

        puts("Wi-Fi stopped");
    } else {

        printf("Wi-Fi did not stop. Error: %d\n", err);
    }
}

void wifiStart(void) {

    puts("Starting Wi-Fi");

    int err = esp_wifi_start();

    if(err == ESP_OK) {

        puts("Wi-Fi started");
    } else {

        printf("Wi-Fi did not start. Error: %d\n", err);
    }


    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
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

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);

    uint8_t macAdd[8] = {0};
    int err2 = esp_wifi_get_mac(WIFI_MODE_STA, *macAdd);

    printf("Errore mac address: %d\n", err2);

    printf("MAC: ");
    for(int i = 0; i < 8; i++) {
        printf("%d ", macAdd[i]);
    }

    puts("");
}