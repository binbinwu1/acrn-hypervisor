#ifndef EPC_MANAGER_H
#define EPC_MANAGER_H

#define CONFIG_MAX_EPC_SECTIONS 		0x2U
#define CONFIG_EPC_SECTION_SIZE_MAX 		(128UL*(1UL << 20U))
#define CPUID_SGX_LEAF				0x12U
#define CPUID_SGX_EPC_SUBLEAF_BASE 		0x2U
#define CPUID_SGX_EPC_TYPE_MASK			0xFU
#define CPUID_SGX_EPC_TYPE_INVALID		0x0U
#define CPUID_SGX_EPC_TYPE_VALID		0x1U
#define CPUID_SGX_EPC_VAL_HIGH_MASK		0x000FFFFFUL
#define CPUID_SGX_EPC_VAL_LOW_MASK		0xFFFFF000UL

struct epc_section
{
	uint64_t base;
	uint64_t size;
};

struct epc_sections
{
	struct epc_section sections[CONFIG_MAX_EPC_SECTIONS];
	uint32_t count;
};

struct virt_epc_section
{
	uint64_t hpa_base;
	uint64_t gpa_base;
	uint64_t size;
};

struct virt_epc_sections
{
	struct virt_epc_section sections[CONFIG_MAX_EPC_SECTIONS];
	uint32_t count;
};

int32_t init_platform_epc_sections(void);
int32_t init_vm_epc_sections(void);
struct epc_sections* get_platform_epc(void);
struct virt_epc_sections* get_vm_epc(uint16_t vm_id);

#endif