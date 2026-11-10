################################################################################
# MRS Version: 3.0.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_clock.c \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_debug.c \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_fifo.c \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_font.c \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_function.c \
d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_interrupt.c 

C_DEPS += \
./zf_common/zf_common_clock.d \
./zf_common/zf_common_debug.d \
./zf_common/zf_common_fifo.d \
./zf_common/zf_common_font.d \
./zf_common/zf_common_function.d \
./zf_common/zf_common_interrupt.d 

OBJS += \
./zf_common/zf_common_clock.o \
./zf_common/zf_common_debug.o \
./zf_common/zf_common_fifo.o \
./zf_common/zf_common_font.o \
./zf_common/zf_common_function.o \
./zf_common/zf_common_interrupt.o 

DIR_OBJS += \
./zf_common/*.o \

DIR_DEPS += \
./zf_common/*.d \

DIR_EXPANDS += \
./zf_common/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
zf_common/zf_common_clock.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_clock.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
zf_common/zf_common_debug.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_debug.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
zf_common/zf_common_fifo.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_fifo.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
zf_common/zf_common_font.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_font.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
zf_common/zf_common_function.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_function.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
zf_common/zf_common_interrupt.o: d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common/zf_common_interrupt.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/Libraries/doc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_components" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Core" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Ld" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Peripheral" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/sdk/Startup" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/user/inc" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_common" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_device" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/project/code" -I"d:/Project/智能车/CHV307_Library-master/practice/摄像头/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

