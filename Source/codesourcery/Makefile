####################################################################
# Makefile                                                         #
####################################################################

.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release clean

####################################################################
# Definitions                                                      #
####################################################################

DEVICE 		= EFM32GG280F1024
BOARD 		= CubeCompV2B
#BOARD       = CubeCompV3
PROJECTNAME = Source

OBJ_DIR = build
EXE_DIR = exe
LST_DIR = lst

####################################################################
# Definitions of toolchain.                                        #
# You might need to do changes to match your system setup          #
####################################################################

# Change path to CodeSourcery tools according to your system configuration
WINDOWSCS = CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_EABI
LINUXCS   = /cad/codesourcery/arm-none-eabi/arm-2010q1
GCCVERSION = $(shell $(CC) -dumpversion)

ifeq ($(ComSpec),)
  ifeq ($(COMSPEC),)
    # Assume we are making on a linux platform
    TOOLDIR = $(LINUXCS)
    RM = rm -rf
  else
    TOOLDIR = $(PROGRAMFILES)/$(WINDOWSCS)
    RM = "$(TOOLDIR)/bin/cs-rm" -rf
    QUOTE ="
  endif
else
  TOOLDIR = $(ProgramFiles)/$(WINDOWSCS)
  RM = "$(TOOLDIR)/bin/cs-rm" -rf
  QUOTE ="
endif

CC      = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-gcc$(QUOTE)
LD      = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-ld$(QUOTE)
AR      = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-ar$(QUOTE)
OBJCOPY = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-objcopy$(QUOTE)
DUMP    = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-objdump$(QUOTE) --disassemble

####################################################################
# Flags                                                            #
####################################################################

# -MMD : Don't generate dependencies on system header files.
# -MP  : Add phony targets, useful when a h-file is removed from a project.
# -MF  : Specify a file to write the dependencies to.
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

# Add -Wa,-ahld=$(LST_DIR)/$(@F:.o=.lst) to CFLAGS to produce assembly list files
CFLAGS += -D$(DEVICE) -D$(BOARD) -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections \
-mfix-cortex-m3-ldrd -fomit-frame-pointer -Wall -DDEBUG_EFM  $(DEPFLAGS)

ASMFLAGS += -x assembler-with-cpp

LDFLAGS += -Xlinker -Map=$(LST_DIR)/$(PROJECTNAME).map -mcpu=cortex-m3 -mthumb \
-T../../libraries/Device/EnergyMicro/EFM32GG/Source/G++/efm32gg.ld \
-L"$(TOOLDIR)/arm-none-eabi/lib/thumb2" \
-L"$(TOOLDIR)/lib/gcc/arm-none-eabi/$(GCCVERSION)/thumb2" \
-Wl,--gc-sections

LIBS = -Wl,--start-group -lgcc -lc -lcs3 -lcs3unhosted -Wl,--end-group

INCLUDEPATHS += \
-I.. \
-I../../libraries/CMSIS/Include \
-I../../libraries/Device/EnergyMicro/EFM32GG/Include \
-I../../libraries/emlib/inc \
-I../../libraries/bspLib/inc \
-I../../libraries/flashLib \
-I../../libraries/flashLib/device \
-I../../libraries/flashLib/asp \
-I../../libraries/flashLib/trace \
-I../../libraries/fatfs/inc \
-I../../libraries/FreeRTOS/Source/include \
-I../../libraries/FreeRTOS/Source/portable/GCC/ARM_CM3 \
-I../../libraries/FSW/inc \
-I../../libraries/Interface/inc

####################################################################
# Files                                                            #
####################################################################

C_SRC +=  \
../../libraries/Device/EnergyMicro/EFM32GG/Source/system_efm32gg.c \
../../libraries/emlib/src/em_system.c \
../../libraries/emlib/src/em_adc.c \
../../libraries/emlib/src/em_assert.c \
../../libraries/emlib/src/em_cmu.c \
../../libraries/emlib/src/em_dma.c \
../../libraries/emlib/src/em_ebi.c \
../../libraries/emlib/src/em_emu.c \
../../libraries/emlib/src/em_gpio.c \
../../libraries/emlib/src/em_i2c.c \
../../libraries/emlib/src/em_rtc.c \
../../libraries/emlib/src/em_usart.c \
../../libraries/emlib/src/em_wdog.c \
../../libraries/emlib/src/em_acmp.c \
../../libraries/bspLib/src/bsp_acmp.c \
../../libraries/bspLib/src/bsp_uart.c \
../../libraries/bspLib/src/bsp_rtc.c \
../../libraries/bspLib/src/bsp_wdg.c \
../../libraries/bspLib/src/bsp_adc.c \
../../libraries/bspLib/src/bsp_dma.c \
../../libraries/bspLib/src/bsp_i2c.c \
../../libraries/bspLib/src/bsp_ebi.c \
../../libraries/flashLib/lld.c \
../../libraries/fatfs/src/ff.c \
../../libraries/fatfs/src/diskio.c \
../../libraries/fatfs/src/microsd.c \
../../libraries/FreeRTOS/Source/list.c \
../../libraries/FreeRTOS/Source/queue.c \
../../libraries/FreeRTOS/Source/tasks.c \
../../libraries/FreeRTOS/Source/timers.c \
../../libraries/FreeRTOS/Source/portable/MemMang/heap_4.c \
../../libraries/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c \
../../libraries/FSW/src/fsw_adcs.c \
../../libraries/FSW/src/fsw_cdh.c \
../../libraries/FSW/src/fsw_comm.c \
../../libraries/FSW/src/fsw_filesystem.c \
../../libraries/FSW/src/fsw_healthandhousekeeping.c \
../../libraries/FSW/src/fsw_payload.c \
../../libraries/FSW/src/fsw_power.c \
../../libraries/FSW/src/fsw_modes.c \
../../libraries/FSW/src/z_HILcomm.c \
../../libraries/Interface/src/CubeSense.1.c \
../background.c \
../comms.c \
../tests.c \
../main.c

S_SRC +=  \
../../libraries/Device/EnergyMicro/EFM32GG/Source/G++/startup_efm32gg.s

####################################################################
# Rules                                                            #
####################################################################

C_FILES = $(notdir $(C_SRC) )
S_FILES = $(notdir $(S_SRC) )
#make list of source paths, sort also removes duplicates
C_PATHS = $(sort $(dir $(C_SRC) ) )
S_PATHS = $(sort $(dir $(S_SRC) ) )

C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.o))
S_OBJS = $(addprefix $(OBJ_DIR)/, $(S_FILES:.s=.o))
C_DEPS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.d))

vpath %.c $(C_PATHS)
vpath %.s $(S_PATHS)

# Default build is debug build
all:      debug

debug:    CFLAGS += -DDEBUG -O0 -g3
debug:    $(OBJ_DIR) $(LST_DIR) $(EXE_DIR) $(EXE_DIR)/$(PROJECTNAME).bin

release:  CFLAGS += -DNDEBUG -O0 -g3 
release:  $(OBJ_DIR) $(LST_DIR) $(EXE_DIR) $(EXE_DIR)/$(PROJECTNAME).bin

# Create directories
$(OBJ_DIR):
	mkdir $(OBJ_DIR)
	@echo "Created build directory."

$(EXE_DIR):
	mkdir $(EXE_DIR)
	@echo "Created executable directory."

$(LST_DIR):
	mkdir $(LST_DIR)
	@echo "Created list directory."

# Create objects from C SRC files
$(OBJ_DIR)/%.o: %.c
	@echo "Building file: $<"
	$(CC) $(CFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Assemble .s files
$(OBJ_DIR)/%.o: %.s
	@echo "Assembling $<"
	$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Link
$(EXE_DIR)/$(PROJECTNAME).out: $(C_OBJS) $(S_OBJS)
	@echo "Linking target: $@"
	$(CC) $(LDFLAGS) $(C_OBJS) $(S_OBJS) $(LIBS) -o $(EXE_DIR)/$(PROJECTNAME).out

# Create binary file
$(EXE_DIR)/$(PROJECTNAME).bin: $(EXE_DIR)/$(PROJECTNAME).out
	@echo "Creating binary file"
	$(OBJCOPY) -O binary $(EXE_DIR)/$(PROJECTNAME).out $(EXE_DIR)/$(PROJECTNAME).bin
# Uncomment next line to produce assembly listing of entire program
#	$(DUMP) $(EXE_DIR)/$(PROJECTNAME).out>$(LST_DIR)/$(PROJECTNAME)out.lst

clean:
	$(RM) $(OBJ_DIR) $(LST_DIR) $(EXE_DIR)

# include auto-generated dependency files (explicit rules)
ifneq (clean,$(findstring clean, $(MAKECMDGOALS)))
-include $(C_DEPS)
endif
