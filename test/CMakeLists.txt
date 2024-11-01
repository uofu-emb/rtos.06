add_executable(test_schedule schedule.c)

target_link_libraries(test_schedule PRIVATE
  pico_stdlib
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
  unity
)
pico_enable_stdio_uart(test_schedule 0)
pico_enable_stdio_usb(test_schedule 1)
add_custom_target(flash_test picotool load -f $<TARGET_FILE:test_schedule> DEPENDS test_schedule)
