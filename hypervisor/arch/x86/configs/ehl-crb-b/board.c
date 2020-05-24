/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * BIOS Information
 * Vendor: Intel Corporation
 * Version: EHLSFWI1.R00.2115.A00.2003130949
 * Release Date: 03/13/2020
 *
 * Base Board Information
 * Manufacturer: Intel Corporation
 * Product Name: ElkhartLake LPDDR4x T3 CRB
 * Version: 2
 */

#include <board.h>
#include <vtd.h>
#include <msr.h>
#include <pci.h>
static struct dmar_dev_scope drhd0_dev_scope[DRHD0_DEV_CNT] = {
	{
		.type   = DRHD0_DEVSCOPE0_TYPE,
		.id     = DRHD0_DEVSCOPE0_ID,
		.bus    = DRHD0_DEVSCOPE0_BUS,
		.devfun = DRHD0_DEVSCOPE0_PATH,
	},
};

static struct dmar_dev_scope drhd1_dev_scope[DRHD1_DEV_CNT] = {
	{
		.type   = DRHD1_DEVSCOPE0_TYPE,
		.id     = DRHD1_DEVSCOPE0_ID,
		.bus    = DRHD1_DEVSCOPE0_BUS,
		.devfun = DRHD1_DEVSCOPE0_PATH,
	},
	{
		.type   = DRHD1_DEVSCOPE1_TYPE,
		.id     = DRHD1_DEVSCOPE1_ID,
		.bus    = DRHD1_DEVSCOPE1_BUS,
		.devfun = DRHD1_DEVSCOPE1_PATH,
	},
};

static struct dmar_drhd drhd_info_array[DRHD_COUNT] = {
	{
		.dev_cnt       = DRHD0_DEV_CNT,
		.segment       = DRHD0_SEGMENT,
		.flags         = DRHD0_FLAGS,
		.reg_base_addr = DRHD0_REG_BASE,
		.ignore        = DRHD0_IGNORE,
		.devices       = drhd0_dev_scope
	},
	{
		.dev_cnt       = DRHD1_DEV_CNT,
		.segment       = DRHD1_SEGMENT,
		.flags         = DRHD1_FLAGS,
		.reg_base_addr = DRHD1_REG_BASE,
		.ignore        = DRHD1_IGNORE,
		.devices       = drhd1_dev_scope
	},
};

struct dmar_info plat_dmar_info = {
	.drhd_count = DRHD_COUNT,
	.drhd_units = drhd_info_array,
};

#ifdef CONFIG_RDT_ENABLED
struct platform_clos_info platform_l2_clos_array[MAX_PLATFORM_CLOS_NUM] = {
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 0,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 1,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 2,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 3,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 4,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 5,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 6,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 7,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 8,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 9,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 10,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 11,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 12,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 13,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 14,
	},
	{
		.clos_mask = 0xfffU,
		.msr_index = MSR_IA32_L2_MASK_BASE + 15,
	},
};

struct platform_clos_info platform_l3_clos_array[MAX_PLATFORM_CLOS_NUM];
struct platform_clos_info platform_mba_clos_array[MAX_PLATFORM_CLOS_NUM];
#endif

static const struct cpu_cx_data board_cpu_cx[3] = {
	{{SPACE_FFixedHW, 0x01U, 0x02U, 0x01U, 0x00UL}, 0x01U, 0x01U, 0x00U},	/* C1 */
	{{SPACE_FFixedHW, 0x01U, 0x02U, 0x01U, 0x31UL}, 0x02U, 0x61U, 0x00U},	/* C2 */
	{{SPACE_FFixedHW, 0x01U, 0x02U, 0x01U, 0x60UL}, 0x03U, 0x40AU, 0x00U},	/* C3 */
};

/* Px data is not available */
static const struct cpu_px_data board_cpu_px[0];

const struct cpu_state_table board_cpu_state_tbl = {
	"Genuine Intel(R) CPU 0000 @ 1.90GHz",
	{(uint8_t)ARRAY_SIZE(board_cpu_px), board_cpu_px,
	(uint8_t)ARRAY_SIZE(board_cpu_cx), board_cpu_cx}
};
const union pci_bdf plat_hidden_pdevs[MAX_HIDDEN_PDEVS_NUM];
