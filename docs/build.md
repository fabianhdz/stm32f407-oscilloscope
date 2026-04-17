# Build and debug

Requires CMake 3.25+, Ninja, Git, Python 3, a native C compiler, and ARM GCC
with Newlib. OpenOCD and an ARM-capable GDB provide ST-Link debugging.

## Firmware

```sh
python3 tools/bootstrap.py
cmake --preset debug
cmake --build --preset debug
```

The ELF, BIN, HEX, and linker map are written to `build/debug/`.
The `release` preset produces optimized firmware in `build/release/`.
Dependencies are fetched once and pinned in `third_party/lock.json`.

With the board powered and ST-Link connected:

```sh
openocd -f tools/openocd.cfg -c "program build/debug/oscilloscope.elf verify reset exit"
```

The VS Code launch configuration uses the same OpenOCD setup. A fault leaves
its reason in `scope_fault_reason`. UART diagnostics use PA2 at 115200 baud,
8N1, and include dropped-block, overrun, timing, and stack counters.

## Host tests

```sh
cmake --preset host
cmake --build --preset host
ctest --preset host
```

Use `host-sanitize` instead of `host` for AddressSanitizer and
UndefinedBehaviorSanitizer. LeakSanitizer needs a terminal without process
tracing. CI runs the sanitizer tests and builds both firmware configurations.

## Controls and loopback

Select cycles through time scale, voltage scale, trigger level, edge, mode,
and view. +/− edit the selected field. Run/Stop holds or resumes; Run rearms
Single mode after a capture.

`cmake --preset debug -DSCOPE_DEMO_DAC=ON` enables the nominal 1 kHz source
on PA4. After rebuilding and flashing, PA4 can feed the analog front end.
Set the option back to `OFF` to disable it. Start with 1 ms/div and 500 mV/div.
Wiring and calibration details are in [hardware/wiring.md](../hardware/wiring.md).
