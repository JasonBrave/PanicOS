.text
	.globl _start
_start:
	li a0, 0x10000000 			# Load UART base address
	lb a1, 0x5(a0)
	li a2, 'A'
	sb a2, 0x0(a0)
	la sp, (stack + 0x4000)
	j kmain

.comm stack, 0x4000
