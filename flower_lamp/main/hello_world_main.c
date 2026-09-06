#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define RADAR_UART_NUM UART_NUM_2
#define RADAR_TXD_PIN  17
#define RADAR_RXD_PIN  16
#define RELAY_PIN      19

void init_radar_uart() {
    const uart_config_t uart_config = {
        .baud_rate = 256000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(RADAR_UART_NUM, &uart_config);
    uart_set_pin(RADAR_UART_NUM, RADAR_TXD_PIN, RADAR_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(RADAR_UART_NUM, 1024, 0, 0, NULL, 0);
}

void set_relay_state(bool lamp_on) {
    if (lamp_on) {
        gpio_set_level(RELAY_PIN, 0);
    } else {
        gpio_set_level(RELAY_PIN, 1);
    }
}

void radar_task(void *arg) {
    uint8_t data[128];
    printf("Radar Monitor Started...\n");

    gpio_reset_pin(RELAY_PIN);
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);

    bool level = false;

    while (1) {
        level = !level;
        gpio_set_level(RELAY_PIN, level);

        printf("DIODE GPIO %d -> %s (%s)\n", 
               RELAY_PIN,
               level ? "HIGH" : "LOW",
               level ? "3.3V" : "0V");

        // 500ms HIGH + 500ms LOW = 1000ms total period (1 Hz frequency)
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // while (1) {
    //     int len = uart_read_bytes(RADAR_UART_NUM, data, 128, 20 / portTICK_PERIOD_MS);

    //     if (len >= 13) {
    //         // Ищем заголовок пакета LD2410
    //         if (data[0] == 0xF4 && data[1] == 0xF3 && data[2] == 0xF2 && data[3] == 0xF1) {

    //             uint8_t presence_state = data[8]; // 0 - никого, 1+ - кто-то есть

    //             if (presence_state > 0) {
    //                 set_relay_state(false);
    //                 printf("[TARGET] Presence: %d | Lamp: OFF\n", presence_state);
    //             } else {
    //                 set_relay_state(true);
    //                 printf("[EMPTY] Presence: 0 | Lamp: ON\n");
    //             }
    //         }
    //     }
    // }
}

void app_main(void) {
    printf("=== FLOWER LAMP SYSTEM BOOT ===\n");

    init_radar_uart();

    xTaskCreate(radar_task, "radar_task", 4096, NULL, 10, NULL);
}
