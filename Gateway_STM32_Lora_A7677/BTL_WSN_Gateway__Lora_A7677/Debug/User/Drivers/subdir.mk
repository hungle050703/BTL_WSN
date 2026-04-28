################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Drivers/a7677.c \
../User/Drivers/sx1278.c 

OBJS += \
./User/Drivers/a7677.o \
./User/Drivers/sx1278.o 

C_DEPS += \
./User/Drivers/a7677.d \
./User/Drivers/sx1278.d 


# Each subdirectory must supply rules for building sources it contributes
User/Drivers/%.o User/Drivers/%.su User/Drivers/%.cyclo: ../User/Drivers/%.c User/Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I"/home/hung-le/STM32Cube/Code/BTL_WSN/Gateway_STM32_Lora_A7677/BTL_WSN_Gateway__Lora_A7677/User/Drivers" -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Drivers

clean-User-2f-Drivers:
	-$(RM) ./User/Drivers/a7677.cyclo ./User/Drivers/a7677.d ./User/Drivers/a7677.o ./User/Drivers/a7677.su ./User/Drivers/sx1278.cyclo ./User/Drivers/sx1278.d ./User/Drivers/sx1278.o ./User/Drivers/sx1278.su

.PHONY: clean-User-2f-Drivers

