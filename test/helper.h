#pragma once

void run_analyzer_split(uint32_t test_duration,
                        TaskFunction_t pri_thread_entry,
                        int pri_priority,  uint32_t pri_delay,
                        configRUN_TIME_COUNTER_TYPE *pri_duration,

                        TaskFunction_t sec_thread_entry,
                        int sec_priority,  uint32_t sec_delay,
                        configRUN_TIME_COUNTER_TYPE *sec_duration,

                        configRUN_TIME_COUNTER_TYPE *total_duration,
                        configRUN_TIME_COUNTER_TYPE *total_ticks);

void run_analyzer(TaskFunction_t thread_entry,
                  int pri_priority,  uint32_t pri_delay,
                  configRUN_TIME_COUNTER_TYPE *pri_duration,
                  int sec_priority,  uint32_t sec_delay,
                  configRUN_TIME_COUNTER_TYPE *sec_duration,
                  configRUN_TIME_COUNTER_TYPE *total_duration,
                  configRUN_TIME_COUNTER_TYPE *total_ticks);
