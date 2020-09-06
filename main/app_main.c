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








EventGroupHandle_t connectionEvent = NULL;
EventBits_t connectionBits = 0;
extern EventGroupHandle_t buttonEvent;
extern EventBits_t buttonBits;


#define ETH_BIT (1UL << 0UL)

void connectionTask(void) {


    while(1) {

        buttonBits = xEventGroupWaitBits(buttonEvent, BUTTON_BIT_0 | BUTTON_BIT_1, pdTRUE, pdFALSE, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));

        if(buttonBits & BUTTON_BIT_0) {
            ethernetStart();
            wifiStop();
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

    jsonFunction();


    //xTaskCreate(ethernetSetup, "ETHERNET", 4000, NULL, 5, NULL);
    xTaskCreate(buttonTask, "BUTTON", 4000, NULL, 5, NULL);
    xTaskCreate(connectionTask, "CONN", 4000, NULL, 5, NULL);

}
