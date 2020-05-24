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

#ifndef PCI_DEVICES_H_
#define PCI_DEVICES_H_

#define PTDEV_HI_MMIO_SIZE                      0x1000d000UL

#define HOST_BRIDGE                             .pbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U}

#define VGA_COMPATIBLE_CONTROLLER_0             .pbdf.bits = {.b = 0x00U, .d = 0x02U, .f = 0x00U}, \
                                                .vbar_base[0] = 0x82000000UL, \
                                                .vbar_base[2] = PTDEV_HI_MMIO_START + 0x0UL

#define SYSTEM_PERIPHERAL_0                     .pbdf.bits = {.b = 0x00U, .d = 0x08U, .f = 0x00U}, \
                                                .vbar_base[0] = 0x8335c000UL

#define SYSTEM_PERIPHERAL_1                     .pbdf.bits = {.b = 0x00U, .d = 0x1DU, .f = 0x00U}, \
                                                .vbar_base[0] = 0x83000000UL

#define SERIAL_BUS_CONTROLLER_0                 .pbdf.bits = {.b = 0x00U, .d = 0x10U, .f = 0x00U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10000000UL

#define SERIAL_BUS_CONTROLLER_1                 .pbdf.bits = {.b = 0x00U, .d = 0x10U, .f = 0x01U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10001000UL

#define SERIAL_BUS_CONTROLLER_2                 .pbdf.bits = {.b = 0x00U, .d = 0x12U, .f = 0x00U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10002000UL

#define SERIAL_BUS_CONTROLLER_3                 .pbdf.bits = {.b = 0x00U, .d = 0x15U, .f = 0x00U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10003000UL

#define SERIAL_BUS_CONTROLLER_4                 .pbdf.bits = {.b = 0x00U, .d = 0x15U, .f = 0x02U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10004000UL

#define SERIAL_BUS_CONTROLLER_5                 .pbdf.bits = {.b = 0x00U, .d = 0x15U, .f = 0x03U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10005000UL

#define SERIAL_BUS_CONTROLLER_6                 .pbdf.bits = {.b = 0x00U, .d = 0x19U, .f = 0x00U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10006000UL

#define SERIAL_BUS_CONTROLLER_7                 .pbdf.bits = {.b = 0x00U, .d = 0x1EU, .f = 0x02U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10008000UL

#define SERIAL_BUS_CONTROLLER_8                 .pbdf.bits = {.b = 0x00U, .d = 0x1FU, .f = 0x05U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x1000c000UL, \
                                                .vbar_base[1] = PTDEV_HI_MMIO_START + 0x80000000UL

#define USB_CONTROLLER_0                        .pbdf.bits = {.b = 0x00U, .d = 0x14U, .f = 0x00U}, \
                                                .vbar_base[0] = 0x83340000UL

#define RAM_MEMORY_0                            .pbdf.bits = {.b = 0x00U, .d = 0x14U, .f = 0x02U}, \
                                                .vbar_base[0] = 0x83350000UL, \
                                                .vbar_base[2] = 0x83360000UL

#define COMMUNICATION_CONTROLLER_0              .pbdf.bits = {.b = 0x00U, .d = 0x16U, .f = 0x00U}, \
                                                .vbar_base[0] = 0x83364000UL

#define COMMUNICATION_CONTROLLER_1              .pbdf.bits = {.b = 0x00U, .d = 0x1EU, .f = 0x00U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x10007000UL

#define SATA_CONTROLLER_0                       .pbdf.bits = {.b = 0x00U, .d = 0x17U, .f = 0x00U}, \
                                                .vbar_base[0] = 0x8335a000UL, \
                                                .vbar_base[1] = 0x8336e000UL, \
                                                .vbar_base[5] = 0x8336d000UL

#define SD_HOST_CONTROLLER_0                    .pbdf.bits = {.b = 0x00U, .d = 0x1AU, .f = 0x00U}, \
                                                .vbar_base[0] = 0x83366000UL

#define SD_HOST_CONTROLLER_1                    .pbdf.bits = {.b = 0x00U, .d = 0x1AU, .f = 0x01U}, \
                                                .vbar_base[0] = 0x83367000UL

#define NON_VGA_UNCLASSIFIED_DEVICE_0           .pbdf.bits = {.b = 0x00U, .d = 0x1AU, .f = 0x03U}, \
                                                .vbar_base[0] = 0x83300000UL

#define ETHERNET_CONTROLLER_0                   .pbdf.bits = {.b = 0x00U, .d = 0x1EU, .f = 0x04U}, \
                                                .vbar_base[0] = PTDEV_HI_MMIO_START + 0x1000a000UL, \
                                                .vbar_base[2] = PTDEV_HI_MMIO_START + 0x8336a000UL

#define ISA_BRIDGE_0                            .pbdf.bits = {.b = 0x00U, .d = 0x1FU, .f = 0x00U}

#define MULTIMEDIA_AUDIO_CONTROLLER_0           .pbdf.bits = {.b = 0x00U, .d = 0x1FU, .f = 0x03U}, \
                                                .vbar_base[0] = 0x83354000UL, \
                                                .vbar_base[4] = 0x83200000UL

#define SMBUS_0                                 .pbdf.bits = {.b = 0x00U, .d = 0x1FU, .f = 0x04U}, \
                                                .vbar_base[0] = 0x8336b000UL

#endif /* PCI_DEVICES_H_ */
