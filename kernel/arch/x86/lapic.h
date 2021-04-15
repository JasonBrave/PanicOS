#ifndef _ARCH_X86_LAPIC_H
#define _ARCH_X86_LAPIC_H

int lapicid(void);
void lapiceoi(void);
void lapicinit(void);
void lapicstartap(unsigned char, unsigned int);
void microdelay(int);

#endif
