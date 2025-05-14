#include "egg_demo_lib.h"

#include "lvgl.h"
#define TAG "BLE_WIFI"



bool prov;
bool is_ble_initialized =false;



    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
    const char *pop = "abcd1234";  // Proof of Possession
    const char *service_name = "PROV_ESP32";  // BLE 廣播名稱
    const char *service_key = NULL;  // 可設定密鑰


static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI("LVGL", "🔘 按鈕被按下了！");
    }
}

void show_lvgl_button(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());    // 在主畫面上建立按鈕
    // lv_obj_center(btn);                              // 將按鈕置中
    lv_obj_set_size(btn, 120, 50);  // 設定大小
    lv_obj_set_pos(btn, 50, 100);   // ➜ 設定位置 (X, Y)
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);  // 加入 callback

    lv_obj_t * label = lv_label_create(btn);         // 建立標籤放在按鈕上
    lv_label_set_text(label, "on/off");
    lv_obj_center(label);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "✅ Wi-Fi 連線成功!");
        LCD_PrintText("BLE Provisioning_success");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGE(TAG, "⚠️ Wi-Fi 斷線，重新嘗試...");
        esp_wifi_connect();
    }
}
static void wifi_init() {
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "🚀 Wi-Fi 初始化完成");
}
static void prov_event_handler(void *user_data, wifi_prov_cb_event_t event, void *event_data) {
    switch (event) {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "📡 BLE Provisioning 開始...");
            break;
        case WIFI_PROV_CRED_RECV: {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "📥 接收到 Wi-Fi 設定 -> SSID: %s, 密碼: %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
           
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "✅ Provisioning 成功");
            break;
        case WIFI_PROV_END:
            ESP_LOGI(TAG, "⚡ Provisioning 結束");
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
    }
}

void blu_prov()
{
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&prov));  // <<== 加上這行！正確讀取狀態

    if (!prov) {
        ESP_LOGI(TAG, "🔵 沒憑證，啟動 BLE 配對...");
        LCD_PrintText("BLE Provisioning_begin");
        if (!is_ble_initialized) {
            wifi_prov_mgr_config_t config = {
                .scheme = wifi_prov_scheme_ble,
                .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
            };
            ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
            is_ble_initialized = true;
        }
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));
    } else {
        ESP_LOGI(TAG, "📶 已有憑證，直接連線 Wi-Fi");
        LCD_PrintText("No need to connect to Bluetooth");
        wifi_config_t current_conf;
    esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &current_conf);
    if (err == ESP_OK)
    {
    ESP_LOGI("WIFI", "✅ Connected to SSID: %s", (char *)current_conf.sta.ssid);
    ESP_LOGI("WIFI", "🔐 Password used : %s", (char *)current_conf.sta.password);
    }

        // 這邊不用再 set_mode 和 start了，因為上面已經 start Wi-Fi
    }
}
void nvs_init()
{
    ESP_LOGI(TAG, "初始化nvs");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}
void LCD_PrintText(const char *text)
{
    lv_obj_clean(lv_scr_act());  // 清除畫面（避免重複堆疊）
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void EGG_main(void)
{
    nvs_init();
    // ST77916_Init();
    // LVGL_Init();
    Set_Backlight(100);  // 開啟背光

    wifi_init();
    // ✅ 啟動 BLE Provisioning
    blu_prov();
    show_lvgl_button();
    

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
