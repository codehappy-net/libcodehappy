/***

	asm.cpp

	x86/x86-64 specific asm routines, providing access to low-level hardware features,
	etc.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifdef CODEHAPPY_NATIVE

#define	ASM_ENDL			"\n\t"

bool hardware_rng_supported(void) {
	u32 o;

#if defined(__x86_64)
	asm volatile (
		"movl	$1, %%eax"				ASM_ENDL
		"cpuid"							ASM_ENDL
		"xorl	%%eax, %%eax"			ASM_ENDL
		"btl	$30, %%ecx" 			ASM_ENDL
		"rcll	$1, %%eax"				ASM_ENDL	/* return value in eax */
		"jmp	LDone%="					ASM_ENDL
		"subl	%%eax, %%eax"			ASM_ENDL
		"LDone%=: "
		"movl	%%eax, %[Output]"		ASM_ENDL
		: [Output] "=r" (o)
		: /* no inputs */
		: "eax", "ebx", "cc"
	);
#else
	// this should work on Intel processors all the way back to 80386
	asm volatile (
		"pushl	%%ebx"					ASM_ENDL
		"pushf"							ASM_ENDL	/* check for 486+ */
		"popl	%%eax"					ASM_ENDL
		"xorl	$0x200000, %%eax"		ASM_ENDL
		"push	%%eax"					ASM_ENDL
		"popf"							ASM_ENDL
		"pushf"							ASM_ENDL
		"popl	%%ebx"					ASM_ENDL
		"xorl	%%eax, %%ebx"			ASM_ENDL
		"testl	$0x200000, %%ebx"		ASM_ENDL
		"jnz	LNoSupport%="				ASM_ENDL
		"xorl	%%eax, %%eax"			ASM_ENDL	/* does CPUID actually work? */
		"cpuid"							ASM_ENDL
		"testl	%%eax, %%eax"			ASM_ENDL
		"jz		LNoSupport%="				ASM_ENDL
		"movl	$1, %%eax"				ASM_ENDL	/* now test for presence of RDRAND isn */
		"cpuid"							ASM_ENDL
		"xorl	%%eax, %%eax"			ASM_ENDL
		"btl	$30, %%ecx"				ASM_ENDL
		"rcll	$1, %%eax"				ASM_ENDL	/* return value in eax */
		"jmp	LDone%="					ASM_ENDL
		"LNoSupport%=: "
		"subl	%%eax, %%eax"			ASM_ENDL
		"LDone%=: "
		"popl	%%ebx"					ASM_ENDL
		"movl	%%eax, %[Output]"		ASM_ENDL
		: [Output] "=r" (o)
		: /* no inputs */
		: "eax", "cc"
	);
#endif

	return(o != 0);
}

/* Returns a random number from hardware source of entropy (on later x86/x86-64 CPUs, this is the RDRAND isn.)
	Call hardware_rng_supported() first to determine if the CPU supports this function. */
u32 hardware_rng(void) {
	u32 o;

	asm volatile (
		"rdrand		%[Output]"			ASM_ENDL
		: [Output] "=r" (o)
		: /* no inputs */
		: /* nothing clobbered besides the output register */
	);

	return(o);
}

/* Returns the value of the hardware counter. */
u64 hardware_counter(void) {
	/*** Supported on Intel chips from Pentium up; any modern x86/x86-64 CPU will have this ***/
	u64 ret;

#if defined(__x86_64)
	// x64
	u32 l32, h32;
	
	asm volatile  (
		"rdtsc;"
		: "=a"(l32), "=d"(h32)
		);

	ret = ((u64)h32 << 32) + (u64)l32;
#else
	// x86
	asm volatile (
		".byte 0x0F, 0x31"
		: "=A" (ret)
	);
#endif

	return ret;
}

/*** Read the value of cr0; does not require kernel-level privileges. ***/
u32 read_cr0_register(void) {
	u32 ret;

#if defined(__x86_64)
		asm (
			"smsw %%eax"	ASM_ENDL
			: "=A" (ret)
		);
#else
	ret = 0UL;
#endif
	return(ret);
}

/*** Cause an invalid op code exception. ***/
void invalid_opcode_exception(void) {
	asm (
		"ud2"			ASM_ENDL	/* defined to be an undefined instruction -- funny architecture, x86 is */
	);
}

#else  // !CODEHAPPY_NATIVE

bool hardware_rng_supported(void) {
	return false;
}

u32 hardware_rng(void) {
	return RandU32();
}
#endif  // CODEHAPPY_NATIVE

void barf() {
	// deliberately segfault
	*((volatile int*)(0x00)) = 0xDEADC0DE;
}
/* end asm.cpp */
