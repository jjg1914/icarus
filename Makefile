TARGET := out.bin
CFILES := main.c
ASFILES := boot.s
LDFILE := link.ld
OBJFILES := $(CFILES:.c=.o) $(ASFILES:.s=.o)

TARGETCPU := arm926ej-s #cortex-m3 
SIMULATECPU := versatilepb #stm32vldiscovery
SIMULATEMEM := 128M

CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy
QEMU := qemu-system-arm

ASFLAGS := -mcpu=$(TARGETCPU)
CFLAGS := -MP -MD -mcpu=$(TARGETCPU)
LDFLAGS := -T $(LDFILE)
OBJCOPYFLAGS := -O binary
QEMUFLAGS := -M $(SIMULATECPU) -m $(SIMULATEMEM) -nographic -no-reboot

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: CFLAGS += -g
debug: ASFLAGS += -g
debug: all

.PHONY: qemu
qemu: $(TARGET)
	$(QEMU) $(QEMUFLAGS) -kernel $<

.PHONY: debug_qemu
debug_qemu: QEMUFLAGS += -s -S
debug_qemu: debug qemu

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(OBJFILES:.o=.d) $(TARGET) $(TARGET:.bin=.elf)

$(TARGET): $(TARGET:.bin=.elf)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	
$(TARGET:.bin=.elf): $(OBJFILES)
	$(LD) $(LDFLAGS) $^ -o $@

-include $(OBJFILES:.o=.d)
