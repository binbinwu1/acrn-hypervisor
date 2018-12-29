/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hypervisor.h>

/*
 * According to "SDM APPENDIX C VMX BASIC EXIT REASONS",
 * there are 65 Basic Exit Reasons.
 */
#define NR_VMX_EXIT_REASONS	65U

static int32_t unhandled_vmexit_handler(struct acrn_vcpu *vcpu);
static int32_t xsetbv_vmexit_handler(struct acrn_vcpu *vcpu);
static int32_t wbinvd_vmexit_handler(struct acrn_vcpu *vcpu);

/* VM Dispatch table for Exit condition handling */
static const struct vm_exit_dispatch dispatch_table[NR_VMX_EXIT_REASONS] = {
	[VMX_EXIT_REASON_EXCEPTION_OR_NMI] = {
		.handler = exception_vmexit_handler},
	[VMX_EXIT_REASON_EXTERNAL_INTERRUPT] = {
		.handler = external_interrupt_vmexit_handler},
	[VMX_EXIT_REASON_TRIPLE_FAULT] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_INIT_SIGNAL] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_STARTUP_IPI] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_IO_SMI] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_OTHER_SMI] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_INTERRUPT_WINDOW] = {
		.handler = interrupt_window_vmexit_handler},
	[VMX_EXIT_REASON_NMI_WINDOW] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_TASK_SWITCH] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_CPUID] = {
		.handler = cpuid_vmexit_handler},
	[VMX_EXIT_REASON_GETSEC] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_HLT] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_INVD] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_INVLPG] = {
		.handler = unhandled_vmexit_handler,},
	[VMX_EXIT_REASON_RDPMC] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_RDTSC] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_RSM] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMCALL] = {
		.handler = vmcall_vmexit_handler},
	[VMX_EXIT_REASON_VMCLEAR] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMLAUNCH] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMPTRLD] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMPTRST] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMREAD] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMRESUME] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMWRITE] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMXOFF] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMXON] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_CR_ACCESS] = {
		.handler = cr_access_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_DR_ACCESS] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_IO_INSTRUCTION] = {
		.handler = pio_instr_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_RDMSR] = {
		.handler = rdmsr_vmexit_handler},
	[VMX_EXIT_REASON_WRMSR] = {
		.handler = wrmsr_vmexit_handler},
	[VMX_EXIT_REASON_ENTRY_FAILURE_INVALID_GUEST_STATE] = {
		.handler = unhandled_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_ENTRY_FAILURE_MSR_LOADING] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_MWAIT] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_MONITOR_TRAP] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_MONITOR] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_PAUSE] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_ENTRY_FAILURE_MACHINE_CHECK] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_TPR_BELOW_THRESHOLD] = {
		.handler = tpr_below_threshold_vmexit_handler},
	[VMX_EXIT_REASON_APIC_ACCESS] = {
		.handler = apic_access_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_VIRTUALIZED_EOI] = {
		.handler = veoi_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_GDTR_IDTR_ACCESS] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_LDTR_TR_ACCESS] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_EPT_VIOLATION] = {
		.handler = ept_violation_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_EPT_MISCONFIGURATION] = {
		.handler = ept_misconfig_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_INVEPT] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_RDTSCP] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMX_PREEMPTION_TIMER_EXPIRED] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_INVVPID] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_WBINVD] = {
		.handler = wbinvd_vmexit_handler},
	[VMX_EXIT_REASON_XSETBV] = {
		.handler = xsetbv_vmexit_handler},
	[VMX_EXIT_REASON_APIC_WRITE] = {
		.handler = apic_write_vmexit_handler,
		.need_exit_qualification = 1},
	[VMX_EXIT_REASON_RDRAND] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_INVPCID] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_VMFUNC] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_ENCLS] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_RDSEED] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_PAGE_MODIFICATION_LOG_FULL] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_XSAVES] = {
		.handler = unhandled_vmexit_handler},
	[VMX_EXIT_REASON_XRSTORS] = {
		.handler = unhandled_vmexit_handler}
};

void debug_dump_host_state(void)
{
	uint64_t flags;
	uint64_t host_cr0, host_cr3, host_cr4, host_rsp;
	uint16_t cs_sel;
	uint16_t ds_sel;
	uint16_t ss_sel;
	uint16_t es_sel;
	uint16_t fs_sel;
	uint16_t gs_sel;

	asm volatile("pushfq; popq %0" :"=m"(flags));
	asm volatile("mov %%cr0, %0" : "=r" (host_cr0));
	asm volatile("mov %%cr3, %0" : "=r" (host_cr3));
	asm volatile("mov %%cr4, %0" : "=r" (host_cr4));
	asm volatile("mov %%rsp, %0" : "=r" (host_rsp));
	CPU_SEG_READ(cs, &cs_sel);
	CPU_SEG_READ(ds, &ds_sel);
	CPU_SEG_READ(ss, &ss_sel);
	CPU_SEG_READ(es, &es_sel);
	CPU_SEG_READ(fs, &fs_sel);
	CPU_SEG_READ(gs, &gs_sel);
	// asm volatile("mov %%cs, %0" : "=r" (cs_sel));
	// asm volatile("mov %%ds, %0" : "=r" (ds_sel));
	// asm volatile("mov %%ss, %0" : "=r" (ss_sel));
	// asm volatile("mov %%es, %0" : "=r" (es_sel));
	// asm volatile("mov %%fs, %0" : "=r" (fs_sel));
	// asm volatile("mov %%gs, %0" : "=r" (gs_sel));
	pr_err("DEBUG: =  Host flags =0x%016llx ", flags);
	pr_err("DEBUG: =  Host CR0   =0x%016llx, 0x%016llx", exec_vmread(VMX_HOST_CR0), host_cr0);
	pr_err("DEBUG: =  Host CR3   =0x%016llx, 0x%016llx ", exec_vmread(VMX_HOST_CR3), host_cr3);
	pr_err("DEBUG: =  Host CR4   =0x%016llx, 0x%016llx ", exec_vmread(VMX_HOST_CR4), host_cr4);
	pr_err("DEBUG: =  Host RSP   =0x%016llx, 0x%016llx ", exec_vmread(VMX_HOST_RSP), host_rsp);
	pr_err("DEBUG: =  Host RIP   =0x%016llx", exec_vmread(VMX_HOST_RIP));
	pr_err("DEBUG: =  Host CS sel %04llx, %04llx ", exec_vmread(VMX_HOST_CS_SEL), cs_sel);
	pr_err("DEBUG: =  Host SS sel %04llx, %04llx ", exec_vmread(VMX_HOST_SS_SEL), ds_sel);
	pr_err("DEBUG: =  Host DS sel %04llx, %04llx ", exec_vmread(VMX_HOST_DS_SEL), ss_sel);
	pr_err("DEBUG: =  Host ES sel %04llx, %04llx ", exec_vmread(VMX_HOST_ES_SEL), es_sel);
	pr_err("DEBUG: =  Host FS sel %04llx, %04llx ", exec_vmread(VMX_HOST_FS_SEL), fs_sel);
	pr_err("DEBUG: =  Host GS sel %04llx, %04llx ", exec_vmread(VMX_HOST_GS_SEL), gs_sel);
	pr_err("DEBUG: =  Host TR sel %04llx ", exec_vmread(VMX_HOST_TR_SEL));

	pr_err("DEBUG: =  Host GDTR Base %016llx ", exec_vmread(VMX_HOST_GDTR_BASE));
	pr_err("DEBUG: =  Host IDTR Base %016llx ", exec_vmread(VMX_HOST_IDTR_BASE));
	pr_err("DEBUG: =  Host TR Base %016llx ", exec_vmread(VMX_HOST_TR_BASE));
	pr_err("DEBUG: =  Host GS Base %016llx ", exec_vmread(VMX_HOST_GS_BASE));
	pr_err("DEBUG: =  Host FS Base %016llx ", exec_vmread(VMX_HOST_FS_BASE));
	pr_err("DEBUG: =  Host SYSENTER   CS=%08x, ESP=0x%016llx , EIP=0x%016llx",
		exec_vmread32(VMX_HOST_IA32_SYSENTER_CS),
		exec_vmread(VMX_HOST_IA32_SYSENTER_ESP),
		exec_vmread(VMX_HOST_IA32_SYSENTER_EIP));

	pr_err("DEBUG: host PAT:  %016llx, %016llx", msr_read(MSR_IA32_PAT), exec_vmread(VMX_HOST_IA32_PAT_FULL));
	pr_err("DEBUG: host EFER: %016llx, %016llx", msr_read(MSR_IA32_EFER), exec_vmread(VMX_HOST_IA32_EFER_FULL));

	pr_err("DEBUG: host VMX_EXIT_CTLS: %016llx, %016llx", msr_read(MSR_IA32_VMX_EXIT_CTLS), exec_vmread(VMX_EXIT_CONTROLS));
	pr_err("DEBUG: host PERF_GLOBAL_CTRL: %016llx", msr_read(MSR_IA32_PERF_GLOBAL_CTRL));
}

/* for debug purpose */
void debug_dump_guest_cpu_regs(struct acrn_vcpu *vcpu)
{
	uint32_t instruction_err = 0;
	instruction_err = exec_vmread32(VMX_INSTR_ERROR);
	pr_info("DEBUG: =  PCPU ID %d ==== VCPU ID %d====", vcpu->pcpu_id, vcpu->vcpu_id);
	pr_info("DEBUG: vmexit fail err_inst=%x, exit_reason=%lx", instruction_err, vcpu->arch.exit_reason);

	if (instruction_err) {
		debug_dump_host_state();
	}

	pr_err("DEBUG: =  RIP   =0x%016llx  , 0x%016llx", exec_vmread(VMX_GUEST_RIP), vcpu_get_rip(vcpu));
	pr_err("DEBUG: =  RFLAGS=0x%016llx  , 0x%016llx", exec_vmread(VMX_GUEST_RFLAGS), vcpu_get_rflags(vcpu));
	pr_err("DEBUG: =  CR0   =0x%016llx  , 0x%016llx", exec_vmread(VMX_GUEST_CR0), vcpu_get_cr0(vcpu));
	pr_err("DEBUG: =  CR2   =0x%016llx  , ------------------", vcpu_get_cr2(vcpu));
	pr_err("DEBUG: =  CR3   =0x%016llx  , ------------------", exec_vmread(VMX_GUEST_CR3));
	pr_err("DEBUG: =  CR4   =0x%016llx  , 0x%016llx", exec_vmread(VMX_GUEST_CR4), vcpu_get_cr4(vcpu));
	pr_err("DEBUG: =  EFER  =0x%016llx  , 0x%016llx", exec_vmread(VMX_GUEST_IA32_EFER_FULL), vcpu_get_efer(vcpu));

	pr_err("DEBUG: =  CS sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_CS_SEL), exec_vmread(VMX_GUEST_CS_ATTR), exec_vmread(VMX_GUEST_CS_LIMIT), exec_vmread(VMX_GUEST_CS_BASE));
	pr_err("DEBUG: =  SS sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_SS_SEL), exec_vmread(VMX_GUEST_SS_ATTR), exec_vmread(VMX_GUEST_SS_LIMIT), exec_vmread(VMX_GUEST_SS_BASE));
	pr_err("DEBUG: =  DS sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_DS_SEL), exec_vmread(VMX_GUEST_DS_ATTR), exec_vmread(VMX_GUEST_DS_LIMIT), exec_vmread(VMX_GUEST_DS_BASE));
	pr_err("DEBUG: =  ES sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_ES_SEL), exec_vmread(VMX_GUEST_ES_ATTR), exec_vmread(VMX_GUEST_ES_LIMIT), exec_vmread(VMX_GUEST_ES_BASE));
	pr_err("DEBUG: =  FS sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_FS_SEL), exec_vmread(VMX_GUEST_FS_ATTR), exec_vmread(VMX_GUEST_FS_LIMIT), exec_vmread(VMX_GUEST_FS_BASE));
	pr_err("DEBUG: =  GS sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_GS_SEL), exec_vmread(VMX_GUEST_GS_ATTR), exec_vmread(VMX_GUEST_GS_LIMIT), exec_vmread(VMX_GUEST_GS_BASE));

	pr_err("DEBUG: =  GDTR sel ---- attr -------- limit %08llx base %016llx", exec_vmread(VMX_GUEST_GDTR_LIMIT), exec_vmread(VMX_GUEST_GDTR_BASE));
	pr_err("DEBUG: =  LDTR sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_LDTR_SEL), exec_vmread(VMX_GUEST_LDTR_ATTR), exec_vmread(VMX_GUEST_LDTR_LIMIT), exec_vmread(VMX_GUEST_LDTR_BASE));
	pr_err("DEBUG: =  IDTR sel ---- attr -------- limit %08llx base %016llx", exec_vmread(VMX_GUEST_IDTR_LIMIT), exec_vmread(VMX_GUEST_IDTR_BASE));
	pr_err("DEBUG: =  TR   sel %04llx attr %08llx limit %08llx base %016llx", exec_vmread(VMX_GUEST_TR_SEL), exec_vmread(VMX_GUEST_TR_ATTR), exec_vmread(VMX_GUEST_TR_LIMIT), exec_vmread(VMX_GUEST_TR_BASE));

	pr_err("DEBUG: =  RAX=0x%016llx    RCX=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_RAX), vcpu_get_gpreg(vcpu, CPU_REG_RCX));
	pr_err("DEBUG: =  RDX=0x%016llx    RBX=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_RDX), vcpu_get_gpreg(vcpu, CPU_REG_RBX));
	pr_err("DEBUG: =  RSP=0x%016llx  , RBP=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_RSP), vcpu_get_gpreg(vcpu, CPU_REG_RBP));
	pr_err("DEBUG: =  RSI=0x%016llx  , RDI=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_RSI), vcpu_get_gpreg(vcpu, CPU_REG_RDI));
	pr_err("DEBUG: =  R8 =0x%016llx  , R9 =0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_R8), vcpu_get_gpreg(vcpu, CPU_REG_R9));
	pr_err("DEBUG: =  R10=0x%016llx  , R11=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_R10), vcpu_get_gpreg(vcpu, CPU_REG_R11));
	pr_err("DEBUG: =  R12=0x%016llx  , R13=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_R12), vcpu_get_gpreg(vcpu, CPU_REG_R13));
	pr_err("DEBUG: =  R14=0x%016llx  , R15=0x%016llx", vcpu_get_gpreg(vcpu, CPU_REG_R14), vcpu_get_gpreg(vcpu, CPU_REG_R15));

}

int32_t vmexit_handler(struct acrn_vcpu *vcpu)
{
	struct vm_exit_dispatch *dispatch = NULL;
	uint16_t basic_exit_reason;
	int32_t ret;

	if (get_cpu_id() != vcpu->pcpu_id) {
		pr_fatal("vcpu is not running on its pcpu!");
		ret = -EINVAL;
	} else {
		/* Obtain interrupt info */
		vcpu->arch.idt_vectoring_info = exec_vmread32(VMX_IDT_VEC_INFO_FIELD);
		/* Filter out HW exception & NMI */
		if ((vcpu->arch.idt_vectoring_info & VMX_INT_INFO_VALID) != 0U) {
			uint32_t vector_info = vcpu->arch.idt_vectoring_info;
			uint32_t vector = vector_info & 0xffU;
			uint32_t type = (vector_info & VMX_INT_TYPE_MASK) >> 8U;
			uint32_t err_code = 0U;

			if (type == VMX_INT_TYPE_HW_EXP) {
				if ((vector_info & VMX_INT_INFO_ERR_CODE_VALID) != 0U) {
					err_code = exec_vmread32(VMX_IDT_VEC_ERROR_CODE);
				}
				(void)vcpu_queue_exception(vcpu, vector, err_code);
				vcpu->arch.idt_vectoring_info = 0U;
			} else if (type == VMX_INT_TYPE_NMI) {
				vcpu_make_request(vcpu, ACRN_REQUEST_NMI);
				vcpu->arch.idt_vectoring_info = 0U;
			} else {
				/* No action on EXT_INT or SW exception. */
			}
		}

		/* Calculate basic exit reason (low 16-bits) */
		basic_exit_reason = (uint16_t)(vcpu->arch.exit_reason & 0xFFFFU);

		/* Log details for exit */
		pr_dbg("Exit Reason: 0x%016llx ", vcpu->arch.exit_reason);

		if (!is_vm0(vcpu->vm)) {
			/* Log details for exit */
			pr_err("\n");
			pr_err("Exit Reason: 0x%016llx ", vcpu->arch.exit_reason);
			debug_dump_guest_cpu_regs(vcpu);
		}

		/* Ensure exit reason is within dispatch table */
		if (basic_exit_reason >= ARRAY_SIZE(dispatch_table)) {
			pr_err("Invalid Exit Reason: 0x%016llx ", vcpu->arch.exit_reason);
			ret = -EINVAL;
		} else {
			/* Calculate dispatch table entry */
			dispatch = (struct vm_exit_dispatch *)(dispatch_table + basic_exit_reason);

			/* See if an exit qualification is necessary for this exit handler */
			if (dispatch->need_exit_qualification != 0U) {
				/* Get exit qualification */
				vcpu->arch.exit_qualification = exec_vmread(VMX_EXIT_QUALIFICATION);
			}

			/* exit dispatch handling */
			if (basic_exit_reason == VMX_EXIT_REASON_EXTERNAL_INTERRUPT) {
				/* Handling external_interrupt should disable intr */
				CPU_IRQ_DISABLE();
				ret = dispatch->handler(vcpu);
				CPU_IRQ_ENABLE();
			} else {
				ret = dispatch->handler(vcpu);
			}
		}
	}

	return ret;
}

static int32_t unhandled_vmexit_handler(struct acrn_vcpu *vcpu)
{
	pr_fatal("Error: Unhandled VM exit condition from guest at 0x%016llx ",
			exec_vmread(VMX_GUEST_RIP));

	pr_fatal("Exit Reason: 0x%016llx ", vcpu->arch.exit_reason);

	pr_err("Exit qualification: 0x%016llx ",
			exec_vmread(VMX_EXIT_QUALIFICATION));

	TRACE_2L(TRACE_VMEXIT_UNHANDLED, vcpu->arch.exit_reason, 0UL);

	return 0;
}

int32_t cpuid_vmexit_handler(struct acrn_vcpu *vcpu)
{
	uint64_t rax, rbx, rcx, rdx;

	rax = vcpu_get_gpreg(vcpu, CPU_REG_RAX);
	rbx = vcpu_get_gpreg(vcpu, CPU_REG_RBX);
	rcx = vcpu_get_gpreg(vcpu, CPU_REG_RCX);
	rdx = vcpu_get_gpreg(vcpu, CPU_REG_RDX);
	guest_cpuid(vcpu, (uint32_t *)&rax, (uint32_t *)&rbx,
		(uint32_t *)&rcx, (uint32_t *)&rdx);
	vcpu_set_gpreg(vcpu, CPU_REG_RAX, rax);
	vcpu_set_gpreg(vcpu, CPU_REG_RBX, rbx);
	vcpu_set_gpreg(vcpu, CPU_REG_RCX, rcx);
	vcpu_set_gpreg(vcpu, CPU_REG_RDX, rdx);

	TRACE_2L(TRACE_VMEXIT_CPUID, (uint64_t)vcpu->vcpu_id, 0UL);

	return 0;
}

int32_t cr_access_vmexit_handler(struct acrn_vcpu *vcpu)
{
	uint64_t reg;
	uint32_t idx;
	uint64_t exit_qual;
	int32_t ret = 0;

	exit_qual = vcpu->arch.exit_qualification;
	idx = (uint32_t)vm_exit_cr_access_reg_idx(exit_qual);

	ASSERT((idx <= 15U), "index out of range");
	reg = vcpu_get_gpreg(vcpu, idx);

	switch ((vm_exit_cr_access_type(exit_qual) << 4U) |
			vm_exit_cr_access_cr_num(exit_qual)) {
	case 0x00UL:
		/* mov to cr0 */
		vcpu_set_cr0(vcpu, reg);
		break;
	case 0x04UL:
		/* mov to cr4 */
		vcpu_set_cr4(vcpu, reg);
		break;
	case 0x08UL:
		/* mov to cr8 */
		/* According to SDM 6.15 "Exception and interrupt Reference":
		 *
		 * set reserved bit in CR8 causes GP to guest
		 */
		if ((reg & ~0xFUL) != 0UL) {
			pr_dbg("Invalid cr8 write operation from guest");
			vcpu_inject_gp(vcpu, 0U);
			break;
		}
		vlapic_set_cr8(vcpu_vlapic(vcpu), reg);
		break;
	case 0x18UL:
		/* mov from cr8 */
		reg = vlapic_get_cr8(vcpu_vlapic(vcpu));
		vcpu_set_gpreg(vcpu, idx, reg);
		break;
	default:
		panic("Unhandled CR access");
		ret = -EINVAL;
		break;
	}

	TRACE_2L(TRACE_VMEXIT_CR_ACCESS, vm_exit_cr_access_type(exit_qual),
			vm_exit_cr_access_cr_num(exit_qual));

	return ret;
}

/*
 * XSETBV instruction set's the XCR0 that is used to tell for which
 * components states can be saved on a context switch using xsave.
 */
static int32_t xsetbv_vmexit_handler(struct acrn_vcpu *vcpu)
{
	int32_t idx;
	uint64_t val64;
	int32_t ret = 0;

	val64 = exec_vmread(VMX_GUEST_CR4);
	if ((val64 & CR4_OSXSAVE) == 0UL) {
		vcpu_inject_gp(vcpu, 0U);
	} else {
		idx = vcpu->arch.cur_context;
		if (idx >= NR_WORLD) {
			ret = -1;
		} else {
			/* to access XCR0,'rcx' should be 0 */
			if (vcpu_get_gpreg(vcpu, CPU_REG_RCX) != 0UL) {
				vcpu_inject_gp(vcpu, 0U);
			} else {
				val64 = (vcpu_get_gpreg(vcpu, CPU_REG_RAX) & 0xffffffffUL) |
						(vcpu_get_gpreg(vcpu, CPU_REG_RDX) << 32U);

				/* bit 0(x87 state) of XCR0 can't be cleared */
				if ((val64 & 0x01UL) == 0UL) {
					vcpu_inject_gp(vcpu, 0U);
				} else {
					/*
					 * XCR0[2:1] (SSE state & AVX state) can't not be
					 * set to 10b as it is necessary to set both bits
					 * to use AVX instructions.
					 */
					if (((val64 >> 1U) & 0x3UL) == 0x2UL) {
						vcpu_inject_gp(vcpu, 0U);
					} else {
						write_xcr(0, val64);
					}
				}
			}
		}
	}

	return ret;
}

static int32_t wbinvd_vmexit_handler(struct acrn_vcpu *vcpu)
{
	if (!iommu_snoop_supported(vcpu->vm)) {
		cache_flush_invalidate_all();
	}

	return 0;
}
