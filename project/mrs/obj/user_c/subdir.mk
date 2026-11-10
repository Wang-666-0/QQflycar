################################################################################
# MRS Version: 3.0.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/src/isr.c \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/src/main.c 

C_DEPS += \
./user_c/isr.d \
./user_c/main.d 

OBJS += \
./user_c/isr.o \
./user_c/main.o 

DIR_OBJS += \
./user_c/*.o \

DIR_DEPS += \
./user_c/*.d \

DIR_EXPANDS += \
./user_c/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
user_c/isr.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/src/isr.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
user_c/main.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/src/main.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

