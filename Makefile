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

TARGET := out.bin
CFILES := $(wildcard *.c) $(MCUDIR)/Source/Templates/system_stm32f1xx.c
ASFILES := $(wildcard *.s) $(MCUDIR)/Source/Templates/gcc/startup_stm32f100xb.s
LDFILE := $(MCUDIR)/Source/Templates/gcc/linker/STM32F100XB_FLASH.ld
OBJFILES := $(CFILES:.c=.o) $(ASFILES:.s=.o)

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

ASFLAGS := -g -mcpu=$(TARGETCPU)
CPPFLAGS := -I$(CMSISDIR)/CMSIS/Core/Include -I$(DEVICEDIR)/Include -I$(MCUDIR)/Include -D ARMCM3 -D STM32F100xB
#CPPFLAGS := -I$(MCUDIR)/Include -D ARMCM3 -D __INITIAL_SP=__StackTop -D __LIMIT_SP=__StackLimit -D __PROGRAM_START=_main
CFLAGS := -g -MP -MD -mcpu=$(TARGETCPU)
LDFLAGS :=
OBJCOPYFLAGS := -O binary
QEMUFLAGS := -M $(SIMULATECPU) -m $(SIMULATEMEM) -nographic -no-reboot -d in_asm,int,exec,cpu,guest_errors,unimp,cpu_reset -D qemu.log #-serial mon:stdio
GDBFLAGS :=
CGDBFLAGS := -d $(GDB) -- $(GDBFLAGS)
PATCHFLAGS := -btp 1

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET)
	$(QEMU) $(QEMUFLAGS) -s -S -kernel $< & $(CGDB) $(CGDBFLAGS) $(<:.bin=.elf); kill -TERM %%1 || true
	#$(QEMU) $(QEMUFLAGS) -s -S -kernel $< & $(GDB) $(GDBFLAGS) $(<:.bin=.elf); kill -TERM %%1 || true

.PHONY: qemu
qemu: $(TARGET)
	$(QEMU) $(QEMUFLAGS) -kernel $<

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(OBJFILES:.o=.d) $(TARGET) $(TARGET:.bin=.elf)

$(TARGET): $(TARGET:.bin=.elf)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	
$(TARGET:.bin=.elf): $(LDFILE) $(OBJFILES)
	$(LD) $(LDFLAGS) -T $^ -o $@

.PHONY: deps
deps: $(CMSISDIR) $(MCUDIR)

%: %.tar.gz
	tar -C $(dir $@) -xzf $<
	find $(PATCHDIR) -name $(notdir $@).patch -exec $(PATCH) $(PATCHFLAGS) -d $@ -i '{}' ';'

$(CMSISDIR).tar.gz: | $(VENDORDIR)
	wget $(CMSISURL) -O $@

$(MCUDIR).tar.gz: | $(VENDORDIR)
	wget $(MCUURL) -O $@

$(VENDORDIR):
	mkdir -p $(VENDORDIR)

-include $(OBJFILES:.o=.d)
