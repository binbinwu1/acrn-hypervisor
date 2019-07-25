/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <board.h>
#include <vtd.h>
#include <msr.h>

#ifndef CONFIG_ACPI_PARSE_ENABLED
#error "DMAR info is not available, please set ACPI_PARSE_ENABLED to y in Kconfig. \
	Or use acrn-config tool to generate platform DMAR info."
#endif

struct dmar_info plat_dmar_info;
struct platform_clos_info platform_clos_array[MAX_PLATFORM_CLOS_NUM];
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

