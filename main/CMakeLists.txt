idf_component_register(SRCS  "app_main.c" "esp_setup.c" "WiFi-STA.c" "ethernet.c" "button.c" "JsonUtility.cpp" "mqtt.c"

        INCLUDE_DIRS ""
        EMBED_TXTFILES ${project_dir}/Certificate/MQTT/mqtt_eclipse_org.pem)

add_subdirectory(/Users/michelemocellin/Library/Arduino15/staging/libraries/ArduinoJson-6.16.1 ArduinoJson)

target_link_libraries( ${COMPONENT_LIB} PUBLIC ArduinoJson)