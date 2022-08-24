TARGETCPU := cortex-m3

TARGET := out.bin
CFILES := $(wildcard *.c)
ASFILES := $(wildcard *.s)
LDFILE := link.pp.ld
OBJFILES := $(CFILES:.c=.o) $(ASFILES:.s=.o)

# supported:
SIMULATECPU := stm32vldiscovery
SIMULATEMEM := 128M

CC := arm-none-eabi-gcc
CPP := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy
QEMU := qemu-system-arm
GDB := arm-none-eabi-gdb
CGDB := cgdb

ASFLAGS := -g -mcpu=$(TARGETCPU)
CFLAGS := -g -MP -MD -mcpu=$(TARGETCPU)
LDFLAGS :=
OBJCOPYFLAGS := -O binary
QEMUFLAGS := -M $(SIMULATECPU) -m $(SIMULATEMEM) -nographic -no-reboot -d in_asm,int,exec,cpu,guest_errors,unimp,cpu_reset -D qemu.log
GDBFLAGS :=
CGDBFLAGS := -d $(GDB) -- $(GDBFLAGS)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET)
	$(QEMU) $(QEMUFLAGS) -s -S -kernel $< & $(CGDB) $(CGDBFLAGS) $(<:.bin=.elf); kill -TERM %%1 || true

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(OBJFILES:.o=.d) $(TARGET) $(TARGET:.bin=.elf)

$(TARGET): $(TARGET:.bin=.elf)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	
$(TARGET:.bin=.elf): $(LDFILE:.pp.ld=.ld) $(OBJFILES)
	$(LD) $(LDFLAGS) -T $^ -o $@

%.ld: %.pp.ld
	$(CPP) $(CPPFLAGS) -E -xc $< | grep -v '^#' > $@

-include $(OBJFILES:.o=.d)
