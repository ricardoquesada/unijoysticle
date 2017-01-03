#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

static xQueueHandle gpio_evt_queue = NULL;

static int g_level = 0;

void IRAM_ATTR gpio_isr_handler_up(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    g_level = 1;
}

void IRAM_ATTR gpio_isr_handler_down(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    g_level = 0;
}

void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            //ESP_ERROR_CHECK( gpio_set_level(GPIO_NUM_5, cnt % 2) );
        }
    }
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}


void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "queque2",
            .password = "locopajaro",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    // internal LED
    ESP_ERROR_CHECK( gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT) );
    ESP_ERROR_CHECK( gpio_set_level(GPIO_NUM_5, 1) );

    // read POT X
//    gpio_config_t io_conf;
//    io_conf.intr_type = GPIO_INTR_POSEDGE;
//    io_conf.mode = GPIO_MODE_INPUT;
    // PotX, PotY from port#1 and port#2
//    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_36);
//    io_conf.pull_down_en = 0;
//    io_conf.pull_up_en = 1;
//    ESP_ERROR_CHECK( gpio_config(&io_conf) );

    ESP_ERROR_CHECK( gpio_set_direction(GPIO_NUM_21, GPIO_MODE_INPUT) );
    ESP_ERROR_CHECK( gpio_set_intr_type(GPIO_NUM_21, GPIO_INTR_NEGEDGE) );
    ESP_ERROR_CHECK( gpio_set_pull_mode(GPIO_NUM_21, GPIO_PULLUP_ONLY) );
    ESP_ERROR_CHECK( gpio_intr_enable(GPIO_NUM_21) );

    //create a queue to handle gpio event from isr
//    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
//    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    ESP_ERROR_CHECK( gpio_install_isr_service(0) );

    //hook isr handler for specific gpio pin
//    ESP_ERROR_CHECK( gpio_isr_handler_add(GPIO_NUM_36, gpio_isr_handler_up, (void*) GPIO_NUM_36) );
    ESP_ERROR_CHECK( gpio_isr_handler_add(GPIO_NUM_21, gpio_isr_handler_down, (void*) GPIO_NUM_21) );

    while(1) {
        //vTaskDelay(1000 / portTICK_RATE_MS);
        while (g_level == 1) {
        }
        gpio_set_level(GPIO_NUM_5, g_level);
        int i=0;
        for (i=0; i<8000; i++)
            __asm__("nop");
        g_level = 1;
        gpio_set_level(GPIO_NUM_5, g_level);
    }
}
