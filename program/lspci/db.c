// reference:
// PCI Code and ID Assignment Specification rev 1.11 PCI-SIG
// https://pcisig.com/sites/default/files/files/PCI_Code-ID_r_1_11__v24_Jan_2019.pdf

#include <stdint.h>

const char* pci_class_to_str(uint8_t class, uint8_t subclass, uint8_t progif) {
	// storage controllers
	if ((class == 1) && (subclass == 0)) {
		return "SCSI Controller";
	} else if ((class == 1) && (subclass == 1)) {
		return "IDE Controller";
	} else if ((class == 1) && (subclass == 2)) {
		return "Floppy Disk Controller";
	} else if ((class == 1) && (subclass == 4)) {
		return "RAID Controller";
	} else if ((class == 1) && (subclass == 5)) {
		return "ATA Controller";
	} else if ((class == 1) && (subclass == 6) && (progif == 0)) {
		return "SATA Controller (Vendor Specific)";
	} else if ((class == 1) && (subclass == 6) && (progif == 1)) {
		return "AHCI SATA Controller";
	} else if ((class == 1) && (subclass == 7)) {
		return "SAS Controller";
	} else if ((class == 1) && (subclass == 8) && (progif == 1)) {
		return "NVMHCI";
	} else if ((class == 1) && (subclass == 8) && (progif == 2)) {
		return "NVM Express";
	} else if (class == 1) {
		return "Storage Controller";
		// network controller
	} else if ((class == 2) && (subclass == 0)) {
		return "Ethernet Controller";
		// display controller
	} else if ((class == 3) && (subclass == 0)) {
		return "VGA Controller";
	} else if ((class == 3) && (subclass == 1)) {
		return "XGA Controller";
	} else if ((class == 3) && (subclass == 2)) {
		return "3D Controller";
	} else if (class == 3) {
		return "Display Controller";
		// multimedia device
	} else if ((class == 4) && (subclass == 0)) {
		return "Video Device";
	} else if ((class == 4) && (subclass == 1)) {
		return "Audio Device";
	} else if ((class == 4) && (subclass == 3)) {
		return "HD Audio Controller";
	} else if (class == 4) {
		return "Multimedia Device";
		// bridge device
	} else if ((class == 6) && (subclass == 0)) {
		return "Host Bridge";
	} else if ((class == 6) && (subclass == 1)) {
		return "ISA Bridge";
	} else if ((class == 6) && (subclass == 4) && (progif == 0)) {
		return "PCI Bridge";
	} else if ((class == 6) && (subclass == 4) && (progif == 1)) {
		return "PCI Bridge (Subtractive Decode)";
	} else if (class == 6) {
		return "Bridge Device";
		// simple communication device
	} else if ((class == 7) && (subclass == 0)) {
		return "Serial Controller";
		// serial bus controller
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x0)) {
		return "UHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x10)) {
		return "OHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x20)) {
		return "EHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x30)) {
		return "xHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0xfe)) {
		return "USB Device";
	} else if ((class == 0xc) && (subclass == 0x5)) {
		return "SMBus Controller";
	} else {
		return "Unknown Class";
	}
}

static const char* pci_cap_db[] = {
	[0x0] = "Null",
	[0x1] = "PCI Power Management",
	[0x2] = "AGP",
	[0x3] = "VPD",
	[0x4] = "Slot ID",
	[0x5] = "Message Signaled Interrupts",
	[0x6] = "CompactPCI Hot Swap",
	[0x7] = "PCI-X",
	[0x8] = "HyperTransport",
	[0x9] = "Vendor Specific",
	[0xa] = "Debug Port",
	[0xb] = "CompactPCI central resource control",
	[0xc] = "PCI Standard Hot-Plug Controller",
	[0xd] = "PCI Bridge Subsystem Vendor ID",
	[0xe] = "AGP 8x",
	[0xf] = "Secure Device",
	[0x10] = "PCI Express",
	[0x11] = "MSI-X",
	[0x12] = "SATA Index/Data Pair",
	[0x13] = "Advanced Features",
	[0x14] = "Enhanced Allocation",
	[0x15] = "FLattening Portal Bridge",
};

const char* pci_cap_to_str(unsigned int cap_id) {
	if (cap_id > 0x15) {
		return "Unknown";
	}
	return pci_cap_db[cap_id];
}

static const char* pcie_ecap_db[] = {
	[0x0] = "Null",
	[0x1] = "Advanced Error Reporting",
	[0x2] = "Virtual Channel",
	[0x3] = "Device Serial Number",
	[0x4] = "Power Budgeting",
	[0x5] = "Root Complex Link Declaration",
	[0x6] = "Root Complex Internal Link Control",
	[0x7] = "Root Complex Event Collextor Endpoint Association",
	[0x8] = "Multi-Function Virtual Channel",
	[0x9] = "Virtual Channel",
	[0xa] = "Root Complex Register Block",
	[0xb] = "Vendor Specific",
	[0xc] = "Configuration Access Correlation",
	[0xd] = "Access Control Services",
	[0xe] = "Alternative Routing-ID Interpretation",
	[0xf] = "Address Translation Services",
	[0x10] = "SR-IOV",
	[0x11] = "MR-IOV",
	[0x12] = "Multicast",
	[0x13] = "Page Request Interface",
	[0x14] = "Reserved",
	[0x15] = "Resizable BAR",
	[0x16] = "Dynamic Power Allocation",
	[0x17] = "TPH Requester",
	[0x18] = "Latency Tolerance Reporting",
	[0x19] = "Secondary PCI Express",
	[0x1a] = "Protocol Multiplexing",
	[0x1b] = "Process Address Space ID",
	[0x1c] = "LN Requester",
	[0x1d] = "Downstream Port Containment",
	[0x1e] = "L1 PM Substates",
	[0x1f] = "Precision Time Measurement",
	[0x20] = "PCI Express over M-PHY",
};

const char* pcie_ecap_to_str(unsigned int ecap_id) {
	if (ecap_id > 0x13) {
		return "Unknown";
	}
	return pcie_ecap_db[ecap_id];
}

static struct PCIDeviceIDTable {
	uint16_t vendor, device;
	const char* name;
} pci_device_id_table[] = {
	// QEMU devices
	{0x1af4, 0x1000, "Virtio Network device (legacy)"},
	{0x1af4, 0x1001, "Virtio Block device (legacy)"},
	{0x1af4, 0x1002, "Virtio Balloon device (legacy)"},
	{0x1af4, 0x1003, "Virtio Console device (legacy)"},
	{0x1af4, 0x1004, "Virtio SCSI device (legacy)"},
	{0x1af4, 0x1005, "Virtio Entropy device (legacy)"},
	{0x1af4, 0x1009, "Virtio 9P Filesystem device (legacy)"},
	{0x1af4, 0x1041, "Virtio Network device (modern)"},
	{0x1af4, 0x1042, "Virtio Block device (modern)"},
	{0x1af4, 0x1043, "Virtio Console device (modern)"},
	{0x1af4, 0x1044, "Virtio Entropy device (modern)"},
	{0x1af4, 0x1045, "Virtio Balloon device (modern)"},
	{0x1af4, 0x1048, "Virtio SCSI device (modern)"},
	{0x1af4, 0x1049, "Virtio 9P Filesystem device (modern)"},
	{0x1af4, 0x1050, "Virtio GPU device (modern)"},
	{0x1af4, 0x1052, "Virtio Input device (modern)"},
	{0x1af4, 0x1110, "QEMU ivshmem device"},
	{0x1b36, 0x0001, "QEMU PCI-PCI Bridge"},
	{0x1b36, 0x0002, "QEMU Single-port 16550A serial adapter"},
	{0x1b36, 0x0003, "QEMU Dual-port 16550A serial adapter"},
	{0x1b36, 0x0004, "QEMU Quad-port 16550A serial adapter"},
	{0x1b36, 0x0005, "QEMU PCI Test device"},
	{0x1b36, 0x0006, "QEMU Rocker Ethernet Switch"},
	{0x1b36, 0x0007, "QEMU SD Card Host Controller"},
	{0x1b36, 0x0008, "QEMU PCI Express Host Bridge"},
	{0x1b36, 0x0009, "QEMU PCI Expander Bridge"},
	{0x1b36, 0x000a, "QEMU PCI-PCI Bridge (multiseat)"},
	{0x1b36, 0x000b, "QEMU PCI Express Expander Bridge"},
	{0x1b36, 0x000c, "QEMU PCI Express Root Port"},
	{0x1b36, 0x000d, "QEMU xHCI USB 3.0 Controller"},
	{0x1b36, 0x000e, "QEMU PCI Express to PCI Bridge"},
	{0x1b36, 0x0100, "QEMU QXL Display Adapter"},
};

const char* pci_device_id_to_str(uint16_t vendor, uint16_t device) {
	for (unsigned int i = 0; i < (sizeof(pci_device_id_table) / sizeof(struct PCIDeviceIDTable));
		 i++) {
		if (pci_device_id_table[i].vendor == vendor && pci_device_id_table[i].device == device) {
			return pci_device_id_table[i].name;
		}
	}
	return 0;
}
