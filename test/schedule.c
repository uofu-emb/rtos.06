#include <unity_config.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <unity.h>
#include "helper.c"

configRUN_TIME_COUNTER_TYPE first_stats, second_stats, elapsed_stats;
TickType_t elapsed_ticks;

void setUp(void) {}

void tearDown(void) {
    printf("primary %lld secondary %lld elapsed %lld (us) in %d ticks\n",
           first_stats, second_stats, elapsed_stats, elapsed_ticks);
}

void busy_busy(void *args)
{
    char *name = (char *)args;
    printf("start busy_busy %s\n", name);
    for (int i = 0; ; i++);
}

void busy_yield(void *args)
{
    char *name = (char *)args;
    printf("start busy_yield %s\n", name);
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}

void busy_sleep(void *args)
{
    char *name = (char *)args;
    printf("start busy_sleep %s\n", name);
    busy_wait_us(10000);
    sleep_ms(490);
}

SemaphoreHandle_t semaphore;
void priority_inversion(void *args)
{
    char *name = (char *)args;
    printf("start priority inversion %s\n", name);
    while (1) {
        xSemaphoreTake(semaphore, portMAX_DELAY);
        /* printf("got semaphore %s\n", name); */
        busy_wait_us(100000);
        xSemaphoreGive(semaphore);
    }
}

void test_priority_inversion(void)
{
    semaphore = xSemaphoreCreateBinary();
    run_analyzer(priority_inversion, tskIDLE_PRIORITY+(3), 0, &first_stats,
                 priority_inversion, tskIDLE_PRIORITY+(4), 1, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT(1000 > first_stats);
    TEST_ASSERT(1000 > second_stats);
    vSemaphoreDelete(semaphore);
}

void test_mutex_inversion(void)
{
    semaphore = xSemaphoreCreateMutex();
    run_analyzer(priority_inversion, tskIDLE_PRIORITY+(3), 0, &first_stats,
                 priority_inversion, tskIDLE_PRIORITY+(4), 1, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT(120000 > first_stats);
    TEST_ASSERT(4000000 < second_stats);
    vSemaphoreDelete(semaphore);
}

void test_no_priority__no_yield(void)
{
    run_analyzer(busy_busy, tskIDLE_PRIORITY+(3), 0, &first_stats,
                 busy_busy, tskIDLE_PRIORITY+(3), 0, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT(2000000 < first_stats);
    TEST_ASSERT(2000000 < second_stats);
}

void test_no_priority__yield(void)
{
    run_analyzer(busy_busy, tskIDLE_PRIORITY+(3), 0, &first_stats,
                 busy_yield, tskIDLE_PRIORITY+(3), 0, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT(first_stats > second_stats);
    TEST_ASSERT(first_stats > 4400000);
    TEST_ASSERT(20000 > second_stats);
}

void test_priority__low_first(void)
{
    run_analyzer(busy_busy, tskIDLE_PRIORITY+(3), 0, &first_stats,
                 busy_busy, tskIDLE_PRIORITY+(4), 1, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT(1000 > first_stats);
    TEST_ASSERT(4500000 < second_stats);
}

void test_priority__high_first(void)
{
    run_analyzer(busy_busy, tskIDLE_PRIORITY+(4), 0, &first_stats,
                 busy_busy, tskIDLE_PRIORITY+(3), 1, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT(4500000 < first_stats);
    TEST_ASSERT(1000 > second_stats);
}

void main_thread (void *args)
{
    while (1) {
        UNITY_BEGIN();
        RUN_TEST(test_priority_inversion);
        RUN_TEST(test_mutex_inversion);
        RUN_TEST(test_no_priority__no_yield);
        RUN_TEST(test_no_priority__yield);
        RUN_TEST(test_priority__low_first);
        RUN_TEST(test_priority__high_first);
        UNITY_END();
        vTaskDelay(1000);
    }
}

int main(void)
{
    stdio_init_all();
    TaskHandle_t main_handle;
    xTaskCreate(main_thread, "MainThread",
                configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+10, &main_handle);
    vTaskStartScheduler();
    return 0;
}
