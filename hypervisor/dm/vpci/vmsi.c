/*
 * Copyright (c) 2011 NetApp, Inc.
 * Copyright (c) 2018 Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <errno.h>
#include <vm.h>
#include <ptdev.h>
#include <assign.h>
#include <vpci.h>
#include <vtd.h>
#include <logmsg.h>
#include "vpci_priv.h"


/**
 * @pre vdev != NULL
 * @pre vdev->pdev != NULL
 */
static inline void enable_disable_msi(const struct pci_vdev *vdev, bool enable)
{
	union pci_bdf pbdf = vdev->pdev->bdf;
	uint32_t capoff = vdev->msi.capoff;
	uint32_t msgctrl = pci_pdev_read_cfg(pbdf, capoff + PCIR_MSI_CTRL, 2U);

	if (enable) {
		msgctrl |= PCIM_MSICTRL_MSI_ENABLE;
	} else {
		msgctrl &= ~PCIM_MSICTRL_MSI_ENABLE;
	}
	pci_pdev_write_cfg(pbdf, capoff + PCIR_MSI_CTRL, 2U, msgctrl);
}

static inline uint16_t get_mmc(uint16_t msgctrl)
{
	return (1U << ((msgctrl & PCIM_MSICTRL_MMC_MASK) >> 1U));
}

static inline uint16_t get_mme(uint16_t msgctrl)
{
	return (1U << ((msgctrl & PCIM_MSICTRL_MME_MASK) >> 4U));
}

static void reserve_vmsi_irte(struct pci_vdev *vdev, uint16_t msgctrl)
{
	struct intr_source intr_src;
	uint16_t count = get_mmc(msgctrl);

	if ((vdev->pdev->irte_count == 0U) && (vdev->pdev->irte_start != -EINVAL)) {
		intr_src.is_msi = true;
		intr_src.src.msi.value = vdev->pdev->bdf.value;
		vdev->pdev->irte_start = dmar_reserve_irte(intr_src, count);
		if (vdev->pdev->irte_start) {
			vdev->pdev->irte_count = count;
		}
	}
}

/**
 * @brief Remap vMSI virtual address and data to MSI physical address and data
 * This function is called when physical MSI is disabled.
 *
 * @pre vdev != NULL
 * @pre vdev->vpci != NULL
 * @pre vdev->vpci->vm != NULL
 * @pre vdev->pdev != NULL
 */
static void remap_vmsi(struct pci_vdev *vdev)
{
	struct ptirq_msi_info info = {};
	union pci_bdf pbdf = vdev->pdev->bdf;
	struct acrn_vm *vm = vdev->vpci->vm;
	uint32_t capoff = vdev->msi.capoff;
	uint32_t vmsi_msgdata, vmsi_addrlo, vmsi_addrhi = 0U;
	uint16_t vector_count, i;
	uint16_t msgctrl = pci_vdev_read_cfg_u16(vdev, vdev->msi.capoff + PCIR_MSI_CTRL);

	/* Read the MSI capability structure from virtual device */
	vmsi_addrlo = pci_vdev_read_cfg_u32(vdev, capoff + PCIR_MSI_ADDR);
	if (vdev->msi.is_64bit) {
		vmsi_addrhi = pci_vdev_read_cfg_u32(vdev, capoff + PCIR_MSI_ADDR_HIGH);
		vmsi_msgdata = pci_vdev_read_cfg_u16(vdev, capoff + PCIR_MSI_DATA_64BIT);
	} else {
		vmsi_msgdata = pci_vdev_read_cfg_u16(vdev, capoff + PCIR_MSI_DATA);
	}
	info.vmsi_addr.full = (uint64_t)vmsi_addrlo | ((uint64_t)vmsi_addrhi << 32U);
	info.vmsi_data.full = vmsi_msgdata;

	reserve_vmsi_irte(vdev, msgctrl);
	vector_count = get_mme(msgctrl);

	if ((vector_count > 1) && (vdev->pdev->irte_start == -EINVAL)) {
		pr_err("IR not enabled when the device uses multi-vector MSI!!");
	} else {
		for(i = 0U; i < vector_count; i++) {
			if (ptirq_prepare_msix_remap(vm, vdev->bdf.value, pbdf.value, i, &info,
				vdev->pdev->irte_start + i) != 0) {
				break;
			}
			/* only need to programe the device once */
			if (i == 0U) {
				if (vector_count > 1U) {
					info.pmsi_addr.ir_bits.shv = 1U;
				}

				pci_pdev_write_cfg(pbdf, capoff + PCIR_MSI_ADDR, 0x4U, (uint32_t)info.pmsi_addr.full);
				if (vdev->msi.is_64bit) {
					pci_pdev_write_cfg(pbdf, capoff + PCIR_MSI_ADDR_HIGH, 0x4U,
							(uint32_t)(info.pmsi_addr.full >> 32U));
					pci_pdev_write_cfg(pbdf, capoff + PCIR_MSI_DATA_64BIT, 0x2U,
							(uint16_t)info.pmsi_data.full);
				} else {
					pci_pdev_write_cfg(pbdf, capoff + PCIR_MSI_DATA, 0x2U,
							(uint16_t)info.pmsi_data.full);
				}
			}
		}

		if (i == vector_count) {
			/* If MSI Enable is being set, make sure INTxDIS bit is set */
			enable_disable_pci_intx(pbdf, false);
			enable_disable_msi(vdev, true);
			vdev->msi.vector_enabled = vector_count;
		} else {
			/* TODO: free the previous allocated resource */
			pr_err("%s: failed to prepare msi for device %x:%x.%x",
			__func__, pbdf.bits.b, pbdf.bits.d, pbdf.bits.f);
		}
	}
}

/**
 * @pre vdev != NULL
 */
void vmsi_read_cfg(const struct pci_vdev *vdev, uint32_t offset, uint32_t bytes, uint32_t *val)
{
	/* For PIO access, we emulate Capability Structures only */
	*val = pci_vdev_read_cfg(vdev, offset, bytes);
}

/**
 * @brief Writing MSI Capability Structure
 *
 * @pre vdev != NULL
 */
void vmsi_write_cfg(struct pci_vdev *vdev, uint32_t offset, uint32_t bytes, uint32_t val)
{
	uint32_t msgctrl;

	enable_disable_msi(vdev, false);
	pci_vdev_write_cfg(vdev, offset, bytes, val);

	msgctrl = pci_vdev_read_cfg(vdev, vdev->msi.capoff + PCIR_MSI_CTRL, 2U);
	if ((msgctrl & PCIM_MSICTRL_MSI_ENABLE) != 0U) {
		remap_vmsi(vdev);
	}
}

/**
 * @pre vdev != NULL
 * @pre vdev->vpci != NULL
 * @pre vdev->vpci->vm != NULL
 */
void deinit_vmsi(struct pci_vdev *vdev)
{
	uint32_t i;

	pr_err("%s: for device %x:%x.%x", __func__, vdev->pdev->bdf.bits.b, vdev->pdev->bdf.bits.d, vdev->pdev->bdf.bits.f);

	if (has_msi_cap(vdev)) {
		for (i = 0U; i < vdev->msi.vector_enabled; i++) {
			ptirq_remove_msix_remapping(vdev->vpci->vm, vdev->bdf.value, i);
		}
		vdev->msi.vector_enabled = 0;
	}
}

/**
 * @pre vdev != NULL
 * @pre vdev->pdev != NULL
 */
void init_vmsi(struct pci_vdev *vdev)
{
	struct pci_pdev *pdev = vdev->pdev;
	uint32_t val;

	vdev->msi.capoff = pdev->msi_capoff;

	if (has_msi_cap(vdev)) {
		val = pci_pdev_read_cfg(pdev->bdf, vdev->msi.capoff, 4U);
		vdev->msi.caplen = ((val & (PCIM_MSICTRL_64BIT << 16U)) != 0U) ? 14U : 10U;
		vdev->msi.is_64bit = ((val & (PCIM_MSICTRL_64BIT << 16U)) != 0U);
		pci_vdev_write_cfg(vdev, vdev->msi.capoff, 4U, val);
	}
}

