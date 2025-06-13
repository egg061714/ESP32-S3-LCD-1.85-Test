#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "nvs_flash.h"
#include "protocomm_security.h"
#include "esp_mesh.h"
#include "esp_mac.h" 


// #include "qrcode.h"
#include "esp_lcd_panel_ops.h"
#include "ST77916.h"          // 你原本的 LCD 驅動，內含 panel_handle

#include "cJSON.h"

void send_mac_to_root();


// void bl_pwm_set_percent(uint8_t percent);
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void wifi_init();

void blu_prov();
static void prov_event_handler(void *user_data, wifi_prov_cb_event_t event, void *event_data);
static void start_ble_provisioning();
static void btn_event_cb(lv_event_t * e);
void show_lvgl_button(void);
void nvs_init();
void mesh_commicate();
void LCD_PrintText(const char *text);
void EGG_main(void);