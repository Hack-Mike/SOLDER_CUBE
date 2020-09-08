/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_trace.h"
#include "esp_heap_caps.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"


#include "esp_setup.h"
#include "WiFi-STA.h"
#include "ethernet.h"
#include "button.h"
#include "JsonUtility.hpp"
#include "mqtt.h"







EventGroupHandle_t connectionEvent = NULL;

extern EventGroupHandle_t connectionEventBits;

extern EventGroupHandle_t buttonEvent;
extern EventBits_t buttonBits;

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

void connectionTask(void) {

    while(1) {

        buttonBits = xEventGroupWaitBits(buttonEvent, BUTTON_BIT_0 | BUTTON_BIT_1, pdTRUE, pdFALSE, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));

        if(buttonBits & BUTTON_BIT_0) {
            wifiStop();
            ethernetStart();
        } else if(buttonBits & BUTTON_BIT_1) {
            ethernetStop();
            wifiStart();
        }

        vTaskDelay(1);
    }
}




void app_main() {

    espSetup();

    ethernetSetup();
    wifi_init_sta();

    vTaskDelay(pdMS_TO_TICKS(5000));
    wifiStart();

    vTaskDelay(pdMS_TO_TICKS(5000));

    //mqttStart();

    //vTaskDelay(pdMS_TO_TICKS(5000));


    //char *msg = jsonFunction();

    //publish("/private/topic00", msg);


    //xTaskCreate(ethernetSetup, "ETHERNET", 4000, NULL, 5, NULL);

    puts("Ready to decide connection method. 1-ETHERNET 2-WIFI");
    xTaskCreate(buttonTask, "BUTTON", 4000, NULL, 5, NULL);
    xTaskCreate(connectionTask, "CONN", 4000, NULL, 5, NULL);

}
