set(DSP_ROOT ${PROJECT_SOURCE_DIR}/third_party/cmsis_dsp)
if(NOT EXISTS "${DSP_ROOT}/Include/arm_math.h")
  message(FATAL_ERROR "Run python3 tools/bootstrap.py before configuring")
endif()
add_library(cmsis_dsp STATIC
  ${DSP_ROOT}/Source/TransformFunctions/arm_rfft_fast_f32.c
  ${DSP_ROOT}/Source/TransformFunctions/arm_rfft_fast_init_f32.c
  ${DSP_ROOT}/Source/TransformFunctions/arm_cfft_f32.c
  ${DSP_ROOT}/Source/TransformFunctions/arm_cfft_init_f32.c
  ${DSP_ROOT}/Source/TransformFunctions/arm_cfft_radix8_f32.c
  ${DSP_ROOT}/Source/TransformFunctions/arm_bitreversal2.c
  ${DSP_ROOT}/Source/CommonTables/arm_common_tables.c
  ${DSP_ROOT}/Source/CommonTables/arm_const_structs.c)
target_include_directories(cmsis_dsp SYSTEM PUBLIC ${DSP_ROOT}/Include
  ${PROJECT_SOURCE_DIR}/third_party/cmsis_core/CMSIS/Core/Include)
target_include_directories(cmsis_dsp PRIVATE ${DSP_ROOT}/PrivateInclude)
target_compile_options(cmsis_dsp PRIVATE -O2 -ffunction-sections -fdata-sections)
if(NOT SCOPE_FIRMWARE)
  # CMSIS-DSP's scalar desktop build path; no replacement FFT implementation.
  target_compile_definitions(cmsis_dsp PUBLIC __GNUC_PYTHON__)
endif()
target_link_libraries(scope_core PUBLIC cmsis_dsp)
