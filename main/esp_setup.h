//
// Created by Michele Mocellin on 11/08/2020.
//

#ifndef SOLDER_CUBE_ESP_SETUP_H
#define SOLDER_CUBE_ESP_SETUP_H

#endif //SOLDER_CUBE_ESP_SETUP_H

#define FIRST_SETUP_BIT         (1UL << 0UL)

#define ETH_MODE_BIT            (1UL << 1UL)
#define WIFI_MODE_BIT           (1UL << 2UL)
#define USB_MODE_BIT            (1UL << 3UL)

#define ETH_LINKED              (1UL << 4UL)
#define ETH_CONNECTED_BIT       (1UL << 5UL)
#define ETH_DISCONNECTED_BIT    (1UL << 6UL)
#define ETH_FAIL_BIT            (1UL << 7UL)

#define WIFI_ACTIVED            (1UL << 8UL)
#define WIFI_CONNECTED_BIT      (1UL << 9UL)
#define WIFI_DISCONNECTED_BIT   (1UL << 10UL)
#define WIFI_FAIL_BIT           (1UL << 11UL)





void espSetup(void);

