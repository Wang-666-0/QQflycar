################################################################################
# MRS Version: 3.0.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
d:/Project/智能车/QQ飞车/project/code/car_encoder.c \
d:/Project/智能车/QQ飞车/project/code/car_motor.c \
d:/Project/智能车/QQ飞车/project/code/vision.c \
d:/Project/智能车/QQ飞车/project/code/vision_main.c 

C_DEPS += \
./code/car_encoder.d \
./code/car_motor.d \
./code/vision.d \
./code/vision_main.d 

OBJS += \
./code/car_encoder.o \
./code/car_motor.o \
./code/vision.o \
./code/vision_main.o 

DIR_OBJS += \
./code/*.o \

DIR_DEPS += \
./code/*.d \

DIR_EXPANDS += \
./code/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
code/car_encoder.o: d:/Project/智能车/QQ飞车/project/code/car_encoder.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/QQ飞车/Libraries/doc" -I"d:/Project/智能车/QQ飞车/libraries/zf_components" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Core" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Ld" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Peripheral" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Startup" -I"d:/Project/智能车/QQ飞车/project/user/inc" -I"d:/Project/智能车/QQ飞车/libraries/zf_common" -I"d:/Project/智能车/QQ飞车/libraries/zf_device" -I"d:/Project/智能车/QQ飞车/project/code" -I"d:/Project/智能车/QQ飞车/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
code/car_motor.o: d:/Project/智能车/QQ飞车/project/code/car_motor.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/QQ飞车/Libraries/doc" -I"d:/Project/智能车/QQ飞车/libraries/zf_components" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Core" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Ld" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Peripheral" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Startup" -I"d:/Project/智能车/QQ飞车/project/user/inc" -I"d:/Project/智能车/QQ飞车/libraries/zf_common" -I"d:/Project/智能车/QQ飞车/libraries/zf_device" -I"d:/Project/智能车/QQ飞车/project/code" -I"d:/Project/智能车/QQ飞车/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
code/vision.o: d:/Project/智能车/QQ飞车/project/code/vision.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/QQ飞车/Libraries/doc" -I"d:/Project/智能车/QQ飞车/libraries/zf_components" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Core" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Ld" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Peripheral" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Startup" -I"d:/Project/智能车/QQ飞车/project/user/inc" -I"d:/Project/智能车/QQ飞车/libraries/zf_common" -I"d:/Project/智能车/QQ飞车/libraries/zf_device" -I"d:/Project/智能车/QQ飞车/project/code" -I"d:/Project/智能车/QQ飞车/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
code/vision_main.o: d:/Project/智能车/QQ飞车/project/code/vision_main.c $(FLAGS_C_FILE)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/QQ飞车/Libraries/doc" -I"d:/Project/智能车/QQ飞车/libraries/zf_components" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Core" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Ld" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Peripheral" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Startup" -I"d:/Project/智能车/QQ飞车/project/user/inc" -I"d:/Project/智能车/QQ飞车/libraries/zf_common" -I"d:/Project/智能车/QQ飞车/libraries/zf_device" -I"d:/Project/智能车/QQ飞车/project/code" -I"d:/Project/智能车/QQ飞车/libraries/zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

