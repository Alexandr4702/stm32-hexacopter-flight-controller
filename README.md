# STM32 Hexacopter Flight Controller

Experimental STM32 and FreeRTOS firmware for a six-motor flight controller.

## Features

- Six-motor PWM output and mixing
- RC PWM input capture
- PID attitude stabilization
- IMU and barometer support
- GPS and UBX message processing
- Quaternion-based orientation estimation
- nRF24L01 radio communication
- FreeRTOS-based task scheduling

## Control model

The flight-control loop uses a conventional PID structure for the three attitude
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

FreeRTOS tasks separate radio handling, sensor acquisition, navigation parsing,
orientation estimation, and control processing.

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

The nRF24L01 is connected over SPI with a dedicated interrupt input. It provides
a low-rate command and telemetry link and is handled by a separate FreeRTOS task
to avoid blocking the control loop.

### Motors and ESCs

Six timer PWM outputs drive the electronic speed controllers. The motor mixer
combines collective throttle with roll, pitch, and yaw corrections to generate
an individual command for each motor.

## Runtime flow

1. Initialize the RTOS, sensor buses, UART receivers, radio, RC capture timers,
   and six ESC outputs.
2. Capture RC commands in timer interrupts and receive navigation packets in UART
   interrupts.
3. Run sensor and navigation tasks to decode measurements and publish them through
   FreeRTOS queues and task notifications.
4. Update the quaternion-based orientation estimate from inertial measurements.
5. Compare the requested and estimated attitude in the PID task.
6. Mix the three PID corrections with throttle and limit all six motor commands.
7. Update the ESC PWM outputs and transmit selected telemetry over UART or radio.
8. Apply the arming input before allowing non-idle motor output. A production
   deployment must add and validate loss-of-signal failsafe behavior.

## Project structure

- `Src/`, `Inc/` — application code, RTOS setup, and hardware initialization
- `PID/` — stabilization and motor mixing
- `ADIS/`, `gy89/` — inertial and environmental sensor drivers
- `GPS/`, `UBX/` — navigation protocols and parsers
- `NRF24L01/` — radio driver
- `quateradapt/` — orientation estimation
- `adapt_gps3/` — generated navigation algorithm
- `Middlewares/`, `Drivers/` — FreeRTOS, STM32 HAL, and CMSIS
- `copter_GPs.ioc` — STM32CubeMX configuration

## Building

### STM32CubeIDE

1. Install STM32CubeIDE with STM32F7 support.
2. Clone this repository.
3. In STM32CubeIDE, select **File → Import → Existing Projects into Workspace**.
4. Select the repository directory and import the project.
5. Build the `Debug` configuration.
6. Connect an ST-LINK programmer and flash the firmware.

### Command line

With the ARM GNU Toolchain and `make` available in `PATH`:

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
