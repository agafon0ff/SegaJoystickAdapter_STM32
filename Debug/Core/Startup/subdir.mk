################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f042f4px.s 

S_DEPS += \
./Core/Startup/startup_stm32f042f4px.d 

OBJS += \
./Core/Startup/startup_stm32f042f4px.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/startup_stm32f042f4px.o: ../Core/Startup/startup_stm32f042f4px.s
	arm-none-eabi-gcc -mcpu=cortex-m0 -g3 -c -x assembler-with-cpp -MMD -MP -MF"Core/Startup/startup_stm32f042f4px.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

