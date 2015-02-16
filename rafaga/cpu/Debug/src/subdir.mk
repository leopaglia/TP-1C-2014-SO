################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cpu.c \
../src/cpupanel.c \
../src/funciones.c \
../src/instrucciones.c \
../src/panel.c 

OBJS += \
./src/cpu.o \
./src/cpupanel.o \
./src/funciones.o \
./src/instrucciones.o \
./src/panel.o 

C_DEPS += \
./src/cpu.d \
./src/cpupanel.d \
./src/funciones.d \
./src/instrucciones.d \
./src/panel.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


