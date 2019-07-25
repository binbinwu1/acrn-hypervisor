/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <board.h>
#include <vtd.h>
#include <msr.h>

static struct dmar_dev_scope drhd0_dev_scope[DRHD0_DEV_CNT] = {
	{
		.type = DRHD0_DEVSCOPE0_TYPE,
		.id = DRHD0_DEVSCOPE0_ID,
		.bus = DRHD0_DEVSCOPE0_BUS,
		.devfun = DRHD0_DEVSCOPE0_PATH
	},
};

static struct dmar_dev_scope drhd1_dev_scope[DRHD1_DEV_CNT] = {
	{
		.type = DRHD1_DEVSCOPE0_TYPE,
		.id = DRHD1_DEVSCOPE0_ID,
		.bus = DRHD1_DEVSCOPE0_BUS,
		.devfun = DRHD1_DEVSCOPE0_PATH
	},
	{
		.type = DRHD1_DEVSCOPE1_TYPE,
		.id = DRHD1_DEVSCOPE1_ID,
		.bus = DRHD1_DEVSCOPE1_BUS,
		.devfun = DRHD1_DEVSCOPE1_PATH
	},
};

static struct dmar_drhd drhd_info_array[DRHD_COUNT] = {
	{
		.dev_cnt = DRHD0_DEV_CNT,
		.segment = DRHD0_SEGMENT,
		.flags = DRHD0_FLAGS,
		.reg_base_addr = DRHD0_REG_BASE,
		.ignore = DRHD0_IGNORE,
		.devices = drhd0_dev_scope
	},
	{
		.dev_cnt = DRHD1_DEV_CNT,
		.segment = DRHD1_SEGMENT,
		.flags = DRHD1_FLAGS,
		.reg_base_addr = DRHD1_REG_BASE,
		.ignore = DRHD1_IGNORE,
		.devices = drhd1_dev_scope
	},
};

struct dmar_info plat_dmar_info = {
	.drhd_count = DRHD_COUNT,
	.drhd_units = drhd_info_array,
};

const struct cpu_state_table board_cpu_state_tbl;

/* only ATOM core can support L2 CAT officially */
struct platform_clos_info platform_l2_clos_array[4] = {
	{
		.clos_mask = 0xf0,
		.msr_index = MSR_IA32_L2_MASK_0,
	},
	{
		.clos_mask = 0x0f,
		.msr_index = MSR_IA32_L2_MASK_1,
	},
	{
		.clos_mask = 0x0e,
		.msr_index = MSR_IA32_L2_MASK_2,
	},
	{
		.clos_mask = 0x01,
		.msr_index = MSR_IA32_L2_MASK_3,
	},
};

/* The big core (SKL, KBL, WHL, etc) can support L3 CAT only  */
struct platform_clos_info platform_l3_way8_clos_array[4] = {
	{
		.clos_mask = 0xf0,
		.msr_index = MSR_IA32_L3_MASK_0,
	},
	{
		.clos_mask = 0x0f,
		.msr_index = MSR_IA32_L3_MASK_1,
	},
	{
		.clos_mask = 0x0e,
		.msr_index = MSR_IA32_L3_MASK_2,
	},
	{
		.clos_mask = 0x01,
		.msr_index = MSR_IA32_L3_MASK_3,
	},
};

struct platform_clos_info platform_l3_way12_clos_array[4] = {
	{
		.clos_mask = 0xfc0,
		.msr_index = MSR_IA32_L3_MASK_0,
	},
	{
		.clos_mask = 0x03f,
		.msr_index = MSR_IA32_L3_MASK_1,
	},
	{
		.clos_mask = 0x03e,
		.msr_index = MSR_IA32_L3_MASK_2,
	},
	{
		.clos_mask = 0x001,
		.msr_index = MSR_IA32_L3_MASK_3,
	},
};

struct platform_clos_info platform_l3_way16_clos_array[4] = {
	{
		.clos_mask = 0xff00,
		.msr_index = MSR_IA32_L3_MASK_0,
	},
	{
		.clos_mask = 0x00ff,
		.msr_index = MSR_IA32_L3_MASK_1,
	},
	{
		.clos_mask = 0x00fe,
		.msr_index = MSR_IA32_L3_MASK_2,
	},
	{
		.clos_mask = 0x0001,
		.msr_index = MSR_IA32_L3_MASK_3,
	},
};

uint16_t platform_l2_clos_num = (uint16_t)(sizeof(platform_l2_clos_array)/sizeof(struct platform_clos_info));
uint16_t platform_l3_way8_clos_num = (uint16_t)(sizeof(platform_l3_way8_clos_array)/sizeof(struct platform_clos_info));
uint16_t platform_l3_way12_clos_num = (uint16_t)(sizeof(platform_l3_way12_clos_array)/sizeof(struct platform_clos_info));
uint16_t platform_l3_way16_clos_num = (uint16_t)(sizeof(platform_l3_way16_clos_array)/sizeof(struct platform_clos_info));
