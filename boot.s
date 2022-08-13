.global _boot
_boot:
 LDR sp, =stack_top
 BL _start
 LDR R0, =0x10000020
 LDR R1, =0x0000A05F
 STR R1, [R0]
 LDR R0, =0x10000040
 LDR R1, =0b100000110
 STR R1, [R0]
