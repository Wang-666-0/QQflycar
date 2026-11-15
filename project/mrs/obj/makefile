################################################################################
# MRS Version: 3.0.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf
FLAGS_C :=  -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -I"d:/Project/智能车/QQ飞车/Libraries/doc" -I"d:/Project/智能车/QQ飞车/libraries/zf_components" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Core" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Ld" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Peripheral" -I"d:/Project/智能车/QQ飞车/libraries/sdk/Startup" -I"d:/Project/智能车/QQ飞车/project/user/inc" -I"d:/Project/智能车/QQ飞车/libraries/zf_common" -I"d:/Project/智能车/QQ飞车/libraries/zf_device" -I"d:/Project/智能车/QQ飞车/project/code" -I"d:/Project/智能车/QQ飞车/libraries/zf_driver" -std=gnu11
FLAGS_C_FILE := .compile_c_flags

FLAGS_CPP :=  -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -std=gnu++11 -fabi-version=0
FLAGS_CPP_FILE := .compile_cpp_flags

FLAGS_ASM :=  -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -x assembler -I"d:/Project/智能车/QQ飞车/libraries/sdk/Startup" -I"d:/Project/智能车/QQ飞车/libraries/zf_driver"
FLAGS_ASM_FILE := .compile_asm_flags

FLAGS_LD:=  -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -T "d:\Project\智能车\QQ飞车\libraries\sdk\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"d:\Project\智能车\QQ飞车\project\mrs" -L"d:\Project\智能车\QQ飞车\libraries\zf_device" -Wl,-Map,"Seekfree_CH32V307VCT6_Opensource_Library.map" --specs=nano.specs -u _printf_float --specs=nosys.specs -Wl,--print-memory-usage
FLAGS_LD_FILE := .ld_flags

$(FLAGS_C_FILE): FORCE
	@echo '$(FLAGS_C)' | cmp -s - $@ || echo '$(FLAGS_C)' > $@
$(FLAGS_CPP_FILE): FORCE
	@echo '$(FLAGS_CPP)' | cmp -s - $@ || echo '$(FLAGS_CPP)' > $@
$(FLAGS_ASM_FILE): FORCE
	@echo '$(FLAGS_ASM)' | cmp -s - $@ || echo '$(FLAGS_ASM)' > $@
$(FLAGS_LD_FILE): FORCE
	@echo '$(FLAGS_LD)' | cmp -s - $@ || echo '$(FLAGS_LD)' > $@
.PHONY: FORCE
FORCE:

# All of the sources participating in the build are defined here
-include sources.mk
-include zf_driver/subdir.mk
-include zf_device/subdir.mk
-include zf_components/subdir.mk
-include zf_common/subdir.mk
-include user_c/subdir.mk
-include sdk/Startup/subdir.mk
-include sdk/Peripheral/subdir.mk
-include sdk/Core/subdir.mk
-include code/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

SOURCES_LIST := .sources_list
$(SOURCES_LIST): FORCE
	@echo '$(C_SRCS) $(CPP_SRCS) $(S_UPPER_SRCS)' | tr ' ' '\n' | sort > $@.tmp
	@cmp -s $@.tmp $@ || mv $@.tmp $@
	@rm -f $@.tmp

LD_SCRIPTS := d:/Project/智能车/QQ飞车/libraries/sdk/Ld/Link.ld

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
Seekfree_CH32V307VCT6_Opensource_Library.hex \

SECONDARY_LIST += \
Seekfree_CH32V307VCT6_Opensource_Library.lst \

SECONDARY_SIZE += \
Seekfree_CH32V307VCT6_Opensource_Library.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: Seekfree_CH32V307VCT6_Opensource_Library.elf secondary-outputs

# Tool invocations
Seekfree_CH32V307VCT6_Opensource_Library.elf: $(OBJS) $(USER_OBJS_ESCAPE) $(SOURCES_LIST) $(FLAGS_LD_FILE) $(LD_SCRIPTS)
	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall -g -T "d:\Project\智能车\QQ飞车\libraries\sdk\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"d:\Project\智能车\QQ飞车\project\mrs" -L"d:\Project\智能车\QQ飞车\libraries\zf_device" -Wl,-Map,"Seekfree_CH32V307VCT6_Opensource_Library.map" --specs=nano.specs -u _printf_float --specs=nosys.specs -Wl,--print-memory-usage -o "Seekfree_CH32V307VCT6_Opensource_Library.elf" $(OBJS) $(LIBS)
Seekfree_CH32V307VCT6_Opensource_Library.hex: Seekfree_CH32V307VCT6_Opensource_Library.elf
	@	riscv-none-embed-objcopy -O ihex "Seekfree_CH32V307VCT6_Opensource_Library.elf" "Seekfree_CH32V307VCT6_Opensource_Library.hex"
Seekfree_CH32V307VCT6_Opensource_Library.lst: Seekfree_CH32V307VCT6_Opensource_Library.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "Seekfree_CH32V307VCT6_Opensource_Library.elf" > "Seekfree_CH32V307VCT6_Opensource_Library.lst"
Seekfree_CH32V307VCT6_Opensource_Library.siz: Seekfree_CH32V307VCT6_Opensource_Library.elf
	riscv-none-embed-size --format=berkeley "Seekfree_CH32V307VCT6_Opensource_Library.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) Seekfree_CH32V307VCT6_Opensource_Library.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
	-$(RM) $(FLAGS_C_FILE) $(FLAGS_CPP_FILE) $(FLAGS_ASM_FILE) $(FLAGS_LD_FILE) $(SOURCES_LIST)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets