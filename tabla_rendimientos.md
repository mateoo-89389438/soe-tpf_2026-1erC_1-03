| Actividad | Memoria pool | % de uso de la CPU | % de consumo de RAM | WCET [us] |
| :--- | :---: | :---: | :---: | :---: |
| 01: Device Driver - Transmisión (Polling & Gatekeeper Task) | No | < 1 | 37.58 | 16 |
| 02: Device Driver - Transmisión (Memory Pool & Polling & Gatekeeper Task) | Si | < 1 | 37.58 | 12 |
| 03: Device Driver - Transmisión (Interrupt & Gatekeeper Task) | No | < 1 | 37.62 | 34 |
| 04: Device Driver - Transmisión (Memory Pool & Interrupt & Gatekeeper Task) | Si | < 1 | 38.59 | 74 |
| 05: Device Driver - Transmisión (DMA & Gatekeeper Task) | No | < 1 | 38.09 | 34 |
| 06: Device Driver - Transmisión (Memory Pool & DMA & Gatekeeper Task) | Si | < 1 | 38.09 | 36 |
| 07: Device Driver - Recepción (Known Length & Polling & Gatekeeper Task) | No | - | - | - |
| 08: Device Driver - Recepción (Known Length & Memory Pool & Polling & Gatekeeper Task) | Si | - | - | - |
| 09: Device Driver - Recepción (Known Length & Interrupt & Gatekeeper Task) | No | - | - | - |
| 10: Device Driver - Recepción (Known Length & Memory Pool & Interrupt & Gatekeeper Task) | Si | - | - | - |
| 11: Device Driver - Recepción (Unknown Length & Memory Pool & Interrupt & Gatekeeper Task) | Si | - | - | - |
| 12: Device Driver - Recepción (Known Length & DMA & Gatekeeper Task) | No | - | - | - |
| 13: Device Driver - Recepción (Known Length & Memory Pool & DMA & Gatekeeper Task) | Si | - | - | - |
| 14: Device Driver - Recepción (Unknown Length & Memory Pool & DMA & Gatekeeper Task) | Si | - | - | - |
