#include <types.h>
#include <errno.h>
#include <vcpu.h>
#include <cpufeatures.h>
#include <cpu_caps.h>
#include <sgx.h>
#include <cpuid.h>
#include <irq.h>
#include <vm.h>
#include <logmsg.h>

#define SGX_OPTED_IN (MSR_IA32_FEATURE_CONTROL_SGX_GE | MSR_IA32_FEATURE_CONTROL_LOCK)

static bool is_sgx_supported = false;
static struct epc_sections platform_epc_sections;			/* platform physcial epc sections */
static struct virt_epc_sections vm_epc_sections[CONFIG_MAX_VM_NUM];	/* virtual epc sections for VMs */
/* vars used when alloc epc resource */
static uint32_t alloc_sec_id;
static uint32_t alloc_sec_free_size;

static inline uint64_t get_epc_section_val(uint64_t low, uint64_t high)
{
	return ((high & CPUID_SGX_EPC_VAL_HIGH_MASK) << 32U) | (low & CPUID_SGX_EPC_VAL_LOW_MASK);

}

static int32_t init_epc_section(struct epc_sections *sections, uint64_t base, uint64_t size)
{
	int32_t ret = 0;
	struct epc_section *section = &sections->sections[sections->count];

 	if (size == 0UL) {
		pr_err("%s: epc section size invalid 0x%llx", __func__, size);
		ret = -EINVAL;
	} else {
		section->base = base;
		section->size = size;
		sections->count++;
	}

 	return ret;
}


/* Get physical epc resource */
static int32_t init_platform_epc_sections(void)
{
	uint32_t eax, ebx, ecx, edx;
	uint32_t i;
	uint32_t type;
	uint64_t paddr, size;
	int32_t ret = 0;

 	for (i = 0U; i < CONFIG_MAX_EPC_SECTIONS + 1U; i++) {
		cpuid_subleaf(CPUID_SGX_LEAF, i + CPUID_SGX_EPC_SUBLEAF_BASE, &eax, &ebx, &ecx, &edx);
		type = eax & CPUID_SGX_EPC_TYPE_MASK;
		if (type == CPUID_SGX_EPC_TYPE_INVALID) {
			break;
		} else if (type == CPUID_SGX_EPC_TYPE_VALID) {
			if (i == CONFIG_MAX_EPC_SECTIONS) {
				pr_warn("%s: more than %u epc sections, ignored", __func__, CONFIG_MAX_EPC_SECTIONS);
				break;
			}
			paddr = get_epc_section_val((uint64_t)eax, (uint64_t)ebx);
			size = get_epc_section_val((uint64_t)ecx, (uint64_t)edx);
			ret = init_epc_section(&platform_epc_sections, paddr, size);
		} else {
			pr_err("%s: unsupport EPC type %u", __func__, type);
			ret = -EINVAL;
		}

 		if (ret != 0) {
			break;
		}
	}

 	return ret;
}

static void map_vm_epc(enum acrn_vm_load_order load_order, struct acrn_vm_config *config,
	struct virt_epc_sections *vm_sections)
{
	uint32_t sec_id;
	uint64_t gpa_base;
	uint64_t epc_size = 0UL;

 	if (load_order == SOS_VM) {
		/* keep identical mapping */
		for (sec_id = 0U; sec_id < vm_sections->count; sec_id++) {
			vm_sections->sections[sec_id].gpa_base = vm_sections->sections[sec_id].hpa_base;
		}
	} else {
		gpa_base = config->epc.base;
		for (sec_id = 0U; sec_id < vm_sections->count; sec_id++) {
			vm_sections->sections[sec_id].gpa_base = gpa_base + epc_size;
			epc_size += vm_sections->sections[sec_id].size;
		}
	}
}

static int32_t alloc_vm_epc(uint16_t vm_id)
{
	struct acrn_vm_config *vm_config;
	uint64_t request_size;
	uint32_t vm_epc_sec_id = 0U;
	struct virt_epc_sections *vm_sections = &vm_epc_sections[vm_id];
	int32_t ret = 0;

 	vm_config = get_vm_config(vm_id);
	request_size = vm_config->epc.size;
	if ((vm_config->load_order != UNDEFINED_VM) && (request_size != 0UL)) {
		while(request_size != 0UL) {
			if (alloc_sec_free_size == 0UL) {
				alloc_sec_id++;
				if (alloc_sec_id == platform_epc_sections.count) {
					break;
				}
				alloc_sec_free_size = platform_epc_sections.sections[alloc_sec_id].size;
			}

 			if (request_size <= alloc_sec_free_size) {
				vm_sections->sections[vm_epc_sec_id].size = request_size;
				vm_sections->sections[vm_epc_sec_id].hpa_base =
					platform_epc_sections.sections[alloc_sec_id].base +
					platform_epc_sections.sections[alloc_sec_id].size - alloc_sec_free_size;
				alloc_sec_free_size -= request_size;
				request_size = 0UL;
 			} else if (alloc_sec_free_size != 0UL){
				vm_sections->sections[vm_epc_sec_id].size = alloc_sec_free_size;
				vm_sections->sections[vm_epc_sec_id].hpa_base =
					platform_epc_sections.sections[alloc_sec_id].base +
					platform_epc_sections.sections[alloc_sec_id].size - alloc_sec_free_size;
 				vm_epc_sec_id++;
				request_size -= alloc_sec_free_size;
				alloc_sec_free_size = 0UL;
			} else {
				/* do nothing */
			}
		}
		if (request_size == 0UL) {
			vm_sections->count = vm_epc_sec_id + 1U;
			map_vm_epc(vm_config->load_order, vm_config, vm_sections);
		} else {
			ret = -ENOMEM;
		}
	}
	return ret;
}

static int32_t init_vm_epc_sections(void)
{
	uint16_t vm_id;
	int ret = 0;

 	alloc_sec_id = 0U;
	alloc_sec_free_size = platform_epc_sections.sections[alloc_sec_id].size;

 	for (vm_id = 0U; vm_id < CONFIG_MAX_VM_NUM; vm_id++) {
		if (alloc_vm_epc(vm_id) != 0) {
			ret = -ENOMEM;
			break;
		}
	}

 	return ret;
}

struct epc_sections* get_platform_epc(void)
{
	return &platform_epc_sections;
}

 struct virt_epc_sections* get_vm_epc(uint16_t vm_id)
{
	return &vm_epc_sections[vm_id];
}

void sgx_init(void)
{
	int32_t ret;
	uint64_t feature_control = msr_read(MSR_IA32_FEATURE_CONTROL);

	if (pcpu_has_cap(X86_FEATURE_SGX)) {
		if ((feature_control & SGX_OPTED_IN) == SGX_OPTED_IN){
	 		ret = init_platform_epc_sections();
		 	if(ret == 0) {
				ret = init_vm_epc_sections();
				if (ret == 0) {
					is_sgx_supported = true;
				} else {
					pr_err("%s: no enough EPC, change PRM size in BIOS or vm config", __func__);
				}
			}
		} else {
			pr_warn("%s: sgx not opt-in, please opt-in in BIOS/Firmware", __func__);
		}
	}
}

bool is_vm_sgx_supported(uint16_t vm_id)
{
	struct virt_epc_sections* sections;

 	sections = get_vm_epc(vm_id);

 	return (is_sgx_supported && (sections->count != 0U));
}
