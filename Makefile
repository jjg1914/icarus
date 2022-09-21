TARGET := debug

TARGETCPU := cortex-m3

VENDORDIR := vendor

CMSISVER := 5.9.0
CMSISURL := https://github.com/ARM-software/CMSIS_5/archive/refs/tags/$(CMSISVER).tar.gz
CMSISDIR := $(VENDORDIR)/CMSIS_5-5.9.0
DEVICEDIR := $(CMSISDIR)/Device/ARM/ARMCM3

MCUVER := 4.3.3
MCUURL := https://github.com/STMicroelectronics/cmsis_device_f1/archive/refs/tags/v$(MCUVER).tar.gz
MCUDIR := $(VENDORDIR)/cmsis_device_f1-4.3.3

PATCHDIR := patch

BINFILE := out.bin
CFILES := $(wildcard *.c) $(MCUDIR)/Source/Templates/system_stm32f1xx.c
ASFILES := $(wildcard *.s) $(MCUDIR)/Source/Templates/gcc/startup_stm32f100xb.s
LDFILE := $(MCUDIR)/Source/Templates/gcc/linker/STM32F100XB_FLASH.ld
OBJFILES := $(CFILES:.c=.o) $(ASFILES:.s=.o)
PYVENV := environment

# supported:
SIMULATECPU := stm32vldiscovery
SIMULATEMEM := 128M

CC := arm-none-eabi-gcc
CPP := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy
QEMU := qemu-system-aarch64
GDB := arm-none-eabi-gdb
CGDB := cgdb
PATCH := patch
PYTHON := python3

ASFLAGS := -g -mcpu=$(TARGETCPU)
CPPFLAGS := -I$(CMSISDIR)/CMSIS/Core/Include -I$(DEVICEDIR)/Include -I$(MCUDIR)/Include -D ARMCM3 -D STM32F100xB
CFLAGS := -g -MP -MD -mcpu=$(TARGETCPU)
LDFLAGS :=
OBJCOPYFLAGS := -O binary
QEMUFLAGS := -M $(SIMULATECPU) -m $(SIMULATEMEM) -nographic -no-reboot -d in_asm,int,exec,cpu,guest_errors,unimp,cpu_reset -D qemu.log
GDBFLAGS :=
CGDBFLAGS := -d $(GDB) -- $(GDBFLAGS)
PATCHFLAGS := -btp 1

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: CFLAGS += -g
debug: $(BINFILE)

.PHONY: release
release: $(BINFILE)

.PHONY: test
test: $(TARGET) | $(PYVENV)
	. $(PYVENV)/bin/activate; $(PYTHON) $(PYFLAGS) -m unittest discover

.PHONY: cgdb
cgdb: $(TARGET)
	$(CGDB) $(CGDBFLAGS) $(BINFILE:.bin=.elf)

.PHONY: gdb
gdb: $(TARGET)
	$(GDB) $(GDBFLAGS) $(BINFILE:.bin=.elf)

.PHONY: qemu
qemu: $(TARGET)
	$(QEMU) $(QEMUFLAGS) -kernel $(BINFILE)

.PHONY: qemu_debug
qemu_debug: QEMUFLAGS += -s -S
qemu_debug: qemu

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(OBJFILES:.o=.d) $(BINFILE) $(BINFILE:.bin=.elf)

$(BINFILE): $(BINFILE:.bin=.elf)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	
$(BINFILE:.bin=.elf): $(LDFILE) $(OBJFILES)
	$(LD) $(LDFLAGS) -T $^ -o $@

.PHONY: deps
deps: $(CMSISDIR) $(MCUDIR)

%: %.tar.gz
	tar -C $(dir $@) -xzf $<
	find $(PATCHDIR) -name $(notdir $@).patch -exec $(PATCH) $(PATCHFLAGS) -d $@ -i "$(shell pwd)/{}" ';'

$(CMSISDIR).tar.gz: | $(VENDORDIR)
	wget $(CMSISURL) -O $@

$(MCUDIR).tar.gz: | $(VENDORDIR)
	wget $(MCUURL) -O $@

$(VENDORDIR):
	mkdir -p $(VENDORDIR)

$(PYVENV):
	$(PYTHON) -m venv $@

-include $(OBJFILES:.o=.d)
