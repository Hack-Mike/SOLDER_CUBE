//
// Created by Michele Mocellin on 06/09/2020.
//

#ifndef SOLDER_CUBE_MQTT_H
#define SOLDER_CUBE_MQTT_H

#endif //SOLDER_CUBE_MQTT_H



static const char *mqttTAG = "MQTT: ";

void mqtt_app(void);

void publish(char *topic, char *message);

void subscribe(char *topic);

