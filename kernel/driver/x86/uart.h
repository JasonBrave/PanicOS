#ifndef _DRIVER_X86_UART_H
#define _DRIVER_X86_UART_H

void uart_init(void);
void uart_putc(unsigned char c);
void uart_intr(int irq);

#endif
