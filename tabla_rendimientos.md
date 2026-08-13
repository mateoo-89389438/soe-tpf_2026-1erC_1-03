| Actividad | Memory pool | % de uso de la CPU | % de consumo de RAM | WCET [µs] |
| :--- | :---: | :---: | :---: | :---: |
| 01: Device Driver - Transmisión (Polling & Gatekeeper Task) | No | < 1 | 37.70 | 16 |
| 02: Device Driver - Transmisión (Memory Pool & Polling & Gatekeeper Task) | Si | < 1 | 40.35 | 13 |
| 03: Device Driver - Transmisión (Interrupt & Gatekeeper Task) | No | < 1 | 37.73 | 38 |
| 04: Device Driver - Transmisión (Memory Pool & Interrupt & Gatekeeper Task) | Si | < 1 | 39.69 | 31 |
| 05: Device Driver - Transmisión (DMA & Gatekeeper Task) | No | < 1 | 38.05 | 36 |
| 06: Device Driver - Transmisión (Memory Pool & DMA & Gatekeeper Task) | Si | < 1 | 40.35 | 36 |
| 07: Device Driver - Recepción (Known Length & Polling & Gatekeeper Task) | No | < 1 | 38.09 | 32 |
| 08: Device Driver - Recepción (Known Length & Memory Pool & Polling & Gatekeeper Task) | Si | < 1 | 38.96 | 32 |
| 09: Device Driver - Recepción (Known Length & Interrupt & Gatekeeper Task) | No | < 1 | 38.40 | 48 |
| 10: Device Driver - Recepción (Known Length & Memory Pool & Interrupt & Gatekeeper Task) | Si | < 1 | 40.00 | 56 |
| 11: Device Driver - Recepción (Unknown Length & Memory Pool & Interrupt & Gatekeeper Task) | Si | < 1 | 40.00 | 10 |
| 12: Device Driver - Recepción (Known Length & DMA & Gatekeeper Task) | No | < 1 | 38.40 | 45 |
| 13: Device Driver - Recepción (Known Length & Memory Pool & DMA & Gatekeeper Task) | Si | < 1 | 40.31 | 41 |
| 14: Device Driver - Recepción (Unknown Length & Memory Pool & DMA & Gatekeeper Task) | Si | < 1 | 40.31 | 106 |

_*Todas las mediciones de WCET se realizaron sin código de debug `LOGGER_INFO()`_
