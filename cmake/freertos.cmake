set(RTOS ${PROJECT_SOURCE_DIR}/third_party/freertos)
add_library(freertos STATIC ${RTOS}/tasks.c ${RTOS}/queue.c ${RTOS}/list.c
  ${RTOS}/portable/GCC/ARM_CM4F/port.c)
target_include_directories(freertos SYSTEM PUBLIC ${RTOS}/include ${RTOS}/portable/GCC/ARM_CM4F)
target_include_directories(freertos PUBLIC firmware/app)
target_link_libraries(freertos PUBLIC stm32_hal)
target_sources(oscilloscope PRIVATE firmware/app/tasks.c firmware/app/rtos_hooks.c)
target_link_libraries(oscilloscope PRIVATE freertos)
