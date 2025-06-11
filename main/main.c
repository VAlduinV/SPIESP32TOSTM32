#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "rom/gpio.h"
#include <string.h>

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#define PIN_NUM_LED  2  // встроенный светодиод на ESP32 DevKit

static const char *TAG = "SPI_MASTER";

void app_main(void)
{
    esp_err_t ret;

    // Настройка GPIO для LED как выход
    gpio_pad_select_gpio(PIN_NUM_LED);
    gpio_set_direction(PIN_NUM_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_LED, 0);  // выключен изначально

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .mode = 0,                          // SPI Mode 0
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };

    spi_device_handle_t spi;
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    uint8_t tx_data = 0x42;
    uint8_t rx_data;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;             // 8 бит (1 байт)
    t.tx_buffer = &tx_data;
    t.rx_buffer = &rx_data;

    while (1) {
        ret = spi_device_transmit(spi, &t);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Sent: 0x%02X, Received: 0x%02X", tx_data, rx_data);

            // Включаем LED (моргаем)
            gpio_set_level(PIN_NUM_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));  // горит 100 мс

            // Выключаем LED
            gpio_set_level(PIN_NUM_LED, 0);
        } else {
            ESP_LOGE(TAG, "SPI transmit failed");
            gpio_set_level(PIN_NUM_LED, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(900));  // ждем до следующей передачи (итого 1 секунда)
    }
}
