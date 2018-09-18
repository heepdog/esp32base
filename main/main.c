#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <u8g2.h>

#include <freertos/event_groups.h>

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"
#include "display.h"
#include "udp_broadcast.h"

// SDA - GPIO5
#define PIN_SDA 5

// SCL - GPIO4
#define PIN_SCL 4

static const char *TAG = "ssd1306";

void task_test_SSD1306i2c(void *ignore) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda   = PIN_SDA;
	u8g2_esp32_hal.scl  = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);


	u8g2_t u8g2; // a structure which will contain all the data for one display

	u8g2_Setup_ssd1306_128x64_noname_f(
			&u8g2,
			U8G2_R0,
			u8g2_esp32_msg_i2c_cb,
			u8g2_esp32_msg_i2c_and_delay_cb);  // init u8g2 structure

	int height=55;

		u8x8_SetI2CAddress(&u8g2.u8x8,0x78);

		u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

		u8g2_SetPowerSave(&u8g2, 0); // wake up display
		do {

		u8g2_ClearBuffer(&u8g2);

		u8g2_DrawBox(&u8g2, 0, height, 100,6);
		u8g2_DrawFrame(&u8g2, 0,height,122,6);

		height=5+height;
		if (height >= 60){
			height = 30;

		}

		u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);

		u8g2_DrawStr(&u8g2, 0,14,"Hi Kendra!!");

		u8g2_SendBuffer(&u8g2);

		ESP_LOGI(TAG, "draw done");

		vTaskDelay(300 / portTICK_PERIOD_MS);

	} while (1);
	vTaskDelete(NULL);
}


char my_ip[32];

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
const int STARTED_BIT = BIT1;


static esp_err_t esp32_wifi_eventHandler(void *ctx, system_event_t *event) {

	switch(event->event_id) {
        case SYSTEM_EVENT_WIFI_READY:
        	ESP_LOGD(TAG, "EVENT_WIFI_READY");

            break;

        case SYSTEM_EVENT_AP_STACONNECTED:
        	ESP_LOGD(TAG, "EVENT_AP_START");
            break;

		// When we have started being an access point
		case SYSTEM_EVENT_AP_START:
        	ESP_LOGD(TAG, "EVENT_START");
            xEventGroupSetBits(wifi_event_group, STARTED_BIT);

			break;
        case SYSTEM_EVENT_SCAN_DONE:
        	ESP_LOGD(TAG, "EVENT_SCAN_DONE");
			break;

		case SYSTEM_EVENT_STA_CONNECTED:
       		ESP_LOGD(TAG, "EVENT_STA_CONNECTED");
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;

		// If we fail to connect to an access point as a station, become an access point.
		case SYSTEM_EVENT_STA_DISCONNECTED:
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);

			ESP_LOGD(TAG, "EVENT_STA_DISCONNECTED");
			// We think we tried to connect as a station and failed! ... become
			// an access point.
			break;

		// If we connected as a station then we are done and we can stop being a
		// web server.
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGD(TAG, "********************************************");
			ESP_LOGD(TAG, "* We are now connected to AP")
			ESP_LOGD(TAG, "* - Our IP address is: " IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
			ESP_LOGD(TAG, "********************************************");
            {
                //u32_t *my_ip=(u32_t *)&event->event_info.got_ip.ip_info.ip;
	        sprintf(my_ip,IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
            }
            //printf("Startinng send\n");
            xTaskCreate(&task_UDP_broadcast, "send_thread", 2048, &my_ip, 5, NULL);

			break;

		default: // Ignore the other event types
			break;
	} // Switch event

	return ESP_OK;
} // esp32_wifi_eventHandler


void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(esp32_wifi_eventHandler, NULL) );


//    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "heppners-n",
            .password = "Tinroof36",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;

    //xTaskCreate(&myTask, "myTask", 2048, NULL, 5, NULL);
    xTaskCreate(&task_test_SSD1306i2c, "displayTask", 2048, NULL, 5, NULL);
    while (true) {
        gpio_set_level(GPIO_NUM_4, level);
        level = !level;
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}



