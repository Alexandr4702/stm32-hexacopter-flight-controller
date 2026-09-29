# STM32 Hexacopter Flight Controller

Experimental STM32F746IG firmware for a six-motor controller. It reads the
GY-89 and ADIS16488 inertial sensors, parses GPS/UBX data, runs a Madgwick
orientation filter, captures six RC PWM channels, and generates six ESC PWM
outputs under FreeRTOS.

Attitude PID control and motor mixing are present but disabled at runtime. The
ESC outputs remain at their idle values.

## Requirements

- STM32CubeIDE with STM32F7 support and the ARM GNU C/C++ Toolchain
- Git with submodule support
- ST-LINK programmer/debugger
- STM32F746IG target board
- GY-89 sensor board for the active orientation filter
- Optional: ADIS16488 IMU, UBX-compatible GPS receiver, RC receiver, nRF24L01,
  and six ESCs, as wired by `copter_GPs.ioc`

The project uses STM32 HAL/CMSIS, FreeRTOS, and the Madgwick filter submodule.
The CubeMX configuration is `copter_GPs.ioc`.

## Build

Clone the repository and all nested dependencies:

```sh
git clone --recurse-submodules https://github.com/Alexandr4702/stm32-hexacopter-flight-controller.git
```

For an existing checkout:

```sh
git submodule update --init --recursive
```

Then:

1. Open STM32CubeIDE and select **File → Import → Existing Projects into
   Workspace**.
2. Select the repository directory.
3. Build the `Debug` configuration.

After STM32CubeIDE has generated `Debug/`, the same configuration can be rebuilt
from a terminal with:

```sh
make -C Debug all
```

## Flash and run

1. Connect the configured IMU and ST-LINK.
2. Flash the `Debug` image from STM32CubeIDE and reset the board.
3. Use USART1 at 576000 baud, 8-N-1, to read orientation and navigation
   telemetry.
4. If GPS is used, connect it to USART2 at 9600 baud, 8-N-1.

The Madgwick filter uses GY-89 accelerometer and gyroscope data at a nominal
100 Hz. Roll and pitch are gravity-corrected; yaw is gyro-integrated and will
drift because the magnetometer is not connected to the filter.
