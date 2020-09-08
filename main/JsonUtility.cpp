//
// Created by Michele Mocellin on 04/09/2020.
//

#include <stdio.h>
#include <ArduinoJson.h>
#include "JsonUtility.hpp"

#include "mqtt.h"

char bufferSize[200];

char * jsonFunction(void) {

    const size_t capacity = JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    doc["serialNumber"] = "a0a1-a2a3-a4a5";
    doc["timeStamp"] = 1351824120;
    doc["level"] = 20.52;

    serializeJson(doc, bufferSize);

    printf("Print Json -> %s\n", bufferSize);

    return bufferSize;

}

