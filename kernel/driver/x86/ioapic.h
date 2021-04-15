#ifndef _DRIVER_X86_IOAPIC_H
#define _DRIVER_X86_IOAPIC_H

enum IOAPICTriggerMode {
	IOAPIC_EDGE_TRIGGER = 0,
	IOAPIC_LEVEL_TRIGGER = 1,
};

enum IOAPICIntrPinPolarity {
	IOAPIC_ACTIVE_HIGH = 0,
	IOAPIC_ACTIVE_LOW = 1,
};

void ioapic_init(void);
void ioapic_enable(int irq, unsigned int lapicid, enum IOAPICTriggerMode trigger_mode,
				   enum IOAPICIntrPinPolarity polarity);
void ioapic_disable(int irq);

#endif
