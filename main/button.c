//
// Created by Michele Mocellin on 30/08/2020.
//

#include "button.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

EventGroupHandle_t buttonEvent = NULL;
EventBits_t buttonBits = 0;

_Noreturn void buttonTask(void) {

    buttonEvent = xEventGroupCreate();

    unsigned int buttonPressed = 0;
    unsigned int buttonRelease;
    unsigned int tickButtonHeld;
    unsigned int millisHeld = 0;
    char previousButtonStatus = 0;
    char buttonState;
    char buttonFun = 0;

    while(1){

        buttonState = gpio_get_level(GPIO_NUM_14);

        if(buttonState == 1 && previousButtonStatus == 0) {

            previousButtonStatus = 1;
            buttonPressed = xTaskGetTickCount();

        } else if (buttonState == 0 && previousButtonStatus == 1) {

            previousButtonStatus = 0;
            buttonRelease = xTaskGetTickCount();
            tickButtonHeld = (buttonRelease - buttonPressed);
            millisHeld = (tickButtonHeld * 1000) / configTICK_RATE_HZ;

        }

        if(millisHeld < 50) {
            //debounce value

        } else if(millisHeld >= 50 && millisHeld < 1000) {
            buttonFun = 1;
            xEventGroupSetBits(buttonEvent, BUTTON_BIT_0);
            printf("Millis button held -> %d\n", millisHeld);
            printf("Button function -> %d\n", buttonFun);
        } else if(millisHeld >= 1000 && millisHeld < 2000) {
            buttonFun = 2;
            xEventGroupSetBits(buttonEvent, BUTTON_BIT_1);
            printf("Millis button held -> %d\n", millisHeld);
            printf("Button function -> %d\n", buttonFun);
        } else if(millisHeld >= 2000 && millisHeld < 5000) {
            buttonFun = 3;
            printf("Millis button held -> %d\n", millisHeld);
            printf("Button function -> %d\n", buttonFun);
        } else if(millisHeld >= 5000) {
            buttonFun = 4;
            printf("Millis button held -> %d\n", millisHeld);
            printf("Button function -> %d\n", buttonFun);
        }

        buttonFun = 0;
        millisHeld = 0;

        vTaskDelay(1);
    }

}