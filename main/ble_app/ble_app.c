/*
 * ble_app.c
 *
 *  Created on: 2 дек. 2018 г.
 *      Author: lvr_laptop
 */

#include "../global_incl.h"

extern QueueHandle_t xMailboxIndoor;
extern QueueHandle_t xMailboxOutdoor;

struct DoorProf_s doorBle;

void BleAppCore1(void *pvParameter)
{
	const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;
	const static char *GATTC_TAG = "BLE_CORE";

	memset(&doorBle, 0, sizeof(struct DoorProf_s));
	xQueueOverwrite(xMailboxOutdoor, &doorBle);

	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	esp_err_t ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_init();
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(GATTC_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
		return;
	}

	//register the  callback function to the gap module
	ret = esp_ble_gap_register_callback(esp_gap_cb);
	if (ret){
		ESP_LOGE(GATTC_TAG, "%s gap register failed, error code = %x\n", __func__, ret);
		return;
	}

	//register the callback function to the gattc module
	ret = esp_ble_gattc_register_callback(esp_gattc_cb);
	if(ret){
		ESP_LOGE(GATTC_TAG, "%s gattc register failed, error code = %x\n", __func__, ret);
		return;
	}

	ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
	if (ret){
		ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
	}
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
	if (local_mtu_ret){
	ESP_LOGE(GATTC_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
	}

#if 1

	extern struct gattc_profile_inst gl_profile_tab[PROFILE_NUM];
	extern struct LVR_GattServer_s serverUuids;
	extern struct LVR_Charac_s characHandles[SERVICE_QTY][SERVICE_CHARACS_MAX];

	esp_err_t status;
	BaseType_t mailStat;

	while (true)
	{
		vTaskDelay(xDelay);

		mailStat = xQueuePeek(xMailboxIndoor, &doorBle, 100);
		if (mailStat != pdTRUE)
		{
			doorBle.drCmdCode = '0';
		}

		mailStat = xQueueReset(xMailboxIndoor);
		if (mailStat != pdPASS) {ESP_LOGE(GATTC_TAG, "RST Q");}

		ESP_LOGI(GATTC_TAG, "try to open from app");
		ESP_LOGI(GATTC_TAG, "with lock req [%c]", doorBle.drCmdCode);

		status = esp_ble_gattc_search_service(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,
									 gl_profile_tab[PROFILE_A_APP_ID].conn_id,
									 &serverUuids.serviceUuids[DOOR_SERV_NUM]);
		if (status != ESP_GATT_OK){
			ESP_LOGE(GATTC_TAG, "door serv search fail[0x%02X]", status);
		}

		status = esp_ble_gattc_search_service(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,
									gl_profile_tab[PROFILE_A_APP_ID].conn_id,
									&serverUuids.serviceUuids[BATT_SERV_NUM]);
		if (status != ESP_GATT_OK){
			ESP_LOGE(GATTC_TAG, "battery serv search fail[0x%02X]", status);
		}

		doorBle.batLevel = characHandles[BATT_SERV_NUM][ADCLVL_CHARAC_NUM].val;
		doorBle.doorOpen = characHandles[DOOR_SERV_NUM][GERCON_CHARAC_NUM].val;

		xQueueOverwrite(xMailboxOutdoor, &doorBle);
	}
}
#endif

