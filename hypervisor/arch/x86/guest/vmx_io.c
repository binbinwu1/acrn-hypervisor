/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <types.h>
#include <errno.h>
#include <atomic.h>
#include <io_req.h>
#include <vcpu.h>
#include <vm.h>
#include <instr_emul.h>
#include <vmexit.h>
#include <vmx.h>
#include <ept.h>
#include <trace.h>
#include <logmsg.h>

void arch_fire_vhm_interrupt(void)
{
	/*
	 * use vLAPIC to inject vector to SOS vcpu 0 if vlapic is enabled
	 * otherwise, send IPI hardcoded to BOOT_CPU_ID
	 */
	struct acrn_vm *sos_vm;
	struct acrn_vcpu *vcpu;

	sos_vm = get_sos_vm();
	vcpu = vcpu_from_vid(sos_vm, BOOT_CPU_ID);

	vlapic_set_intr(vcpu, get_vhm_notification_vector(), LAPIC_TRIG_EDGE);
}

/**
 * @brief General complete-work for port I/O emulation
 *
 * @pre io_req->io_type == REQ_PORTIO
 *
 * @remark This function must be called when \p io_req is completed, after
 * either a previous call to emulate_io() returning 0 or the corresponding VHM
 * request having transferred to the COMPLETE state.
 */
void
emulate_pio_complete(struct acrn_vcpu *vcpu, const struct io_request *io_req)
{
	const struct pio_request *pio_req = &io_req->reqs.pio;
	uint64_t mask = 0xFFFFFFFFUL >> (32UL - (8UL * pio_req->size));

	if (pio_req->direction == REQUEST_READ) {
		uint64_t value = (uint64_t)pio_req->value;
		uint64_t rax = vcpu_get_gpreg(vcpu, CPU_REG_RAX);

		rax = ((rax) & ~mask) | (value & mask);
		vcpu_set_gpreg(vcpu, CPU_REG_RAX, rax);
	}
}


/**
 * @brief The handler of VM exits on I/O instructions
 *
 * @param vcpu The virtual CPU which triggers the VM exit on I/O instruction
 */
int32_t pio_instr_vmexit_handler(struct acrn_vcpu *vcpu)
{
	int32_t status;
	uint64_t exit_qual;
	uint32_t mask;
	int32_t cur_context_idx = vcpu->arch.cur_context;
	struct io_request *io_req = &vcpu->req;
	struct pio_request *pio_req = &io_req->reqs.pio;

	exit_qual = vcpu->arch.exit_qualification;

	io_req->io_type = REQ_PORTIO;
	pio_req->size = vm_exit_io_instruction_size(exit_qual) + 1UL;
	pio_req->address = vm_exit_io_instruction_port_number(exit_qual);
	if (vm_exit_io_instruction_access_direction(exit_qual) == 0UL) {
		mask = 0xFFFFFFFFU >> (32U - (8U * pio_req->size));
		pio_req->direction = REQUEST_WRITE;
		pio_req->value = (uint32_t)vcpu_get_gpreg(vcpu, CPU_REG_RAX) & mask;
	} else {
		pio_req->direction = REQUEST_READ;
	}

	TRACE_4I(TRACE_VMEXIT_IO_INSTRUCTION,
		(uint32_t)pio_req->address,
		(uint32_t)pio_req->direction,
		(uint32_t)pio_req->size,
		(uint32_t)cur_context_idx);

	status = emulate_io(vcpu, io_req);

	return status;
}

static void debug_show_ept(struct acrn_vcpu *vcpu, uint64_t *pml4_page, uint64_t gpa)
{
	uint64_t *pml4e;
	uint64_t *pdpte;
	uint64_t *pde;
	uint64_t *pte;
	bool large_page;

	pml4e = pml4e_offset(pml4_page, gpa);

	pr_err("%s: 0x%016llx", __func__, gpa);

	if ((*pml4e & 0x7UL) != 0UL) {
		pr_err("EPT: pml4e = 0x%016llx", *pml4e);
		pdpte = pml4e_page_vaddr(*pml4e) + pdpte_index(gpa);
		if ((*pdpte & 0x7UL) != 0UL) {
			large_page = (bool)(*pdpte & 0x80UL);
			pr_err("EPT: pdpte = 0x%016llx, 1GB Page (%u)", *pdpte, large_page);
			if (large_page)
				return;
			pde = pdpte_page_vaddr(*pdpte) + pde_index(gpa);
			if ((*pde & 0x7UL) != 0UL) {
				large_page = (bool)(*pde & 0x80UL);
				pr_err("EPT: pde = 0x%016llx, 2MB Page (%u)", *pde, large_page);
				if (large_page)
					return;
				pte = pde_page_vaddr(*pde) + pte_index(gpa);
				if ((*pte & 0x7UL) != 0UL) {
					pr_err("EPT: pte = 0x%016llx, 4KB Page", *pte);
				}
			}
		}


	}
	pr_err("%s done\n", __func__);
}

int32_t ept_violation_vmexit_handler(struct acrn_vcpu *vcpu)
{
	int32_t status = -EINVAL, ret;
	uint64_t exit_qual;
	uint64_t gpa;
	struct io_request *io_req = &vcpu->req;
	struct mmio_request *mmio_req = &io_req->reqs.mmio;

	/* Handle page fault from guest */
	exit_qual = vcpu->arch.exit_qualification;

	io_req->io_type = REQ_MMIO;

	/* Specify if read or write operation */
	if ((exit_qual & 0x2UL) != 0UL) {
		/* Write operation */
		mmio_req->direction = REQUEST_WRITE;
		mmio_req->value = 0UL;

		/* XXX: write access while EPT perm RX -> WP */
		if ((exit_qual & 0x38UL) == 0x28UL) {
			io_req->io_type = REQ_WP;
		}
	} else {
		/* Read operation */
		mmio_req->direction = REQUEST_READ;

		/* TODO: Need to determine how sign extension is determined for
		 * reads
		 */
	}

	/* Get the guest physical address */
	gpa = exec_vmread64(VMX_GUEST_PHYSICAL_ADDR_FULL);

	TRACE_2L(TRACE_VMEXIT_EPT_VIOLATION, exit_qual, gpa);

	/* Adjust IPA appropriately and OR page offset to get full IPA of abort
	 */
	mmio_req->address = gpa;

	ret = decode_instruction(vcpu);
	if (ret > 0) {
		mmio_req->size = (uint64_t)ret;
		/*
		 * For MMIO write, ask DM to run MMIO emulation after
		 * instruction emulation. For MMIO read, ask DM to run MMIO
		 * emulation at first.
		 */

		/* Determine value being written. */
		if (mmio_req->direction == REQUEST_WRITE) {
			status = emulate_instruction(vcpu);
			if (status != 0) {
				ret = -EFAULT;
			}
		}

		if (ret > 0) {
			status = emulate_io(vcpu, io_req);
		}
	} else {
		if (ret == -EFAULT) {
			pr_info("page fault happen during decode_instruction");
			status = 0;
		}
	}

	if (ret <= 0) {
		pr_acrnlog("Guest Linear Address: 0x%016llx", exec_vmread(VMX_GUEST_LINEAR_ADDR));
		pr_acrnlog("Guest Physical Address address: 0x%016llx", gpa);
	}
	return status;
}

/**
 * @brief Allow a VM to access a port I/O range
 *
 * This API enables direct access from the given \p vm to the port I/O space
 * starting from \p port_address to \p port_address + \p nbytes - 1.
 *
 * @param vm The VM whose port I/O access permissions is to be changed
 * @param port_address The start address of the port I/O range
 * @param nbytes The size of the range, in bytes
 */
void allow_guest_pio_access(struct acrn_vm *vm, uint16_t port_address,
		uint32_t nbytes)
{
	uint16_t address = port_address;
	uint32_t *b;
	uint32_t i;

	b = (uint32_t *)vm->arch_vm.io_bitmap;
	for (i = 0U; i < nbytes; i++) {
		b[address >> 5U] &= ~(1U << (address & 0x1fU));
		address++;
	}
}

void deny_guest_pio_access(struct acrn_vm *vm, uint16_t port_address,
		uint32_t nbytes)
{
	uint16_t address = port_address;
	uint32_t *b;
	uint32_t i;

	b = (uint32_t *)vm->arch_vm.io_bitmap;
	for (i = 0U; i < nbytes; i++) {
		b[address >> 5U] |= (1U << (address & 0x1fU));
		address++;
	}
}
