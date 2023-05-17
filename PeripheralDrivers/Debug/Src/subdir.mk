################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/BasicTimer.c \
../Src/ExtiDriver.c \
../Src/GPIOxDriver.c \
../Src/I2CDriver.c \
../Src/PwmDriver.c \
../Src/SysTickDriver.c \
../Src/USARTxDriver.c 

OBJS += \
./Src/BasicTimer.o \
./Src/ExtiDriver.o \
./Src/GPIOxDriver.o \
./Src/I2CDriver.o \
./Src/PwmDriver.o \
./Src/SysTickDriver.o \
./Src/USARTxDriver.o 

C_DEPS += \
./Src/BasicTimer.d \
./Src/ExtiDriver.d \
./Src/GPIOxDriver.d \
./Src/I2CDriver.d \
./Src/PwmDriver.d \
./Src/SysTickDriver.d \
./Src/USARTxDriver.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/PeripheralDrivers/Inc" -I/home/santiago/Documentos/ProyectoLibreria/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Core/Include -I/home/santiago/Documentos/ProyectoLibreria/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/BasicTimer.d ./Src/BasicTimer.o ./Src/BasicTimer.su ./Src/ExtiDriver.d ./Src/ExtiDriver.o ./Src/ExtiDriver.su ./Src/GPIOxDriver.d ./Src/GPIOxDriver.o ./Src/GPIOxDriver.su ./Src/I2CDriver.d ./Src/I2CDriver.o ./Src/I2CDriver.su ./Src/PwmDriver.d ./Src/PwmDriver.o ./Src/PwmDriver.su ./Src/SysTickDriver.d ./Src/SysTickDriver.o ./Src/SysTickDriver.su ./Src/USARTxDriver.d ./Src/USARTxDriver.o ./Src/USARTxDriver.su

.PHONY: clean-Src

