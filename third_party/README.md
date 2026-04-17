# Dependencies

`python3 tools/bootstrap.py` fetches the versions pinned in `lock.json` and
checks that existing checkouts match. Builds then use the local sources.

| Library | Version | License |
|---|---|---|
| STM32F4 HAL | v1.8.3 | BSD-3-Clause |
| CMSIS device F4 | v2.6.10 | Apache-2.0 |
| CMSIS Core | 5.9.0 | Apache-2.0 |
| FreeRTOS Kernel | V11.1.0 | MIT |
| CMSIS-DSP | v1.16.2 | Apache-2.0 |

Upstream license texts are retained in `licenses/` and included with CI
firmware artifacts.
