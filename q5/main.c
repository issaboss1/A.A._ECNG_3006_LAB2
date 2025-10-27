//816035242
//question 5 lab2 
//task- Update your application - using either a semaphore or a message to replace the mutex for inter-task communication.

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"

#define LED_PIN 2

const int LED_ON_TASK_PRIORITY = 3;
const int LED_OFF_TASK_PRIORITY = 2;
const int STATUS_TASK_PRIORITY = 1;

SemaphoreHandle_t xBinarySemaphore;//binary semaphore
volatile uint32_t idle_counter = 0;

void light_sleep_ms(uint32_t ms)// function to enter short light sleep, then yield
{
    esp_sleep_enable_timer_wakeup(ms*1000);//converts the millisecs to microsecs
    esp_light_sleep_start();
    taskYIELD(); 
}

void vApplicationIdleHook(void)//Idle hook function
{
    idle_counter++;
}

//task 1
void led_on_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            gpio_set_level(LED_PIN, 1);
            printf("[LED_ON_TASK] LED ON\n");
            xSemaphoreGive(xBinarySemaphore);
        }
        light_sleep_ms(500);
    }
}

//task 2
void led_off_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)//wait for mutex
        {
            gpio_set_level(LED_PIN, 0);//turns led off for 1s
            xSemaphoreGive(xMutex);
        }
        light_sleep_ms(1000);
        printf("[LED_OFF_TASK] Gave semaphore, LED_ON_TASK may run\n");
    }
}

//task 3
void status_task(void *pvParameters)
{
    uint32_t prev_idle = 0;

    while (1)
    {
        uint32_t current_idle = idle_counter;
        uint32_t diff = current_idle - prev_idle;
        prev_idle = current_idle;

        printf("[STATUS_TASK] Free heap: %u bytes | Idle count: %u\n",
               esp_get_free_heap_size(), diff);

        light_sleep_ms(1000);
    }
}


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

    // create binary semaphore
    xBinarySemaphore = xSemaphoreCreateBinary();
    if (xBinarySemaphore == NULL)
    {
        printf("Failed to create binary semaphore!\n");
        return;
    }

    // Initially give semaphore to start LED_ON_TASK
    xSemaphoreGive(xBinarySemaphore);

    // Create tasks
    xTaskCreate(led_on_task, "LED_ON_TASK", 2048, NULL, LED_ON_TASK_PRIORITY, NULL);
    xTaskCreate(led_off_task, "LED_OFF_TASK", 2048, NULL, LED_OFF_TASK_PRIORITY, NULL);
    xTaskCreate(status_task, "STATUS_TASK", 2048, NULL, STATUS_TASK_PRIORITY, NULL);

    printf("Tasks started. LED pin: GPIO%d\n", LED_PIN);
}
