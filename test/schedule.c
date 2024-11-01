#include <unity_config.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <unity.h>
#include "helper.c"

void setUp(void) {}

void tearDown(void) {}

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
        if (!(i & 0xFF)) {
            taskYIELD();
        }
    }
}

void busy_sleep(void *args)
{
    char *name = (char *)args;
    printf("start busy_busy %s\n", name);
    busy_wait_us(10000);
    sleep_ms(490);
}

SemaphoreHandle_t semaphore;
void priority_inversion(void *args)
{
    char *name = (char *)args;
    printf("start priority inversion %s\n", name);
    xSemaphoreTake(semaphore, portMAX_DELAY);
    printf("got semaphore %s\n", name);
    for (int i = 0; ; i++);
}

void test_priority_inversion(void)
{
    configRUN_TIME_COUNTER_TYPE high_stats, low_stats, elapsed_stats, elapsed_ticks;
    semaphore = xSemaphoreCreateBinary();

    run_analyzer(priority_inversion,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &low_stats,
                 /*PREEMPT*/tskIDLE_PRIORITY+(4), 1, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_LESS_THAN_INT64(2000, high_stats);
    TEST_ASSERT_GREATER_THAN_INT64(4500000, low_stats);
    vSemaphoreDelete(semaphore);
}

void test_coop__no_priority__no_yield(void)
{
    configRUN_TIME_COUNTER_TYPE first_stats, second_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_busy,
                 /*COOP*/tskIDLE_PRIORITY+(3), 0, &first_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 1, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_EQUAL_UINT64(0, second_stats);
    TEST_ASSERT_UINT64_WITHIN(400000, 5500000, first_stats);
}

void test_preempt__no_priority__no_yield(void)
{
    configRUN_TIME_COUNTER_TYPE first_stats, second_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_busy,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &first_stats,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 1, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_EQUAL_UINT64(0, second_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, first_stats);
}

void test_coop__no_priority__yield(void)
{
    configRUN_TIME_COUNTER_TYPE first_stats, second_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_yield,
                 /*COOP*/tskIDLE_PRIORITY+(3), 0, &first_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 0, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(100000, 2500000, first_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 2500000, second_stats);
}

void test_preempt__no_priority__yield(void)
{
    configRUN_TIME_COUNTER_TYPE first_stats, second_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_yield,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &first_stats,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &second_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(100000, 2500000, first_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 2500000, second_stats);
}

void test_coop__priority_low_first__no_yield(void)
{
    configRUN_TIME_COUNTER_TYPE high_stats, low_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_busy,
                 /*COOP*/tskIDLE_PRIORITY+(2), 0, &low_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 1, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_EQUAL_UINT64(0, high_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, low_stats);
}

void test_coop__priority_high_first__no_yield(void)
{
    configRUN_TIME_COUNTER_TYPE high_stats, low_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_busy,
                 /*COOP*/tskIDLE_PRIORITY+(2), 1, &low_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 0, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_EQUAL_UINT64(0, low_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, high_stats);
}

void test_coop__priority_low_first__yield(void)
{
    configRUN_TIME_COUNTER_TYPE low_stats, high_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_yield,
                 /*COOP*/tskIDLE_PRIORITY+(2), 0, &low_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 1, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(5000, 0, low_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, high_stats);
}

void test_coop__priority_high_first__yield(void)
{
    configRUN_TIME_COUNTER_TYPE low_stats, high_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_yield,
                 /*COOP*/tskIDLE_PRIORITY+(2), 0, &low_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 1, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(5000, 0, low_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, high_stats);
}

void test_preempt__priority__no_yield(void)
{
    configRUN_TIME_COUNTER_TYPE low_stats, high_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_busy,
                 /*PREEMPT*/tskIDLE_PRIORITY+(2), 0, &low_stats,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(5000, 0, low_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, high_stats);
}

void test_preempt__priority__yield(void)
{
    configRUN_TIME_COUNTER_TYPE low_stats, high_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_yield,
                 /*PREEMPT*/tskIDLE_PRIORITY+(2), 0, &low_stats,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(5000, 0, low_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, high_stats);
}


void test_mix__priority__yield(void)
{
    configRUN_TIME_COUNTER_TYPE low_stats, high_stats, elapsed_stats, elapsed_ticks;
    run_analyzer(busy_yield,
                 /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &low_stats,
                 /*COOP*/tskIDLE_PRIORITY+(3), 1, &high_stats,
                 &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(5000, 0, low_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, high_stats);
}


void test_preempt__priority__sleepy(void)
{
    configRUN_TIME_COUNTER_TYPE low_stats, high_stats, elapsed_stats, elapsed_ticks;
    run_analyzer_split(5000,
                       busy_busy,
                       /*PREEMPT*/tskIDLE_PRIORITY+(2), 0, &low_stats,
                       busy_sleep,
                       /*PREEMPT*/tskIDLE_PRIORITY+(3), 0, &high_stats,
                       &elapsed_stats, &elapsed_ticks);
    TEST_ASSERT_UINT64_WITHIN(1000, 10000, high_stats);
    TEST_ASSERT_UINT64_WITHIN(100000, 5000000, low_stats);
}

void main_thread (void *args)
{
    while (1) {
        UNITY_BEGIN();
        RUN_TEST(test_priority_inversion);
        /* RUN_TEST(test_coop__no_priority__no_yield); */
        /* RUN_TEST(test_preempt__no_priority__no_yield); */

        /* RUN_TEST(test_coop__no_priority__yield); */
        /* RUN_TEST(test_preempt__no_priority__yield); */

        /* RUN_TEST(test_coop__priority_low_first__no_yield); */
        /* RUN_TEST(test_coop__priority_high_first__no_yield); */

        /* RUN_TEST(test_coop__priority_low_first__yield); */
        /* RUN_TEST(test_coop__priority_high_first__yield); */

        /* RUN_TEST(test_preempt__priority__no_yield); */
        /* RUN_TEST(test_preempt__priority__yield); */
        /* RUN_TEST(test_preempt__priority__sleepy); */
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
