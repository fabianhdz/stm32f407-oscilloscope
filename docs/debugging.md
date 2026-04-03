# Flashing and debugging
Build with `cmake --preset debug && cmake --build --preset debug`.
Install OpenOCD and an ARM-capable GDB (e.g. arm-none-eabi-gdb).
Connect ST-Link SWDIO to PA13, SWCLK to PA14, reference voltage to 3.3 V,
and GND to GND. Supply board power separately; follow your probe's pin labels.
BOOT0 must select flash. SWD pins are never used as logic channels.
Run `python3 tools/flash.py` from the repository root to program and verify.
VS Code's Cortex-Debug configuration launches the same ELF with OpenOCD.
For an onboard ST-Link use its USB debug connector and board jumpers.
The generic configuration does not require NRST. Connect NRST and change the
reset configuration if under-reset recovery is needed.
Inspect `scope_fault_reason` after faults. HAL timing uses TIM5; RTOS timing
will use SysTick. HSI frequency error affects both sampling and UART timing.
