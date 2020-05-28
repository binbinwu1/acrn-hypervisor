/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <vm_config.h>
#include <pci_devices.h>
#include <vpci.h>
#include <mmu.h>
#include <page.h>

/* The vbar_base info of pt devices is included in device MACROs which defined in
 *           arch/x86/configs/$(CONFIG_BOARD)/pci_devices.h.
 * The memory range of vBAR should exactly match with the e820 layout of VM.
 */
