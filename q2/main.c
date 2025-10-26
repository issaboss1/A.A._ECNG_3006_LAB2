//Student id - 816035242
//lab2 question 2-
//task 1 - Two tasks should share a single GPIO pin connected to an LED, One task will turn the GPIO pin on, and actively wait for 0.5 seconds, before yielding
// task 2 - turn the GPIO pin off, and task-delay for 1 second. 
// task 3 - The GPIO pin should be managed using a mutex. The third task will print a status message via the serial UART, and task-delay for 1 second. 

#include <stdio.h>
#include "freertos/FreeRTOS.h"//freertos core
#include "freertos/task.h"// freertos task
#include "freertos/semphr.h"//semaphore function
#include "driver/gpio.h"//controls led pin
#include "esp_system.h"
#include "esp_sleep.h"//for the sleep function
#include "esp_log.h"

#define LED_PIN 2

//task priorities
const int LED_ON_TASK_PRIORITY = 3;
const int LED_OFF_TASK_PRIORITY = 2;
const int STATUS_TASK_PRIORITY = 1;

SemaphoreHandle_t xMutex;//mutex declaration

void light_sleep_ms(uint32_t ms)// function to enter short light sleep, then yield
{
    esp_sleep_enable_timer_wakeup(ms*1000);//converts the millisecs to microsecs
    esp_light_sleep_start();
    taskYIELD(); 
}

void led_on_task(void *pvParameters)//first task 
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)//wait for mutex
        {
            gpio_set_level(LED_PIN, 1);//turn led on
            xSemaphoreGive(xMutex);//releases mutex
        }
        light_sleep_ms(500);
    }
}

void led_off_task(void *pvParameters)// task 2
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)//wait for mutex
        {
            gpio_set_level(LED_PIN, 0);//turns led off for 1s
            xSemaphoreGive(xMutex);
        }
        light_sleep_ms(1000);
    }
}

void status_task(void *pvParameters)//task 3
{
    while (1)
    {
        printf("Free heap= %u bytes\n", esp_get_free_heap_size());//status message via uart
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
    gpio_config(&io_conf);//config led as ooutput

    xMutex = xSemaphoreCreateMutex();//creates mutex and test if it fails
    if (xMutex == NULL)
    {
        printf("Mutex creation failed!\n");
        return;
    }
    //to create task.

    xTaskCreate(led_on_task, "LED_ON_TASK", 2048, NULL, LED_ON_TASK_PRIORITY, NULL);
    xTaskCreate(led_off_task, "LED_OFF_TASK", 2048, NULL, LED_OFF_TASK_PRIORITY, NULL);
    xTaskCreate(status_task, "STATUS_TASK", 2048, NULL, STATUS_TASK_PRIORITY, NULL);

    printf("Tasks works\n");
}

