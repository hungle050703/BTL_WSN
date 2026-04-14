################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/App/alarm_logic.c 

OBJS += \
./User/App/alarm_logic.o 

C_DEPS += \
./User/App/alarm_logic.d 


# Each subdirectory must supply rules for building sources it contributes
User/App/%.o User/App/%.su User/App/%.cyclo: ../User/App/%.c User/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"/home/hung-le/STM32Cube/Code/BTL _Embedded_System_Design/User/App" -I"/home/hung-le/STM32Cube/Code/BTL _Embedded_System_Design/User/Services" -I"/home/hung-le/STM32Cube/Code/BTL _Embedded_System_Design/User/Drivers" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-App

clean-User-2f-App:
	-$(RM) ./User/App/alarm_logic.cyclo ./User/App/alarm_logic.d ./User/App/alarm_logic.o ./User/App/alarm_logic.su

.PHONY: clean-User-2f-App

