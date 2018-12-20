/*
 * global_incl.h
 *
 *  Created on: 18 но€б. 2018 г.
 *      Author: lvr_laptop
 */

#ifndef MAIN_GLOBAL_INCL_H_
#define MAIN_GLOBAL_INCL_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_wifi.h"

#include "bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "web_app/httpmg_server/include/mongoose.h"
#include "web_app/wifi_station/include/wifi_station.h"
#include "ble_app/gap_level/include/gap_level.h"
#include "ble_app/gatt_level/include/gatt_level.h"
#endif /* MAIN_GLOBAL_INCL_H_ */
