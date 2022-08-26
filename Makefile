TARGETCPU := cortex-m3

VENDORDIR := vendor
CMSISVER := 5.9.0
CMSISURL := https://github.com/ARM-software/CMSIS_5/archive/refs/tags/$(CMSISVER).tar.gz
CMSISDIR := $(VENDORDIR)/CMSIS_5-5.9.0
DEVICEDIR := $(CMSISDIR)/Device/ARM/ARMCM3

PATCHDIR := patch

TARGET := out.bin
CFILES := $(wildcard *.c) $(DEVICEDIR)/Source/startup_ARMCM3.c $(DEVICEDIR)/Source/system_ARMCM3.c
ASFILES := $(wildcard *.s)
LDFILE := $(DEVICEDIR)/Source/GCC/gcc_arm.ld
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
CPPFLAGS := -I$(CMSISDIR)/CMSIS/Core/Include -I$(DEVICEDIR)/Include -D ARMCM3 -D __INITIAL_SP=__StackTop -D __PROGRAM_START=_main
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
	
$(TARGET:.bin=.elf): $(LDFILE:.pp.ld=.ld) $(OBJFILES)
	$(LD) $(LDFLAGS) -T $^ -o $@

.PHONY: deps
deps: $(CMSISDIR)

%: %.tar.gz
	tar -C $(dir $@) -xzf $<
	[ -f $(PATCHDIR)/$(notdir $@).patch ] && $(PATCH) $(PATCHFLAGS) -d $@ < $(PATCHDIR)/$(notdir $@).patch

$(CMSISDIR).tar.gz: | $(VENDORDIR)
	wget $(CMSISURL) -O $@

$(VENDORDIR):
	mkdir -p $(VENDORDIR)

-include $(OBJFILES:.o=.d)
