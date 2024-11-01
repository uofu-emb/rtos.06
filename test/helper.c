#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "helper.h"

void run_analyzer_split(uint32_t test_duration,
                        TaskFunction_t pri_thread_entry,
                        int pri_priority,  uint32_t pri_delay,
                        configRUN_TIME_COUNTER_TYPE *pri_duration,

                        TaskFunction_t sec_thread_entry,
                        int sec_priority,  uint32_t sec_delay,
                        configRUN_TIME_COUNTER_TYPE *sec_duration,

                        configRUN_TIME_COUNTER_TYPE *total_duration,
                        configRUN_TIME_COUNTER_TYPE *total_ticks)
{
    TaskHandle_t primary_thread, secondary_thread;
    char *primary_name = "primary";
    char *secondary_name = "secondary";
    TickType_t start_ticks = xTaskGetTickCount();
    configRUN_TIME_COUNTER_TYPE start_count = portGET_RUN_TIME_COUNTER_VALUE();
    // These should have lower priority than this thread

    vTaskDelay(pdMS_TO_TICKS(pri_delay));
    printf("create primary\n");
    xTaskCreate(pri_thread_entry,
                "primary",
                configMINIMAL_STACK_SIZE,
                primary_name,
                pri_priority,
                &primary_thread);
    vTaskDelay(pdMS_TO_TICKS(sec_delay));
    printf("create secondary\n");
    xTaskCreate(sec_thread_entry,
                "secondary",
                configMINIMAL_STACK_SIZE,
                secondary_name,
                sec_priority,
                &secondary_thread);
    vTaskDelay(pdMS_TO_TICKS(test_duration));
    printf("super took over\n");

    TickType_t end_ticks = xTaskGetTickCount();
    configRUN_TIME_COUNTER_TYPE end_count = portGET_RUN_TIME_COUNTER_VALUE();
    configRUN_TIME_COUNTER_TYPE primary = ulTaskGetRunTimeCounter(primary_thread);
    configRUN_TIME_COUNTER_TYPE secondary = ulTaskGetRunTimeCounter(secondary_thread);
    configRUN_TIME_COUNTER_TYPE elapsed = end_count - start_count;
    TickType_t elapsed_ticks = end_ticks > start_ticks;

    printf("primary %lld secondary %lld start %lld end %lld elapsed %lld (us) in %d to %d ticks\n",
           primary, secondary, start_count, end_count, elapsed, start_ticks, end_ticks);

    *pri_duration = primary;
    *sec_duration = secondary;
    *total_duration = elapsed;
    *total_ticks = elapsed_ticks;

    vTaskDelete(primary_thread);
    vTaskDelete(secondary_thread);
}

void run_analyzer(TaskFunction_t thread_entry,
                  int pri_priority,  uint32_t pri_delay,
                  configRUN_TIME_COUNTER_TYPE *pri_duration,
                  int sec_priority,  uint32_t sec_delay,
                  configRUN_TIME_COUNTER_TYPE *sec_duration,
                  configRUN_TIME_COUNTER_TYPE *total_duration,
                  configRUN_TIME_COUNTER_TYPE *total_ticks)
{
    run_analyzer_split(5000,
                       thread_entry,
                       pri_priority, pri_delay, pri_duration,
                       thread_entry,
                       sec_priority, sec_delay, sec_duration,
                       total_duration,
                       total_ticks);

}
