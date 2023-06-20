################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/main.c 

OBJS += \
./App/Src/main.o 

C_DEPS += \
./App/Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/pruebaJoystick/App/Inc" -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/PeripheralDrivers/Inc" -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/pruebaJoystick/Core/Include" -I"/home/santiago/Documentos/GitHub/TallerV_01_2023/pruebaJoystick/Core/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/main.d ./App/Src/main.o ./App/Src/main.su

.PHONY: clean-App-2f-Src

