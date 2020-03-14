################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
../Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
../Middlewares/Third_Party/FreeRTOS/Source/list.c \
../Middlewares/Third_Party/FreeRTOS/Source/queue.c \
../Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
../Middlewares/Third_Party/FreeRTOS/Source/timers.c 

OBJS += \
./Middlewares/Third_Party/FreeRTOS/Source/croutine.o \
./Middlewares/Third_Party/FreeRTOS/Source/event_groups.o \
./Middlewares/Third_Party/FreeRTOS/Source/list.o \
./Middlewares/Third_Party/FreeRTOS/Source/queue.o \
./Middlewares/Third_Party/FreeRTOS/Source/tasks.o \
./Middlewares/Third_Party/FreeRTOS/Source/timers.o 

C_DEPS += \
./Middlewares/Third_Party/FreeRTOS/Source/croutine.d \
./Middlewares/Third_Party/FreeRTOS/Source/event_groups.d \
./Middlewares/Third_Party/FreeRTOS/Source/list.d \
./Middlewares/Third_Party/FreeRTOS/Source/queue.d \
./Middlewares/Third_Party/FreeRTOS/Source/tasks.d \
./Middlewares/Third_Party/FreeRTOS/Source/timers.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/FreeRTOS/Source/%.o: ../Middlewares/Third_Party/FreeRTOS/Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F746xx -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Inc" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/GPS" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/adapt_gps3" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/PID" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/ADIS" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/quateradapt" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/UBX" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/gy89" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/NRF24L01" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Drivers/STM32F7xx_HAL_Driver/Inc" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Drivers/CMSIS/Include" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/run/media/gilg/linData/avionika/workspace/stm32f746igt6/copter_GPs/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


