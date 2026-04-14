################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../EWARM/startup_stm32h563xx.s 

S_DEPS += \
./EWARM/startup_stm32h563xx.d 

OBJS += \
./EWARM/startup_stm32h563xx.o 


# Each subdirectory must supply rules for building sources it contributes
EWARM/%.o: ../EWARM/%.s EWARM/subdir.mk
	$(error unable to generate command line)

clean: clean-EWARM

clean-EWARM:
	-$(RM) ./EWARM/startup_stm32h563xx.d ./EWARM/startup_stm32h563xx.o

.PHONY: clean-EWARM

