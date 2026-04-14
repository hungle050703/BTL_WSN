################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Services/sensor_service.c 

OBJS += \
./User/Services/sensor_service.o 

C_DEPS += \
./User/Services/sensor_service.d 


# Each subdirectory must supply rules for building sources it contributes
User/Services/%.o User/Services/%.su User/Services/%.cyclo: ../User/Services/%.c User/Services/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"/home/hung-le/STM32Cube/Code/BTL _Embedded_System_Design/User/App" -I"/home/hung-le/STM32Cube/Code/BTL _Embedded_System_Design/User/Drivers" -I"/home/hung-le/STM32Cube/Code/BTL _Embedded_System_Design/User/Services" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Services

clean-User-2f-Services:
	-$(RM) ./User/Services/sensor_service.cyclo ./User/Services/sensor_service.d ./User/Services/sensor_service.o ./User/Services/sensor_service.su

.PHONY: clean-User-2f-Services

