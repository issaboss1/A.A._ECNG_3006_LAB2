//student id - 816035242
//question 3(c)
//Task 1 has the highest priority, then Task 2, then Task 3
//priority inheritance enabled using a mutex

#include <stdio.h>
#include "freertos/FreeRTOS.h"//freertos core
#include "freertos/task.h"// freertos task
#include "freertos/semphr.h"//semaphore function
#include "driver/gpio.h"//controls led pin
#include "esp_system.h"
#include "esp_sleep.h"//for the sleep function
#include "esp_log.h"

#define LED_PIN 2

const int LED_ON_TASK_PRIORITY = 3;   // Highest
const int LED_OFF_TASK_PRIORITY = 2;  // Middle
const int STATUS_TASK_PRIORITY = 1;   // Lowest

SemaphoreHandle_t xMutex;//creates mutex

// short sleep function
void light_sleep_ms(uint32_t ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_light_sleep_start();
    taskYIELD();
}

// task 1
void led_on_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            gpio_set_level(LED_PIN, 1);
            printf("[LED_ON_TASK] LED ON\n");
            light_sleep_ms(500);
            xSemaphoreGive(xMutex);
        }
    }
}

// task 2
void led_off_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            gpio_set_level(LED_PIN, 0);
            printf("[LED_OFF_TASK] LED OFF\n");
            light_sleep_ms(1000);
            xSemaphoreGive(xMutex);
        }
    }
}

// Task 3: Status
void status_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            printf("[STATUS_TASK] Free heap = %u bytes\n", esp_get_free_heap_size());
            xSemaphoreGive(xMutex);
        }
        light_sleep_ms(1000);
    }
}

// Main setup
void app_main(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    // Create mutex
    xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL)
    {
        printf("Mutex creation failed\n");
        return;
    }

    // Create tasks with different priorities
    xTaskCreate(led_on_task, "LED_ON_TASK", 2048, NULL, LED_ON_TASK_PRIORITY, NULL);
    xTaskCreate(led_off_task, "LED_OFF_TASK", 2048, NULL, LED_OFF_TASK_PRIORITY, NULL);
    xTaskCreate(status_task, "STATUS_TASK", 2048, NULL, STATUS_TASK_PRIORITY, NULL);

    printf("Tasks started with priority inheritance via mutex\n");
}
