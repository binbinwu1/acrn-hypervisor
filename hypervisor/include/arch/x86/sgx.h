#ifndef SGX_H
#define SGX_H

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

struct epc_sections* get_platform_epc(void);
struct virt_epc_sections* get_vm_epc(uint16_t vm_id);
bool is_vm_sgx_supported(uint16_t vm_id);
void sgx_init(void);

#endif
