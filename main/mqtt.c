//
// Created by Michele Mocellin on 06/09/2020.
//

#include "mqtt.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
//#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_tls.h"

#include "esp_log.h"
#include "mqtt_client.h"

esp_mqtt_client_handle_t client;

extern const uint8_t mqtt_eclipse_org_pem_start[] asm("_binary_mqtt_eclipse_org_pem_start");
extern const uint8_t mqtt_eclipse_org_pem_end[] asm("_binary_mqtt_eclipse_org_pem_end");

//extern const uint8_t cert_chain_pem_start[] asm("_binary_chain_pem_start");
//extern const uint8_t cert_chain_pem_end[] asm("_binary_chain_pem_end");

//extern const uint8_t esseci_pem_start[] asm("_binary_esseci_pem_start");
//extern const uint8_t esseci_pem_end[] asm("_binary_esseci_pem_end");

//extern const uint8_t esseci_der_der_start[] asm("_binary_esseci_der_der_start");
//extern const uint8_t esseci_der_der_end[] asm("_binary_esseci_der_der_end");

char topicMsg[128];
char topicName[50];

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {

    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    printf("EVENT -> %d\n", event->event_id);

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(mqttTAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(mqttTAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/private/topic00", 0);
            ESP_LOGI(mqttTAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(mqttTAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(mqttTAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(mqttTAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_DATA");
            //printf("Topic length -> %d\n", event->topic_len);
            size_t topicNameLength = event->topic_len;
            char *topic = event->topic;


            for(size_t i = 0; i < topicNameLength; i++) {
                topicName[i] = topic[i];
            }
            topicName[topicNameLength] = '\0';
            printf("Message arrive from topic -> %s\n", topicName);

            printf("Data length -> %d\n", event->data_len);
            size_t messageLength = event->data_len;
            char *data = event->data;

            for(size_t i = 0; i < messageLength; i++) {
                topicMsg[i] = data[i];
            }

            topicMsg[messageLength] = '\0';

            printf("Topic -> %s\n", topicMsg);
            //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(mqttTAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(mqttTAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(mqttTAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_app(void)
{


    esp_mqtt_client_config_t mqtt_cfg = {

            //.uri= "mqtts://test.mosquitto.org:8884"
            //.uri = "mqtts://mqtt.eclipse.org:8883",
            .uri = "mqtt://mqtt.eclipse.org:1883"
            //.client_id = "QBO_1",
            //.host = "esseci-staging.crispybacon.us",
            //.port = 8443,

            //.uri = "mqtts://esseci-staging.crispybacon.us:8443",

            //.cert_pem = (const char *)mqtt_eclipse_org_pem_start,
            //.cert_pem = (const char *)cert_chain_pem_start,
            //.cert_pem = (const char *)esseci_pem_start,

            //.cert_pem = (const char *)esseci_der_der_start,
            //.cert_len = esseci_pem_end - esseci_pem_start,



    };

    puts("DEBUG 1");
    client = esp_mqtt_client_init(&mqtt_cfg);

    puts("DEBUG 2");



    puts("DEBUG 3");

    esp_err_t connection = ESP_FAIL;

    while (connection != ESP_OK) {
        connection = esp_mqtt_client_start(client);
        ESP_LOGI(mqttTAG, "Client started");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);


    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);

    puts("DEBUG 4");
}
void subscribe(char *topic) {
    esp_mqtt_client_subscribe(client, topic, 0);
}

void publish(char *topic, char *message) {
    esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    //int response = esp_mqtt_client_publish(client, "/private/topic/00", "data_3", 0, 1, 0);
    ESP_LOGI(mqttTAG, "message publish on topic" );
}


/*
void app_main()
{
    ESP_LOGI(mqttTAG, "[APP] Startup..");
    ESP_LOGI(mqttTAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(mqttTAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    * This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     *
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
} */
