/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef MISC_CFG_H
#define MISC_CFG_H

#define MAX_PCPU_NUM	4U
#define MAX_PLATFORM_CLOS_NUM	16U
#define MAX_VMSIX_ON_MSI_PDEVS_NUM	3U
#define ROOTFS_0		"root=/dev/mmcblk0p3 "
#define ROOTFS_1		"root=/dev/sda3 "
#define ROOTFS_2		"root=/dev/sdb2 "
#define ROOTFS_3		"root=/dev/sdb3 "
#define ROOTFS_4		"root=/dev/sdb4 "

#define SOS_ROOTFS		"root=/dev/sda3 "
#define SOS_CONSOLE		"console=ttyS0 "
#define SOS_COM1_BASE		0x3F8U
#define SOS_COM1_IRQ		3U
#define SOS_COM2_BASE		0x3E8U
#define SOS_COM2_IRQ		4U

#define SOS_BOOTARGS_DIFF	"rw " \
				"rootwait "	\
				"console=tty0 "	\
				"consoleblank=0 "	\
				"no_timer_check "	\
				"quiet "	\
				"loglevel=3 "	\
				"i915.nuclear_pageflip=1 "	\
				"i915.avail_planes_per_pipe=0x01010F "	\
				"i915.domain_plane_owners=0x011111110000 "	\
				"i915.enable_gvt=1"

#define MAX_HIDDEN_PDEVS_NUM	0U

#define HI_MMIO_START		~0UL
#define HI_MMIO_END		0UL

#endif /* MISC_CFG_H */
