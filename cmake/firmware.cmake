set(DEPS ${PROJECT_SOURCE_DIR}/third_party)
foreach(dep stm32_hal cmsis_device cmsis_core freertos cmsis_dsp)
  if(NOT EXISTS "${DEPS}/${dep}/.git")
    message(FATAL_ERROR "Missing ${dep}; run python3 tools/bootstrap.py")
  endif()
endforeach()
add_library(stm32_hal STATIC
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_cortex.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_rcc.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_rcc_ex.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_gpio.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_flash.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_flash_ex.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_pwr.c
  ${DEPS}/stm32_hal/Src/stm32f4xx_hal_pwr_ex.c)
target_include_directories(stm32_hal SYSTEM PUBLIC
  ${DEPS}/stm32_hal/Inc ${DEPS}/cmsis_device/Include ${DEPS}/cmsis_core/CMSIS/Core/Include)
target_include_directories(stm32_hal PUBLIC firmware/board)
target_compile_definitions(stm32_hal PUBLIC STM32F407xx USE_HAL_DRIVER)
add_executable(oscilloscope
  firmware/app/main.c
  ${DEPS}/cmsis_device/Source/Templates/system_stm32f4xx.c
  ${DEPS}/cmsis_device/Source/Templates/gcc/startup_stm32f407xx.s)
target_link_libraries(oscilloscope PRIVATE scope_core stm32_hal m)
target_compile_options(oscilloscope PRIVATE -Wall -Wextra -Werror -Wshadow)
target_link_options(oscilloscope PRIVATE
  -T${PROJECT_SOURCE_DIR}/firmware/board/stm32f407.ld
  --specs=nano.specs --specs=nosys.specs -nostartfiles
  -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/oscilloscope.map
  -Wl,--print-memory-usage)
set_target_properties(oscilloscope PROPERTIES SUFFIX ".elf" LINK_DEPENDS ${PROJECT_SOURCE_DIR}/firmware/board/stm32f407.ld)
add_custom_command(TARGET oscilloscope POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:oscilloscope> ${CMAKE_CURRENT_BINARY_DIR}/oscilloscope.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:oscilloscope> ${CMAKE_CURRENT_BINARY_DIR}/oscilloscope.hex
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:oscilloscope>)
