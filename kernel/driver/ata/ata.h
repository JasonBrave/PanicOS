#ifndef _DRIVER_ATA_ATA_H
#define _DRIVER_ATA_ATA_H

#include <driver/pci/pci.h>

void ata_adapter_init(const struct PciAddress* addr);
void ata_init(void);
void ata_legacy_intr(int irq);
void ata_pci_intr(const struct PciAddress* addr);

#endif
