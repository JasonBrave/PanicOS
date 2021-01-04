/*
 * Device manager
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _DEVMGR_DEVICE_CPP
#define _DEVMGR_DEVICE_CPP

#include <cstdint>
#include <string>
#include <vector>

enum class DevmgrResourceType {
	io,
	memory,
	irq,
	msi,
	msix,
};

struct DevmgrResource {
	DevmgrResourceType type;
	std::uint64_t addr;
	std::uint64_t size;
};

class DevmgrDevice {
public:
	std::string name;
	std::string dev_addr;
	std::string driver_name;
	std::vector<DevmgrResource> resource;
};

extern std::vector<DevmgrDevice> devmgr_devices;

void devices_init();

#endif
