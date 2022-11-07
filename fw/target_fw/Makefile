############## Settings
# MCU in avr/io.h define format
# MCU_DEF := __AVR_ATmega644P__
MCU_DEF := __AVR_ATmega328P__
# MCU_DEF := __AVR_ATtiny24__
MCU := $(shell echo $(MCU_DEF:__AVR_%__=%) | tr A-Z a-z)

F_CPU := 12500000

TARGET_NAME := target_fw

CXX := avr-g++
OBJCOPY := avr-objcopy
OBJDUMP := avr-objdump
SIZE := avr-size
XXD := xxd

CXX_FLAGS += -Os
CXX_FLAGS += -flto
CXX_FLAGS += -std=c++17
# Enable this for delay.h to work - probably shouldnt be used due to variable
# clock rate
# CXX_FLAGS += -DF_CPU=$(F_CPU)
CXX_FLAGS += -mmcu=$(MCU)
# Warnings
CXX_FLAGS += -Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wdouble-promotion
CXX_FLAGS += -Wunused-parameter -Wpadded
CXX_FLAGS += -Wsign-conversion -Wfloat-conversion -Wundef -Wstack-usage=16
CXX_FLAGS += -Wformat=2 -Wformat-overflow -Wformat-truncation
# Optimisations
CXX_FLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
CXX_FLAGS += -fshort-enums -fpack-struct
# Diagnostics/artefacts
CXX_FLAGS += -fstack-usage -MMD

# Useful when generating compile_commands.json
ifdef INCLUDE_AVR_HEADERS
	CXX_FLAGS += -I/usr/avr/include
	CXX_FLAGS += -D$(MCU_DEF)=1
endif

############## Rules

SRC := src
BUILD := build

SRCS := $(shell find src -type f -name "*.cpp" -print)
OBJS := $(SRCS:$(SRC)%.cpp=$(BUILD)%.o)
DEPS = $(OBJS:%.o=%.d)

MAKEFILE := Makefile

TARGET_ELF := $(BUILD)/$(TARGET_NAME).elf
TARGET_STACK_USAGE := $(wildcard $(TARGET_ELF)*.su)
TARGET_BIN := $(BUILD)/$(TARGET_NAME).bin
TARGET_HEADER := $(BUILD)/$(TARGET_NAME).h

PREV_LINE_ANSI := \033[F
RED_ANSI := \033[0;31m
GREEN_ANSI := \033[0;32m
MAGENTA_ANSI := \033[0;35m
NC_ANSI := \033[0m

define setup_target
	@echo -e "--- $(GREEN_ANSI)Building $(1)$(NC_ANSI)"
	@mkdir -p $(dir $(1))
endef

.PHONY: all stats
all: $(TARGET_HEADER)

stats: $(TARGET_ELF)
	@echo -e "$(MAGENTA_ANSI)Stack Usage:$(NC_ANSI)"
	@cat $(TARGET_STACK_USAGE)
	@echo -e "$(MAGENTA_ANSI)Memory Usage:$(NC_ANSI)"
	@$(SIZE) -C --mcu=$(MCU) $(TARGET_ELF)
	@echo -en "$(PREV_LINE_ANSI)"
	@echo -en "$(MAGENTA_ANSI)Uses __do_copy_data: "
	@if [[ "$(shell $(OBJDUMP) -t $(TARGET_ELF))" == *__do_copy_data* ]]; then \
		echo -e "$(RED_ANSI)Yes$(NC_ANSI)"; \
	else \
		echo -e "$(GREEN_ANSI)No$(NC_ANSI)"; \
	fi

$(TARGET_HEADER): $(TARGET_BIN)
	$(call setup_target,$@)
	@cd $(@D) && $(XXD) -i $(<F) $(@F)

$(TARGET_BIN): $(TARGET_ELF)
	$(call setup_target,$@)
	@$(OBJCOPY) -O binary $< $@

$(TARGET_ELF): $(OBJS)
	$(call setup_target,$@)
	@$(CXX) $(CXX_FLAGS) $^ -o $@

-include $(DEPS)
$(BUILD)/%.o: $(SRC)/%.cpp $(MAKEFILE)
	$(call setup_target,$@)
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD)
