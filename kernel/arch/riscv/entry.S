.text
	.globl _start
_start:
	la sp, (stack + 0x4000)
	j kmain

.comm stack, 0x4000
