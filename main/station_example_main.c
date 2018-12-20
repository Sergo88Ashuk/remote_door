/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "global_incl.h"
#include "web_app/include/web_app.h"
#include "ble_app/include/ble_app.h"

QueueHandle_t xMailboxIndoor;
QueueHandle_t xMailboxOutdoor;

void app_main()
{
	const static char *TAG = "MAIN";
	///////////////////////////////////////////////
	//	ESP resources init
	///////////////////////////////////////////////

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

	///////////////////////////////////////////////
	//	FREERTOS init
	///////////////////////////////////////////////

	// Mailbox init
	xMailboxIndoor = xQueueCreate(1, sizeof(struct DoorProf_s));
	xMailboxOutdoor = xQueueCreate(1, sizeof(struct DoorProf_s));

	// Tasks init
	portBASE_TYPE res;
	res = xTaskCreatePinnedToCore(&WebAppCore0, "web_application", 16384, NULL, 8, NULL, 0);
	ESP_LOGI(TAG, "web task stat: %d\n", (unsigned int) res);
	res = xTaskCreatePinnedToCore(&BleAppCore1, "ble_application", 16384, NULL, 8, NULL, 1);
	ESP_LOGI(TAG, "ble task stat: %d\n", (unsigned int) res);

}
