/*
 * gatt_level.h
 *
 *  Created on: 2 дек. 2018 г.
 *      Author: lvr_laptop
 */

#ifndef MAIN_BLE_APP_GATT_LEVEL_INCLUDE_GATT_LEVEL_H_
#define MAIN_BLE_APP_GATT_LEVEL_INCLUDE_GATT_LEVEL_H_

#include "../../global_incl.h"

#define PROFILE_NUM				(1)
#define PROFILE_A_APP_ID		(0)
#define SERVICE_QTY				(2)
#define SERVICE_CHARACS_MAX		(2)
#define INVALID_HANDLE  		(0)

#define DOOR_SERV_NUM			(0)
#define BATT_SERV_NUM			(1)
#define GERCON_CHARAC_NUM		(0)
#define RELAY_CHARAC_NUM		(1)
#define ADCLVL_CHARAC_NUM		(0)
#define STATE_OPEN				(0xCC)

void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

struct DoorProf_s
{
	uint32_t batLevel;
	char	 doorOpen;
	char	 drCmdCode;
};

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

struct LVR_Charac_s
{
	uint16_t handle;
	uint8_t  val;
};

typedef struct
{
	esp_bt_uuid_t characsUuids[SERVICE_CHARACS_MAX];
}LVR_ServCharacs_s;

struct LVR_GattServer_s
{
	esp_bt_uuid_t     serviceUuids[SERVICE_QTY];
	uint8_t  	      serviceToken;
	LVR_ServCharacs_s charac[SERVICE_QTY];
};


#endif /* MAIN_BLE_APP_GATT_LEVEL_INCLUDE_GATT_LEVEL_H_ */
