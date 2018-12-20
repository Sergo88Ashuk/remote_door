/*
 * gatt_level.c
 *
 *  Created on: 2 дек. 2018 г.
 *      Author: lvr_laptop
 */


#include "include/gatt_level.h"

const static char *GATTC_TAG = "BLE_CORE";

#define DOOR_SERV_UUID		{0x2C, 0xB3, 0x4E, 0x53, 0x7B, 0xB4, 0x8B, 0xA5,\
							 0x6D, 0x42, 0x9B, 0x3A, 0x23, 0x15, 0x78, 0xAB}

#define BATT_SERV_UUID		{0x2E, 0xB3, 0x4E, 0x53, 0x7B, 0xB4, 0x8B, 0xA5,\
							 0x6D, 0x42, 0x9B, 0x3A, 0x23, 0x15, 0x78, 0xAB}

#define GERCON_CHARAC		{0x2C, 0xB3, 0x4E, 0x53, 0x7B, 0xB4, 0x8B, 0xA5, \
							 0x6D, 0x42, 0x9B, 0x3A, 0x25, 0x16, 0x78, 0xAB}

#define RELAY_CHARAC		{0x2C, 0xB3, 0x4E, 0x53, 0x7B, 0xB4, 0x8B, 0xA5, \
							 0x6D, 0x42, 0x9B, 0x3A, 0x26, 0x16, 0x78, 0xAB}

#define ADCLVL_CHARAC		{0x2E, 0xB3, 0x4E, 0x53, 0x7B, 0xB4, 0x8B, 0xA5, \
							 0x6D, 0x42, 0x9B, 0x3A, 0x25, 0x15, 0x78, 0xAB}

#define CHECK_STATUS(st)	{if (st != ESP_GATT_OK) {ESP_LOGE(GATTC_TAG, "ERROR_GATTC [0x%02X] _%d_", st, __LINE__);}}

extern struct DoorProf_s doorBle;

bool connectBle = false;
bool get_server = false;

struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};
static esp_gattc_char_elem_t char_elem_result;

struct LVR_GattServer_s serverUuids =
{
	.serviceUuids =
	{
		{.len = ESP_UUID_LEN_128, .uuid = {.uuid128 = DOOR_SERV_UUID,}},
		{.len = ESP_UUID_LEN_128, .uuid = {.uuid128 = BATT_SERV_UUID,}},
	},
	.serviceToken = 0,
	.charac =
	{
		{.characsUuids =
			{
				{.len = ESP_UUID_LEN_128, . uuid = {.uuid128 = GERCON_CHARAC}},
				{.len = ESP_UUID_LEN_128, . uuid = {.uuid128 = RELAY_CHARAC}}
			}
		},
		{.characsUuids =
			{
				{.len = ESP_UUID_LEN_128, . uuid = {.uuid128 = ADCLVL_CHARAC}},
//				{.len = ESP_UUID_LEN_128, . uuid = {.uuid128 = GERCON_CHARAC}}
			}
		}
	}
};

struct LVR_Charac_s characHandles[SERVICE_QTY][SERVICE_CHARACS_MAX];

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            ESP_LOGI(GATTC_TAG, "reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(GATTC_TAG, "REG_EVT");
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "set scan params error, error code = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d", p_data->connect.conn_id, gattc_if);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTC_TAG, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    }
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "open failed, status %d", p_data->open.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "open success");
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG,"config mtu failed, error status = %x", param->cfg_mtu.status);
        }
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
//        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &doorService);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(GATTC_TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(GATTC_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);

        serverUuids.serviceToken = 0xFF;
        uint8_t hitCnt;

        for (uint8_t srv = 0; srv < SERVICE_QTY; srv++)
        {
        	hitCnt = 0;
        	for (uint8_t i = 0; i < ESP_UUID_LEN_128; i++)
            {
            	if (p_data->search_res.srvc_id.uuid.uuid.uuid128[i] != serverUuids.serviceUuids[srv].uuid.uuid128[i])
            	{
            		break;
            	}
            	hitCnt++;
            }
        	if (hitCnt == ESP_UUID_LEN_128) {serverUuids.serviceToken = srv;}
        }

        if ((p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (serverUuids.serviceToken == DOOR_SERV_NUM)) {
        	ESP_LOGI(GATTC_TAG, "door service found");
        	get_server = true;
        	gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
        	gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        if ((p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (serverUuids.serviceToken == BATT_SERV_NUM)) {
        	ESP_LOGI(GATTC_TAG, "battery service found");
        	get_server = true;
        	gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
        	gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
    	CHECK_STATUS(p_data->search_cmpl.status);

        if(p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE)
        {
            ESP_LOGI(GATTC_TAG, "Get service information from remote device");
        }
        else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH)
        {
            ESP_LOGI(GATTC_TAG, "Get service information from flash");
        }
        else
        {
            ESP_LOGI(GATTC_TAG, "unknown service source");
        }
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SEARCH_CMPL_EVT");

        if (get_server)
        {
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            CHECK_STATUS(status);
            ESP_LOGI(GATTC_TAG, "characs qty found [%d]", count);
            if (count > 0)
            {
            	for (uint8_t chs = 0; chs < count; chs++)
            	{
            		uint16_t cnt;
					status = esp_ble_gattc_get_char_by_uuid(gattc_if,
															p_data->search_cmpl.conn_id,
															gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
															gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
															serverUuids.charac[serverUuids.serviceToken].characsUuids[chs],
															&char_elem_result,
															&cnt);
					CHECK_STATUS(status);
					characHandles[serverUuids.serviceToken][chs].handle = char_elem_result.char_handle;
					ESP_LOGI(GATTC_TAG, "handl got [0x%02X] [0x%02X]", characHandles[serverUuids.serviceToken][chs].handle, char_elem_result.char_handle);
					status = esp_ble_gattc_read_char(gattc_if, p_data->search_cmpl.conn_id, char_elem_result.char_handle, ESP_GATT_AUTH_REQ_NONE);
					CHECK_STATUS(status);

					if ((serverUuids.serviceToken == DOOR_SERV_NUM) &&
							(chs == RELAY_CHARAC_NUM))
					{
						status = esp_ble_gattc_write_char(gattc_if,
														  p_data->search_cmpl.conn_id,
														  char_elem_result.char_handle,
														  1,
														  (unsigned char*) &doorBle.drCmdCode,
														  ESP_GATT_WRITE_TYPE_NO_RSP,
														  ESP_GATT_AUTH_REQ_NONE);
						CHECK_STATUS(status);
					}
            	}
            }
            else
            {
                ESP_LOGE(GATTC_TAG, "no char found");
            }
        }
        break;
    }

    case ESP_GATTC_READ_CHAR_EVT: {
    	for(uint8_t srv = 0; srv < SERVICE_QTY; srv++)
    	{
			for(uint8_t chs = 0; chs < SERVICE_CHARACS_MAX; chs++)
			{
				if (characHandles[srv][chs].handle == param->read.handle)
				{
					characHandles[srv][chs].val = *param->read.value;
					ESP_LOGI(GATTC_TAG, "v[0x%02X] to h[0x%02X]", (unsigned int) *param->read.value, (unsigned int) param->read.handle);
				}
			}
    	}

        break;
    }

    case ESP_GATTC_READ_MULTIPLE_EVT: {
    	ESP_LOGE(GATTC_TAG, "Mult read");
    	break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        ESP_LOGE(GATTC_TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
        break;
    }

    case ESP_GATTC_NOTIFY_EVT:
        if (p_data->notify.is_notify){
            ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
        }else{
            ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
        }
        esp_log_buffer_hex(GATTC_TAG, p_data->notify.value, p_data->notify.value_len);
        break;

    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write descr success ");
        break;

    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(GATTC_TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }

    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "write char failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write char success ");
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        connectBle = false;
        get_server = false;
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
        break;

    default:
        break;
    }
}
