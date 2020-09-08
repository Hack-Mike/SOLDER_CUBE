//
// Created by Michele Mocellin on 11/08/2020.
//

#include "esp_setup.h"
#include "solder_cube_error.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "tcpip_adapter.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_tls.h"

#include "driver/gpio.h"

EventGroupHandle_t connectionEventBits;
//EventBits_t connectionBits = 0;

char serialNumber[15];

static int getSerialNumber(void) {

    printf("Serial Number: %s\n", serialNumber);

    uint8_t macAdd[6] = {0};

    int err = esp_efuse_mac_get_default(macAdd);

    if(err != ESP_OK) {
        return FAIL_GET_MAC;
    }


    printf("BASE MAC: ");
    for(int i = 0; i < 6; i++) {
        printf("%02x ", macAdd[i]);
    }
    puts("");

    //create Serial Number String
    for(size_t i = 0; i < 6; i++) {

        char str[3];
        //itoa(macAdd[i], str, 16);
        if(macAdd[i] <= 0x0f) {
            char strTemp[3];
            str[0] = '0';
            str[1] = '\0';
            sprintf(strTemp, "%x", macAdd[i]);
            strcat(str, strTemp);
        } else {
            sprintf(str, "%x", macAdd[i]);
        }

        strcat(serialNumber, str);

        if(i == 1 || i == 3) {
            strcat(serialNumber, "-");
        }
    }

    printf("Serial Number: %s\n", serialNumber);

    return ESP_OK;
}

void espSetup(void) {

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");



    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    int err = getSerialNumber();

    if(err != ESP_OK) {
        printf("Failed to get serial number. Err: %d\n", err);
    }

    connectionEventBits = xEventGroupCreate();
    if(connectionEventBits != NULL) {
        puts("Connection Event created!");
    }

    tcpip_adapter_init();

}