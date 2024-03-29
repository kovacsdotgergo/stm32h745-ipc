################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app.c \
../Core/Src/csv_formatter.c \
../Core/Src/freertos.c \
../Core/Src/ipc_mb.c \
../Core/Src/main.c \
../Core/Src/meas_control.c \
../Core/Src/meas_task.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_hal_timebase_tim.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/uart_commands.c 

OBJS += \
./Core/Src/app.o \
./Core/Src/csv_formatter.o \
./Core/Src/freertos.o \
./Core/Src/ipc_mb.o \
./Core/Src/main.o \
./Core/Src/meas_control.o \
./Core/Src/meas_task.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_hal_timebase_tim.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/uart_commands.o 

C_DEPS += \
./Core/Src/app.d \
./Core/Src/csv_formatter.d \
./Core/Src/freertos.d \
./Core/Src/ipc_mb.d \
./Core/Src/main.d \
./Core/Src/meas_control.d \
./Core/Src/meas_task.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_hal_timebase_tim.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/uart_commands.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -c -I../Core/Inc -I../../Common/Inc -I../../Common/Src -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Ofast -ffunction-sections -fdata-sections -Wall -Wextra -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app.d ./Core/Src/app.o ./Core/Src/app.su ./Core/Src/csv_formatter.d ./Core/Src/csv_formatter.o ./Core/Src/csv_formatter.su ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/ipc_mb.d ./Core/Src/ipc_mb.o ./Core/Src/ipc_mb.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/meas_control.d ./Core/Src/meas_control.o ./Core/Src/meas_control.su ./Core/Src/meas_task.d ./Core/Src/meas_task.o ./Core/Src/meas_task.su ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_hal_timebase_tim.d ./Core/Src/stm32h7xx_hal_timebase_tim.o ./Core/Src/stm32h7xx_hal_timebase_tim.su ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/uart_commands.d ./Core/Src/uart_commands.o ./Core/Src/uart_commands.su

.PHONY: clean-Core-2f-Src

