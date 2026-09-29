# STM32 Hexacopter Flight Controller

Experimental STM32 and FreeRTOS firmware for a six-motor flight controller.

## Features

- Six-motor PWM output and mixing
- RC PWM input capture
- PID attitude-control and six-motor mixer implementation (runtime disabled)
- IMU and barometer support
- GPS and UBX message processing
- Madgwick quaternion-based orientation estimation
- nRF24L01 radio communication
- FreeRTOS-based task scheduling

## Control model

The `PID/` module contains a conventional PID structure for the three attitude
axes:

1. RC PWM channels are converted into roll, pitch, yaw, and throttle commands.
2. The requested attitude is compared with the orientation estimate produced
   from the inertial sensors and quaternion-based filter.
3. Proportional, integral, and derivative terms are calculated at the nominal
   controller period defined in `PID/PID.c`.
4. The resulting axis corrections are combined with throttle by a six-motor
   mixer.
5. Motor commands are limited to the configured ESC range, and the arming input
   can force all motors back to idle output.

The three axes use independent gain arrays. Integral and derivative limiting are
present for selected axes to reduce windup and output spikes. The gains are
fixed in the firmware; there is no automatic tuning or airframe identification,
so they must be validated for the actual frame, motors, propellers, battery, and
sensor mounting.

The PID implementation is not connected to the active runtime. `PID_thread`
keeps all six outputs at the fixed idle value and does not call `pid_update()`, and
`StartDefaultTask` terminates both `PID_thread` and the nRF24 task at startup
before terminating itself. This preserves the repository's safe experimental
state: the estimator and parser code can run, but attitude stabilization and
radio handling must not be presented as operational flight-control features.

## Sensors and connected devices

### ADIS16488 inertial sensor

The ADIS module is connected over SPI and provides angular-rate, acceleration,
and magnetic-field measurements. It is intended to be the primary high-rate
inertial source for attitude estimation and stabilization.

### GY-89 sensor board

The GY-89 board is connected over I2C and combines several sensors:

- **L3GD20** — three-axis gyroscope for angular-rate measurements.
- **LSM303D** — three-axis accelerometer and magnetometer for gravity direction,
  motion, and heading observations.
- **BMP180** — barometric pressure sensor used to derive relative altitude.

The GY-89 data can complement the ADIS measurements and provide independent
signals for diagnostics and estimator development.

### Navigation receivers

Two UART paths are available for navigation data. The firmware contains a UBX
binary-protocol parser and a separate N8IS message path. Parsed position,
velocity, fix status, and related navigation values are passed to the navigation
tasks through FreeRTOS queues.

### RC receiver

Six timer input-capture channels measure RC PWM pulse widths. The captured values
provide attitude, throttle, arming, and auxiliary commands for the controller.

### nRF24L01 radio

The nRF24L01 is connected over SPI with a dedicated interrupt input. Driver and
task code are present, but the default task terminates the radio task at startup.

### Motors and ESCs

Six timer PWM outputs drive the electronic speed controllers. The motor mixer
can combine collective throttle with roll, pitch, and yaw corrections to
generate an individual command for each motor. It is not called by the current
runtime. PWM peripherals start with idle pulse widths around 1010 microseconds;
the disabled PID thread uses 1000 microseconds.

## Runtime flow

1. Initialize the RTOS, sensor buses, UART receivers, radio, RC capture timers,
   and six ESC outputs.
2. Capture RC commands in timer interrupts and receive navigation packets in UART
   interrupts.
3. Run sensor and navigation tasks to decode measurements and publish them through
   FreeRTOS queues and task notifications.
4. Update the Madgwick orientation filter from accelerometer and gyroscope
   measurements and transmit selected telemetry over UART. Roll and pitch are
   corrected by gravity; yaw is gyro-integrated because the magnetometer is not
   yet connected to the filter.
5. Keep the PID and nRF24 tasks disabled through `StartDefaultTask`.

The PID/mixer code includes an arming-input check, but because the control call
is disabled this path is not exercised by the runtime. A production deployment
would need integration and validation of attitude feedback, motor mixing,
arming, loss-of-signal handling, and emergency shutdown.

## Suggested reading order

The following links point to project-specific integration, protocol, and driver
code. STM32-generated files, FreeRTOS, CMSIS, HAL, and the Madgwick submodule are
identified separately and are not presented as original code in this repository.

1. [UBX streaming parser and message handlers](UBX/UBX.c)
2. [N8IS streaming parser](GPS/GPS.c)
3. [ADIS16488 SPI driver](ADIS/ADIS.c)
4. [Madgwick C interface and Euler-angle conversion](Orientation/madgwick_adapter.cpp)
5. [Application and FreeRTOS integration in the CubeMX entry point](Src/main.c)

The [PID calculations and six-motor mixer](PID/PID.c) are retained as legacy
experimental work; as noted above, they are not enabled by the runtime.

## Project structure

- `Src/`, `Inc/` — application code, RTOS setup, and hardware initialization
- `PID/` — stabilization and motor mixing
- `ADIS/`, `gy89/` — inertial and environmental sensor drivers
- `GPS/`, `UBX/` — navigation protocols and parsers
- `NRF24L01/` — radio driver
- `Orientation/` — C interface used by the FreeRTOS navigation task
- `madgwick-orientation-filter/` — C++/Eigen Git submodule containing the active
  orientation filter (the firmware build uses its C++17-compatible API)
- `adapt_gps3/` — retained generated legacy navigation algorithm, not used by
  the active runtime
- `Middlewares/`, `Drivers/` — FreeRTOS, STM32 HAL, and CMSIS
- `copter_GPs.ioc` — STM32CubeMX configuration

## Building

### STM32CubeIDE

1. Install STM32CubeIDE with STM32F7 support.
2. Clone this repository with submodules:

   ```sh
   git clone --recurse-submodules https://github.com/Alexandr4702/stm32-hexacopter-flight-controller.git
   ```

   For an existing checkout, run `git submodule update --init --recursive`.
3. In STM32CubeIDE, select **File → Import → Existing Projects into Workspace**.
4. Select the repository directory and import the project.
5. Build the `Debug` configuration.
6. Connect an ST-LINK programmer and flash the firmware.

The firmware configuration compiles the Madgwick sources as C++17 and requires
the ARM GNU C++ compiler included with STM32CubeIDE.

### Command line

The `Debug/` directory is generated by STM32CubeIDE and is not tracked. After
CubeIDE has generated it, the project can be rebuilt with the ARM GNU Toolchain
and `make` available in `PATH`:

```sh
make -C Debug all
```

## Code formatting

The repository contains a `.clang-format` configuration. Run `clang-format`
on application source files before committing changes. Vendor and generated code
should not be reformatted.

## Safety

Remove all propellers before testing. Verify motor order and direction, output
limits, radio failsafe behavior, and emergency shutdown before flight.

The current firmware is experimental and has not been qualified for flight or
safety-critical operation.

## License

No project-wide license has been specified. Third-party components retain their
respective licenses.
