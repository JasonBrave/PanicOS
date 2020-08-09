#ifndef _LIBCPU_CPUINFO_H
#define _LIBCPU_CPUINFO_H

#ifdef __cplusplus
extern "C" {
#endif

// max-cpuid.c
unsigned int cpu_get_max_cpuid(void);
unsigned int cpu_get_max_ext_cpuid(void);

// brand-str.c
char* cpu_get_vendor_string(char* vendor_str);
char* cpu_get_brand_string(char* brand_str);

// address-size.c
unsigned int cpu_get_physical_address_size(void);
unsigned int cpu_get_virtual_address_size(void);

#ifdef __cplusplus
}
#endif

#endif
