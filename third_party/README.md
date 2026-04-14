# Dependencies

Run `python3 tools/bootstrap.py` once with network access. Releases and full
commit hashes are pinned in `lock.json`; the script rejects modified or
unexpected checkouts. Subsequent builds are offline. Dependencies are ignored
by the project repository; their original sources and licenses remain intact.

| Library | Release | License |
|---|---|---|
| ST STM32F4 HAL | v1.8.3 | BSD-3-Clause |
| ST CMSIS device F4 | v2.6.10 | Apache-2.0 |
| Arm CMSIS Core | 5.9.0 | Apache-2.0 |
| FreeRTOS Kernel | V11.1.0 | MIT |
| Arm CMSIS-DSP | v1.16.2 | Apache-2.0 |

See each checkout's LICENSE file for controlling terms. No DLO source code is
copied. These are deliberately fixed releases, not a claim to track latest.
Unmodified license texts for the pinned components are also retained in
licenses/ and included with CI firmware artifacts.
