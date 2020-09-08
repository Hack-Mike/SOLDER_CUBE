//
// Created by Michele Mocellin on 28/07/2020.
//

#include "ethernet.h"
#include "esp_setup.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define ETH_PHY_ADDR 1
#define ETH_PHY_RST_GPIO 5
#define ETH_MDC_GPIO 23
#define ETH_MDIO_GPIO 18

EventGroupHandle_t eth_event_group;

extern EventGroupHandle_t connectionEventBits;


esp_eth_handle_t eth_handle;

static const char *TAG = "ETHERNET";

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


/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            ESP_LOGI(TAG, "Ethernet Link Up");
            xEventGroupClearBits(connectionEventBits, ETH_LINKED);
            ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                     mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            break;

        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Ethernet Link Down");
            xEventGroupSetBits(connectionEventBits, ETH_LINKED);
            break;

        case ETHERNET_EVENT_START:
            ESP_LOGI(TAG, "Ethernet Started");
            break;

        case ETHERNET_EVENT_STOP:
            ESP_LOGI(TAG, "Ethernet Stopped");
            break;

        default:
            break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const tcpip_adapter_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    xEventGroupSetBits(connectionEventBits, ETH_CONNECTED_BIT);

}

void ethernetSetup() {

    //tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(tcpip_adapter_set_default_eth_handlers());

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = ETH_PHY_ADDR;
    phy_config.reset_gpio_num = ETH_PHY_RST_GPIO;

    //INTERNAL ETHERNET
    mac_config.smi_mdc_gpio_num = ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);



    //ETHERNET PHY IP101
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    eth_handle = NULL;

    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    //ESP_ERROR_CHECK(esp_eth_start(eth_handle));

}

void ethernetStop(void) {

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler));

    int err;
    puts("Stopping Ethernet...");
    err = esp_eth_stop(eth_handle);

    if(err == ESP_OK) {
        puts("Ethernet stopped");
    } else {
        printf("Ethernet did not stop, error: %d\n", err);
    }
}

void ethernetStart(void) {

    puts("Starting Ethernet...");

    EventBits_t bits;
    int err;



    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));
    err = esp_eth_start(eth_handle);

    if(err == ESP_OK) {
        puts("Ethernet started");
    } else {
        printf("Ethernet did not start, error: %d\n", err);
    }

    bits = xEventGroupWaitBits(connectionEventBits,
                                           ETH_CONNECTED_BIT | ETH_FAIL_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           portMAX_DELAY);
    displayBits(bits);

    if (bits & ETH_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to ethernet layer");
    } else if (bits & ETH_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to ethernet layer");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}
