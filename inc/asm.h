/***

	asm.h

	x86/x86-64 specific asm routines, providing access to low-level hardware features,
	etc.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __ASM_H__
#define __ASM_H__

extern bool hardware_rng_supported(void);
extern u32 hardware_rng(void);

#ifdef CODEHAPPY_NATIVE

extern u64 hardware_counter(void);
extern u32 read_cr0_register(void);
extern void invalid_opcode_exception(void);

#endif  // CODEHAPPY_NATIVE

#endif  // __ASM_H__
