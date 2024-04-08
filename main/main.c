/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void write_package(adc_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF ;

    uart_putc_raw(uart0, data.axis);
    uart_putc_raw(uart0, lsb);
    uart_putc_raw(uart0, msb);
    uart_putc_raw(uart0, -1);
}

void x_task() {
    adc_gpio_init(26);

    adc_t x_command;
    int values_x[5] = {0,0,0,0,0};
    int cont_x = 0;

    while (true) {
        adc_select_input(0);
        int result_x = adc_read();
        result_x = (result_x/8) - 255;

        if (result_x > 30 || result_x < -30) {
            values_x[cont_x%5] = result_x;
            cont_x++;
            
            int soma_x = 0;
            for (int i = 0; i < 5; i++) {
                soma_x += values_x[i];
            }

            x_command.val = soma_x/5;
            x_command.axis = 0;
            xQueueSend(xQueueAdc, &x_command, 0);
        }
    }
}

void y_task() {
    adc_gpio_init(27);

    adc_t y_command;
    int values_y[5] = {0,0,0,0,0};
    int cont_y = 0;

    while (true) {
        adc_select_input(1);
        int result_y = adc_read();
        result_y = (result_y/8) - 255;

        if (result_y > 30 || result_y < -30) {
            values_y[cont_y%5] = result_y;
            cont_y++;
            
            int soma_y = 0;
            for (int i = 0; i < 5; i++) {
                soma_y += values_y[i];
            }

            y_command.val = soma_y/5;
            y_command.axis = 1;
            xQueueSend(xQueueAdc, &y_command, 0);
        }
    }
}

void uart_task(void *p) {
    adc_t data;

    while (1) {
        if (xQueueReceive(xQueueAdc, &data, portMAX_DELAY)) {
            write_package(data);
        }
    }
}

int main() {
    stdio_init_all();
    adc_init();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(x_task, "x_task", 4096, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4096, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}


