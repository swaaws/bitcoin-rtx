# bitcoin-rtx

Offline Bitcoin transaction relay using ESP32 mesh networking.

## Concept

Submit Bitcoin transactions via a captive portal WiFi network (`bitcoin.local`), then relay them between ESP32 nodes using **ESP-NOW** — no internet required.

The idea: create a local mesh network of ESP32 devices that can:
1. Accept raw Bitcoin transactions via a web form (captive portal)
2. Broadcast transactions between nodes via ESP-NOW (peer-to-peer, no WiFi infrastructure)
3. Eventually relay to the Bitcoin network when a node gets internet access

## How it works

- ESP32 opens a WiFi AP named `bitcoin.local`
- Captive portal serves a simple TX submission form
- Submitted transactions are sent via ESP-NOW broadcast to all nearby nodes
- Uses FreeRTOS tasks for concurrent operation
- Transactions stored on SPIFFS for persistence

## Hardware

- ESP32 (any variant)
- Optional: M5StickC for display + button interaction

## Status

🔬 Proof of concept / experimental

## Build

Arduino IDE or PlatformIO with ESP32 board support.
