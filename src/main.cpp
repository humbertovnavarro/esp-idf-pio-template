#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "WIFI_AP";

// Task: just prints hello every second
void hello_task(void *pvParameters) {
    while (true) {
        printf("Hello, world!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void) {
    // --- Init NVS (required for Wi-Fi) ---
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // --- Init TCP/IP stack & event loop ---
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // --- Wi-Fi config ---
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t ap_config = {};
    strcpy((char*)ap_config.ap.ssid, "ESP32_Hotspot");   // SSID
    ap_config.ap.ssid_len = strlen("ESP32_Hotspot");
    strcpy((char*)ap_config.ap.password, "12345678");    // WPA2 password
    ap_config.ap.max_connection = 4;                     // up to 4 clients
    ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    // --- Start Wi-Fi ---
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Hotspot started! SSID:%s  Password:%s",
             ap_config.ap.ssid, ap_config.ap.password);

    // --- Create Hello World task ---
    xTaskCreate(hello_task, "hello_task", 2048, NULL, 5, NULL);
}
