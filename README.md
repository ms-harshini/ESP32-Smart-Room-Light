# ESP32 Smart Room Light

An ESP32 smart-light project that measures ambient light with a photoresistor and controls an LED automatically or from a Wi-Fi webpage. The circuit and firmware are designed to run in Wokwi with PlatformIO.

## Features

- Reads the photoresistor analog output on GPIO 34.
- **AUTO mode:** turns the LED on when the room is dark and off when it is bright.
- **MANUAL mode:** provides webpage buttons to turn the LED on or off, regardless of the light reading.
- Displays the light reading, room condition, selected mode, and light state on the webpage.
- Prints sensor and status information to the serial monitor.

## Components

- ESP32 DevKit v1
- Photoresistor (LDR) module with analog output
- LED
- 220 Ω resistor for the LED
- Wokwi ESP32, photoresistor, and LED simulator parts

The Wokwi photoresistor module includes the LDR voltage-divider circuit. For a discrete LDR circuit, use a 10 kΩ resistor as a voltage divider.

## Connections

| Component pin | ESP32 pin |
|---|---|
| Photoresistor `VCC` | `3V3` |
| Photoresistor `GND` | `GND` |
| Photoresistor `AO` | GPIO 34 |
| LED anode (`A`) through 220 Ω resistor | GPIO 18 |
| LED cathode (`C`) | `GND` |

## How it works

The ESP32 samples the LDR and compares the reading with `DARK_THRESHOLD` in `src/main.cpp` (currently `1500`). Readings below the threshold are treated as dark.

- In **AUTO** mode, dark turns the LED on; bright turns it off.
- In **MANUAL** mode, the webpage buttons control the LED and the LDR does not change its state.

Adjust `DARK_THRESHOLD` if your sensor's dark and bright readings do not fall on opposite sides of 1500.

## Run in Wokwi

1. Open the project folder in VS Code with PlatformIO and the Wokwi extension installed.
2. Build the project with PlatformIO.
3. Open `diagram.json` and start the Wokwi simulator.
4. Wait for the serial output to report that Wi-Fi connected and the web server started.
5. Open [http://localhost:8180](http://localhost:8180) in a browser.
6. Use the photoresistor's `lux` control to test AUTO mode. Try a low value such as `10` for dark and a higher value such as `500` for bright.
7. Select **MANUAL** on the webpage to test the **TURN ON** and **TURN OFF** buttons. Select **AUTO** again to return control to the LDR.

The `wokwi.toml` file forwards local port 8180 to port 80 on the simulated ESP32. The simulated ESP32 connects to Wokwi's `Wokwi-GUEST` network.

## Project files

- `src/main.cpp` — ESP32 sensor logic, Wi-Fi connection, web server, and webpage
- `platformio.ini` — PlatformIO board and framework settings
- `diagram.json` — Wokwi circuit and component connections
- `wokwi.toml` — Wokwi firmware paths and local web-server port forwarding

## Using a physical ESP32

The `Wokwi-GUEST` Wi-Fi network is for simulation. To use real hardware, replace the Wi-Fi SSID and password in `src/main.cpp` with your network details, connect to the ESP32's printed IP address from a device on the same network, and verify the sensor threshold using actual LDR readings. Use a low-voltage LED for the demonstration circuit.
