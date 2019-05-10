#include <types.h>
#include <errno.h>
#include <vcpu.h>
#include <sgx.h>
#include <epc_manager.h>
#include <cpuid.h>
#include <irq.h>
#include <vm.h>
#include <logmsg.h>

// static uint64_t epc_section_base;
// static uint64_t epc_section_page_count;

/* SGX related CPUIDs:
 * cpuid_07h_ebx[02] SGX. Supports Intel® SGX Extensions if 1, and requires opt-in enabling by BIOS via
 *                   IA32_FEATURE_CONTROL MSR. Software is considered to have opted into Intel SGX if and only if
 *                   IA32_FEATURE_CONTROL.SGX_ENABLE and IA32_FEATURE_CONTROL.LOCK are set to 1.
 * cpuid_07h_ecx[30] SGX_LC. Supports SGX Launch Configuration if 1.
 * cpuid_12h_01    Intel SGX Capability Enumeration Leaf
 * cpuid_12h_02    Intel SGX Attributes Enumeration Leaf
 * cpuid_12h_03... Intel SGX EPC Enumeration Leaf
 */
// static uint32_t cpuid_07h_ebx;
// static bool is_sgx1_supported()


bool is_sgx_supported = false;

static inline int encls(uint64_t rax, uint64_t rbx, uint64_t rcx, uint64_t rdx)
{
	int ret;

	asm volatile ( "ENCLS;\n\t"
		     : "=a" (ret)
		     : "a" (rax), "b" (rbx), "c" (rcx), "d" (rdx)
		     : "memory", "cc");

	return ret;
}

int32_t sgx_init(void)
{
	int32_t ret;

	ret = init_platform_epc_sections();

	if(ret == 0) {
		ret = init_vm_epc_sections();
	}
	if (ret == 0) {
		is_sgx_supported = true;
	}

	uint16_t vm_id;
	uint32_t section_id;
	struct virt_epc_sections* sections;
	for(vm_id = 0U; vm_id < CONFIG_MAX_VM_NUM; vm_id++) {
		sections = get_vm_epc(vm_id);
		for(section_id = 0U; section_id < sections->count; section_id++) {
			pr_err("%s vm[%u] section[%u] hpa_base=0x%016llx, gpa_base=0x%016llx, size=0x%llx",
				__func__, vm_id, section_id,
				sections->sections[section_id].hpa_base,
				sections->sections[section_id].gpa_base,
				sections->sections[section_id].size);
		}
	}

	return ret;
}

bool is_vm_sgx_supported(uint16_t vm_id)
{
	struct virt_epc_sections* sections;

	sections = get_vm_epc(vm_id);

	return (sections->count != 0U);
}




#define ENCLS_OP_ALIGN_CHECK_MASK	0x0FFFUL
#define ENCLS_OP_ALIGN_PAGE_MASK	0x0FFFUL
#define ENCLS_OP_ALIGN_32BYTE_MASK	0x1FUL
#define ENCLS_OP_ALIGN_64BYTE_MASK	0x3FUL

#define ENCLS_OP_IS_EPC_MASK		0x1000UL
#define ENCLS_OP_NEED_COPY_MASK		0x2000UL
/* @pre vcpu!= NULL
 * @pre data != NULL
 */
static int32_t get_encls_op(struct acrn_vcpu *vcpu, void* output, uint64_t gva, uint64_t size, uint64_t attr_mask)
{
	int32_t ret = 0;
	uint32_t err_code = 0U;
	uint64_t gpa;
//	uint64_t hva;

	if ((gva & (attr_mask & ENCLS_OP_ALIGN_CHECK_MASK)) != 0UL) {
		vcpu_inject_gp(vcpu, 0);
		ret = -EINVAL;
	} else {

		gva2gpa(vcpu, gva, &gpa, &err_code);

		if (err_code != 0U) {
			vcpu_inject_pf(vcpu, err_code, gva);
			ret = -EFAULT;
		} else {
			if ((attr_mask & ENCLS_OP_NEED_COPY_MASK) != 0UL) {
				err_code = copy_from_gpa(vcpu->vm, output, gpa, size);
			} else {
				*((uint64_t *)output) = (uint64_t)gpa2hva(vcpu->vm, gpa);
			}
		}

		if (err_code) {
			vcpu_inject_pf(vcpu, err_code, gva);
			ret = -EFAULT;
		}
	}

	return ret;
}

static int32_t handle_ecreate(struct acrn_vcpu *vcpu)
{
	struct sgx_pageinfo guest_pageinfo;
	struct sgx_pageinfo pageinfo;
	uint64_t secs;
	int32_t ret;

	pr_err("%s: ECREATE", __func__);

	ret = get_encls_op(vcpu, &guest_pageinfo, vcpu_get_gpreg(vcpu, CPU_REG_RBX), sizeof(guest_pageinfo),
		ENCLS_OP_ALIGN_32BYTE_MASK | ENCLS_OP_NEED_COPY_MASK);
	if (ret != 0) {
		goto out;
	}

	ret = get_encls_op(vcpu, &secs, vcpu_get_gpreg(vcpu, CPU_REG_RCX), sizeof(uint64_t),
		ENCLS_OP_ALIGN_PAGE_MASK);
	// guest_secs = vcpu_get_gpreg(vcpu, CPU_REG_RCX);
	// gva2gpa(vcpu, guest_secs, &temp_gpa, &err_code);
	// secs = (uint64_t)gpa2hva(vcpu->vm, temp_gpa);
	if (ret != 0) {
		goto out;
	}

	pageinfo.linaddr = 0UL;
	pageinfo.secs = 0UL;

	ret = get_encls_op(vcpu, &pageinfo.srcpge, guest_pageinfo.srcpge, sizeof(uint64_t),
		ENCLS_OP_ALIGN_PAGE_MASK);
	if (ret != 0) {
		goto out;
	}
	// gva2gpa(vcpu, guest_pageinfo.srcpge, &temp_gpa, &err_code);
	// pageinfo.srcpge = (uint64_t)gpa2hva(vcpu->vm, temp_gpa);

	ret = get_encls_op(vcpu, &pageinfo.metadata, guest_pageinfo.metadata, sizeof(uint64_t),
		ENCLS_OP_ALIGN_64BYTE_MASK);

	if (ret != 0) {
		goto out;
	}
	// gva2gpa(vcpu, guest_pageinfo.metadata, &temp_gpa, &err_code);
	// pageinfo.metadata = (uint64_t)gpa2hva(vcpu->vm, temp_gpa);

	stac();
	encls(SGX_LEAF_FUNCTION_ECREATE, (uint64_t)&pageinfo, secs, 0);
	clac();

out:
	if (ret)
		pr_err("%s: fault occurred", __func__);

	return 0;

}

int32_t sgx_encls_vmexit_handler(struct acrn_vcpu *vcpu)
{
	uint32_t leaf_function = (uint64_t) vcpu_get_gpreg(vcpu, CPU_REG_RAX);
	int32_t ret = 0;

	pr_err("%s: vcpu pid=%d, vid=%d", __func__, vcpu->pcpu_id, vcpu->vcpu_id);

	if (leaf_function == SGX_LEAF_FUNCTION_ECREATE) {
		ret = handle_ecreate(vcpu);

	} else {
		pr_err("unexpected exit on ENCLS[%u]", leaf_function);
	}

	return ret;
};
