/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VM_CONFIGURATIONS_H
#define VM_CONFIGURATIONS_H

#include <misc_cfg.h>

/* Bits mask of guest flags that can be programmed by device model. Other bits are set by hypervisor only */
#define DM_OWNED_GUEST_FLAG_MASK	(GUEST_FLAG_SECURE_WORLD_ENABLED | GUEST_FLAG_LAPIC_PASSTHROUGH | \
						GUEST_FLAG_RT | GUEST_FLAG_IO_COMPLETION_POLLING)

#define PRE_VM_NUM		0U
#define SOS_VM_NUM		1U
#define MAX_POST_VM_NUM		2U
#define CONFIG_MAX_KATA_VM_NUM		0U

#define VM1_CONFIG_CPU_AFFINITY		(AFFINITY_CPU(1U))
#define VM1_VCPU_CLOS		{0U}
#define VM2_CONFIG_CPU_AFFINITY		(AFFINITY_CPU(2U) | AFFINITY_CPU(3U))
#define VM2_VCPU_CLOS		{0U}

#define SOS_VM_BOOTARGS			SOS_ROOTFS	\
					SOS_CONSOLE	\
					SOS_BOOTARGS_DIFF

#endif /* VM_CONFIGURATIONS_H */
