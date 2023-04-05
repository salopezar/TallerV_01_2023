################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Src/BasicTimer.c \
../Drivers/Src/GPIOxDriver.c 

OBJS += \
./Drivers/Src/BasicTimer.o \
./Drivers/Src/GPIOxDriver.o 

C_DEPS += \
./Drivers/Src/BasicTimer.d \
./Drivers/Src/GPIOxDriver.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Src/%.o Drivers/Src/%.su: ../Drivers/Src/%.c Drivers/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I../Inc -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/mcu_headers/CMSIS/Include" -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/mcu_headers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/Clase_Interrupciones/Drivers/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Src

clean-Drivers-2f-Src:
	-$(RM) ./Drivers/Src/BasicTimer.d ./Drivers/Src/BasicTimer.o ./Drivers/Src/BasicTimer.su ./Drivers/Src/GPIOxDriver.d ./Drivers/Src/GPIOxDriver.o ./Drivers/Src/GPIOxDriver.su

.PHONY: clean-Drivers-2f-Src

