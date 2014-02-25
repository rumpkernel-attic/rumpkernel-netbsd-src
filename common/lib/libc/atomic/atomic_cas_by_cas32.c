/*	$NetBSD: atomic_cas_by_cas32.c,v 1.1 2014/02/24 16:15:43 martin Exp $	*/

/*-
 * Copyright (c) 2014 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/inttypes.h>

uint32_t _atomic_cas_32(volatile uint32_t *addr, uint32_t old, uint32_t new);
uint16_t _atomic_cas_16(volatile uint16_t *addr, uint16_t old, uint16_t new);
uint8_t _atomic_cas_8(volatile uint8_t *addr, uint8_t old, uint8_t new);

/*
 * This file provides emulation of 8 bit and 16 bit CAS operations based on
 * 32 bit CAS.
 */
uint16_t
_atomic_cas_16(volatile uint16_t *addr, uint16_t old, uint16_t new)
{
	const uintptr_t base = (uintptr_t)addr & ~3;
	const size_t off = (uintptr_t)addr - base;
	volatile uint32_t * ptr = (volatile uint32_t *)base;
	const size_t shift = off*8;
	const uint32_t mask = 0x0ffff << shift;
	uint32_t old32, new32;

	do {
		old32 = *ptr;
		new32 = (old32 & ~mask) | (uint32_t)new << shift;
		old32 = (old32 & ~mask) | (uint32_t)old << shift;
	} while (_atomic_cas_32(ptr, old32, new32) != old32);
	return (old >> shift) & mask;
}

uint8_t
_atomic_cas_8(volatile uint8_t *addr, uint8_t old, uint8_t new)
{
	const uintptr_t base = (uintptr_t)addr & ~3;
	const size_t off = (uintptr_t)addr - base;
	volatile uint32_t * ptr = (volatile uint32_t *)base;
	const size_t shift = off*8;
	const uint32_t mask = 0x0ff << shift;
	uint32_t old32, new32;

	do {
		old32 = *ptr;
		new32 = (old32 & ~mask) | (uint32_t)new << shift;
		old32 = (old32 & ~mask) | (uint32_t)old << shift;
	} while (_atomic_cas_32(ptr, old32, new32) != old32);
	return (old >> shift) & mask;
}