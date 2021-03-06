/* $NetBSD: hdaudio_ids.c,v 1.8 2014/04/07 21:35:41 njoly Exp $ */

/*
 * Copyright (c) 2010 Jared D. McNeill <jmcneill@invisible.ca>
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Precedence Technologies Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: hdaudio_ids.c,v 1.8 2014/04/07 21:35:41 njoly Exp $");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>

#include "hdaudio_ids.h"

static const struct hdaudio_id {
	uint16_t vendor;
	uint16_t product;
	const char *name;
} hdaudio_ids[] = {
	/* ATI */
	{ HDA_VENDOR_ATI, 0x7919, "RS600 HDMI" },
	{ HDA_VENDOR_ATI, 0x793c, "RS600 HDMI" },
	{ HDA_VENDOR_ATI, 0x791a, "RS690/780 HDMI" },
	{ HDA_VENDOR_ATI, 0xaa01, "R6xx HDMI" },
	{ HDA_VENDOR_ATI, HDA_PRODUCT_ANY, "ATI" },
	/* NVIDIA */
	{ HDA_VENDOR_NVIDIA, 0x0002, "MCP77/78 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x0003, "MCP77/78 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x0005, "MCP77/78 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x0006, "MCP77/78 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x0007, "MCP79/7A HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x000a, "GT220 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x000b, "GT21x HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x000c, "MCP89 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x000d, "GT240 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x0015, "GT5xx HDMI/DP" }, /* ??? GTX 550 Ti */
	{ HDA_VENDOR_NVIDIA, 0x0067, "MCP67 HDMI" },
	{ HDA_VENDOR_NVIDIA, 0x8001, "MCP73 HDMI" },
	{ HDA_VENDOR_NVIDIA, HDA_PRODUCT_ANY, "NVIDIA" },
	/* Realtek */
	{ HDA_VENDOR_REALTEK, 0x0260, "ALC260" },
	{ HDA_VENDOR_REALTEK, 0x0262, "ALC262" },
	{ HDA_VENDOR_REALTEK, 0x0267, "ALC267" },
	{ HDA_VENDOR_REALTEK, 0x0268, "ALC268" },
	{ HDA_VENDOR_REALTEK, 0x0269, "ALC269" },
	{ HDA_VENDOR_REALTEK, 0x0270, "ALC270" },
	{ HDA_VENDOR_REALTEK, 0x0272, "ALC272" },
	{ HDA_VENDOR_REALTEK, 0x0275, "ALC275" },
	{ HDA_VENDOR_REALTEK, 0x0660, "ALC660-VD" },
	{ HDA_VENDOR_REALTEK, 0x0662, "ALC662" },
	{ HDA_VENDOR_REALTEK, 0x0663, "ALC663" },
	{ HDA_VENDOR_REALTEK, 0x0670, "ALC670" },
	{ HDA_VENDOR_REALTEK, 0x0861, "ALC861" },
	{ HDA_VENDOR_REALTEK, 0x0862, "ALC861-VD" },
	{ HDA_VENDOR_REALTEK, 0x0880, "ALC880" },
	{ HDA_VENDOR_REALTEK, 0x0882, "ALC882" },
	{ HDA_VENDOR_REALTEK, 0x0883, "ALC883" },
	{ HDA_VENDOR_REALTEK, 0x0885, "ALC885" },
	{ HDA_VENDOR_REALTEK, 0x0887, "ALC887" },
	{ HDA_VENDOR_REALTEK, 0x0888, "ALC888" },
	{ HDA_VENDOR_REALTEK, 0x0889, "ALC889" },
	{ HDA_VENDOR_REALTEK, 0x0892, "ALC892" },
	{ HDA_VENDOR_REALTEK, HDA_PRODUCT_ANY, "Realtek" },
	/* VIA */
	{ HDA_VENDOR_VIA, 0x1708, "VT1708" },
	{ HDA_VENDOR_VIA, 0x1709, "VT1708" },
	{ HDA_VENDOR_VIA, 0x170a, "VT1708" },
	{ HDA_VENDOR_VIA, 0x170b, "VT1708" },
	{ HDA_VENDOR_VIA, 0xe710, "VT1709 10ch" },
	{ HDA_VENDOR_VIA, 0xe711, "VT1709 10ch" },
	{ HDA_VENDOR_VIA, 0xe712, "VT1709 10ch" },
	{ HDA_VENDOR_VIA, 0xe713, "VT1709 10ch" },
	{ HDA_VENDOR_VIA, 0xe714, "VT1709 6ch" },
	{ HDA_VENDOR_VIA, 0xe715, "VT1709 6ch" },
	{ HDA_VENDOR_VIA, 0xe716, "VT1709 6ch" },
	{ HDA_VENDOR_VIA, 0xe717, "VT1709 6ch" },
	{ HDA_VENDOR_VIA, 0xe720, "VT1708B 8ch" },
	{ HDA_VENDOR_VIA, 0xe721, "VT1708B 8ch" },
	{ HDA_VENDOR_VIA, 0xe722, "VT1708B 8ch" },
	{ HDA_VENDOR_VIA, 0xe723, "VT1708B 8ch" },
	{ HDA_VENDOR_VIA, 0xe724, "VT1708B 4ch" },
	{ HDA_VENDOR_VIA, 0xe725, "VT1708B 4ch" },
	{ HDA_VENDOR_VIA, 0xe726, "VT1708B 4ch" },
	{ HDA_VENDOR_VIA, 0xe727, "VT1708B 4ch" },
	{ HDA_VENDOR_VIA, 0x0397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x1397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x2397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x3397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x4397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x5397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x6397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x7397, "VT1708S" },
	{ HDA_VENDOR_VIA, 0x0398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x1398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x2398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x3398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x4398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x5398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x6398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x7398, "VT1702" },
	{ HDA_VENDOR_VIA, 0x0428, "VT1718S" },
	{ HDA_VENDOR_VIA, 0x4428, "VT1718S" },
	{ HDA_VENDOR_VIA, 0x0441, "VT2020" },
	{ HDA_VENDOR_VIA, 0x4441, "VT1828S" },
	{ HDA_VENDOR_VIA, 0x0433, "VT1716S" },
	{ HDA_VENDOR_VIA, 0xa721, "VT1716S" },
	{ HDA_VENDOR_VIA, 0x0438, "VT2002P" },
	{ HDA_VENDOR_VIA, 0x4438, "VT2002P" },
	{ HDA_VENDOR_VIA, 0x0448, "VT1812" },
	{ HDA_VENDOR_VIA, 0x0440, "VT1818S" },
	{ HDA_VENDOR_VIA, 0x4760, "VT1705" },
	{ HDA_VENDOR_VIA, HDA_PRODUCT_ANY, "VIA" },
	/* Analog Devices */
	{ HDA_VENDOR_ANALOG_DEVICES, 0x184a, "AD1884A" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1882, "AD1882" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1883, "AD1883" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1884, "AD1884" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x194a, "AD1984A" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x194b, "AD1984B" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1981, "AD1981HD" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1983, "AD1983" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1984, "AD1984" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1986, "AD1986A" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x1988, "AD1988A" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x198b, "AD1988B" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x989a, "AD1989A" },
	{ HDA_VENDOR_ANALOG_DEVICES, 0x989b, "AD1989B" },
	{ HDA_VENDOR_ANALOG_DEVICES, HDA_PRODUCT_ANY, "ADI" },
	/* Conexant */
	{ HDA_VENDOR_CONEXANT, 0x5045, "CX20549" },
	{ HDA_VENDOR_CONEXANT, 0x5047, "CX20551" },
	{ HDA_VENDOR_CONEXANT, 0x5051, "CX20561" },
	{ HDA_VENDOR_CONEXANT, 0x5066, "CX20582" },
	{ HDA_VENDOR_CONEXANT, 0x5067, "CX20583" },
	{ HDA_VENDOR_CONEXANT, 0x5069, "CX20585" },
	{ HDA_VENDOR_CONEXANT, 0x506e, "CX20671" },
	{ HDA_VENDOR_CONEXANT, HDA_PRODUCT_ANY, "Conexant" },
	/* CMedia */
	{ HDA_VENDOR_CMEDIA, 0x4980, "CMI9880" },
	{ HDA_VENDOR_CMEDIA, HDA_PRODUCT_ANY, "CMedia" },
	/* Intel */
	{ HDA_VENDOR_INTEL, 0x0054, "Q57 HDMI" },
	{ HDA_VENDOR_INTEL, 0x2801, "G45 HDMI" },
	{ HDA_VENDOR_INTEL, 0x2802, "G45 HDMI" },
	{ HDA_VENDOR_INTEL, 0x2803, "G45 HDMI" },
	{ HDA_VENDOR_INTEL, 0x2804, "G45 HDMI" },
	{ HDA_VENDOR_INTEL, 0x29fb, "G45 HDMI" },
	{ HDA_VENDOR_INTEL, HDA_PRODUCT_ANY, "Intel" },
	/* Sigmatel */
	{ HDA_VENDOR_SIGMATEL, 0x7612, "STAC9230X" },
	{ HDA_VENDOR_SIGMATEL, 0x7613, "STAC9230D" },
	{ HDA_VENDOR_SIGMATEL, 0x7614, "STAC9229X" },
	{ HDA_VENDOR_SIGMATEL, 0x7615, "STAC9229D" },
	{ HDA_VENDOR_SIGMATEL, 0x7616, "STAC9228X" },
	{ HDA_VENDOR_SIGMATEL, 0x7617, "STAC9228D" },
	{ HDA_VENDOR_SIGMATEL, 0x7618, "STAC9227X" },
	{ HDA_VENDOR_SIGMATEL, 0x7619, "STAC9227D" },
	{ HDA_VENDOR_SIGMATEL, 0x7620, "STAC9274" },
	{ HDA_VENDOR_SIGMATEL, 0x7621, "STAC9274D" },
	{ HDA_VENDOR_SIGMATEL, 0x7622, "STAC9273X" },
	{ HDA_VENDOR_SIGMATEL, 0x7623, "STAC9273D" },
	{ HDA_VENDOR_SIGMATEL, 0x7624, "STAC9272X" },
	{ HDA_VENDOR_SIGMATEL, 0x7625, "STAC9272D" },
	{ HDA_VENDOR_SIGMATEL, 0x7626, "STAC9271X" },
	{ HDA_VENDOR_SIGMATEL, 0x7627, "STAC9271D" },
	{ HDA_VENDOR_SIGMATEL, 0x7628, "STAC9274X5NH" },
	{ HDA_VENDOR_SIGMATEL, 0x7629, "STAC9274D5NH" },
	{ HDA_VENDOR_SIGMATEL, 0x7632, "STAC9202" },
	{ HDA_VENDOR_SIGMATEL, 0x7633, "STAC9202D" },
	{ HDA_VENDOR_SIGMATEL, 0x7634, "STAC9250" },
	{ HDA_VENDOR_SIGMATEL, 0x7635, "STAC9250D" },
	{ HDA_VENDOR_SIGMATEL, 0x7636, "STAC9251" },
	{ HDA_VENDOR_SIGMATEL, 0x7637, "STAC9250D" },
	{ HDA_VENDOR_SIGMATEL, 0x7645, "92HD206X" },
	{ HDA_VENDOR_SIGMATEL, 0x7646, "92HD206D" },
	{ HDA_VENDOR_SIGMATEL, 0x7661, "CXD9872RD/K" },
	{ HDA_VENDOR_SIGMATEL, 0x7662, "STAC9872AK" },
	{ HDA_VENDOR_SIGMATEL, 0x7664, "CXD9872AKD" },
	{ HDA_VENDOR_SIGMATEL, 0x7680, "STAC9221 A1" },
	{ HDA_VENDOR_SIGMATEL, 0x7681, "STAC9220D" }, /* also 9223D A2 */
	{ HDA_VENDOR_SIGMATEL, 0x7682, "STAC9221 A2" },
	{ HDA_VENDOR_SIGMATEL, 0x7683, "STAC9221D" },
	{ HDA_VENDOR_SIGMATEL, 0x7690, "STAC9200" },
	{ HDA_VENDOR_SIGMATEL, 0x7691, "STAC9200D" },
	{ HDA_VENDOR_SIGMATEL, 0x7698, "STAC9205" },
	{ HDA_VENDOR_SIGMATEL, 0x76a0, "STAC9205" },
	{ HDA_VENDOR_SIGMATEL, 0x76a1, "STAC9205D" },
	{ HDA_VENDOR_SIGMATEL, 0x76a2, "STAC9204" },
	{ HDA_VENDOR_SIGMATEL, 0x76a3, "STAC9204D" },
	{ HDA_VENDOR_SIGMATEL, 0x76a4, "STAC9255" },
	{ HDA_VENDOR_SIGMATEL, 0x76a5, "STAC9255D" },
	{ HDA_VENDOR_SIGMATEL, 0x76a6, "STAC9254" },
	{ HDA_VENDOR_SIGMATEL, 0x76a7, "STAC9254D" },
	{ HDA_VENDOR_SIGMATEL, 0x7880, "STAC9220 A2" },
	{ HDA_VENDOR_SIGMATEL, 0x7882, "STAC9220 A1" },
	{ HDA_VENDOR_SIGMATEL, HDA_PRODUCT_ANY, "Sigmatel" },
	/* Sigmatel (alternate vendor ID) */
	{ HDA_VENDOR_SIGMATEL2, 0x7603, "92HD75B3X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x7604, "92HD83C1X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x7605, "92HD81B1X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x7608, "92HD75B2X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x7666, "92HD88B3" },
	{ HDA_VENDOR_SIGMATEL2, 0x7667, "92HD88B1" },
	{ HDA_VENDOR_SIGMATEL2, 0x7668, "92HD88B2" },
	{ HDA_VENDOR_SIGMATEL2, 0x7669, "92HD88B4" },
	{ HDA_VENDOR_SIGMATEL2, 0x7674, "92HD73D1X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x7675, "92HD73C1X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x7676, "92HD73E1X5" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b0, "92HD71B8X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b1, "92HD71B8X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b2, "92HD71B7X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b3, "92HD71B7X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b4, "92HD71B6X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b5, "92HD71B6X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b6, "92HD71B5X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76b7, "92HD71B5X" },
	{ HDA_VENDOR_SIGMATEL2, 0x76d4, "92HD83C1C5" },
	{ HDA_VENDOR_SIGMATEL2, 0x76d5, "92HD81B1C5" },
	{ HDA_VENDOR_SIGMATEL2, HDA_PRODUCT_ANY, "Sigmatel" },
	/* VMware */
	{ HDA_VENDOR_VMWARE, 0x1975, "Virtual HDA" },
	{ HDA_VENDOR_VMWARE, HDA_PRODUCT_ANY, "VMware" },
};

void
hdaudio_id2name(uint16_t vendor, uint16_t product, char *buf, size_t len)
{
	const char *name = NULL;
	int i;

	memset(buf, 0, len);
	for (i = 0; i < __arraycount(hdaudio_ids); i++) {
		if (hdaudio_ids[i].vendor != vendor)
			continue;
		if (hdaudio_ids[i].product == product) {
			name = hdaudio_ids[i].name;
			break;
		}
	}

	if (name)
		snprintf(buf, len - 1, "%s", name);
	else if (product == HDA_PRODUCT_ANY)
		snprintf(buf, len - 1, "vendor 0x%04x", vendor);
	else
		snprintf(buf, len - 1, "product 0x%04x", product);
}
