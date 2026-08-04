//**************************************************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: AbiFloorCompat_linux.cpp
//
// Purpose:
//  RHEL 9 ABI floor compatibility shims for Linux language extensions.
//
//  The extensions are built on Ubuntu 24.04 (glibc 2.38, GCC 13) but must load
//  on RHEL 9 (glibc 2.34). A small number of symbols above that floor get pulled
//  in by the toolchain rather than by our own code, so they cannot be removed at
//  the call site:
//
//    __isoc23_strtol   @GLIBC_2.38  glibc 2.38 <stdlib.h> redirects the strtol
//    __isoc23_strtoll  @GLIBC_2.38  family to C23 variants under a single
//    __isoc23_strtoull @GLIBC_2.38  __GLIBC_USE (C2X_STRTOL) guard, so all four
//    __isoc23_strtoul  @GLIBC_2.38  are redirected together; the references come
//                                   from vendored headers and from libstdc++.a
//    arc4random        @GLIBC_2.36  used by libstdc++.a
//
//  _dl_find_object @GLIBC_2.35 is deliberately NOT shimmed. It is referenced only
//  by the libgcc.a unwinder, which is only linked in when -static-libgcc is used.
//  A stub cannot answer that query: GCC 13's _Unwind_Find_FDE #else's out the
//  dl_iterate_phdr fallback when it is built against _dl_find_object, so a stub
//  return makes it yield NULL for every PC and the first throw terminates. The
//  extensions therefore link libgcc dynamically instead.
//
//  Each shim is defined under a distinct C++ name carrying an __asm__ label, so
//  the emitted symbol is the glibc name. Defining a function literally named
//  __isoc23_strtoll does NOT work: <stdlib.h> has already declared strtoll with
//  that same assembler name, and the resulting conflict leaves the reference
//  unresolved (observed as "UND __isoc23_strtoll@GLIBC_2.38" in .dynsym).
//
//  Every shim is hidden. They satisfy references inside this shared object only
//  and are not exported, so this library never interposes on glibc for the rest
//  of the host process.
//
//  Gated by build/linux/validate-native-elf.sh and build/linux/validate-python-elf.sh, which
//  live in the Data-SQL-Language-Extensions superproject that consumes this repository as a
//  submodule. NOTE: those scripts are added by the companion DSLE branch and are NOT on that
//  repository's master yet - if this merges first, the shims ship ungated. Neither script is
//  runnable from this repository, which has no CI of its own.
//
//  Their limit is worth stating: they compare symbol VERSIONS only. They cannot detect a symbol
//  that resolves to a local definition which is present but wrong - that is exactly how an
//  earlier _dl_find_object stub here passed the gate while breaking exception unwinding.
//**************************************************************************************************

#if defined(__linux__)

// <features.h> (pulled in by any libc header) must be included before testing
// __GLIBC__: the macro is defined by glibc's headers, not by the compiler.
// Guarding on __GLIBC__ before any include silently compiles this whole file to
// nothing, which is exactly how an earlier revision of these shims ended up
// linked but inert.
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

#if defined(__GLIBC__)

#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
// For std::ios_base::Init, which is what actually constructs the standard
// streams. See AbiFloorIosBaseLibraryInit below.
#include <ios>
#endif

#define ABI_FLOOR_HIDDEN __attribute__((visibility("hidden")))

extern "C"
{
	// Bind to the classic, always-present glibc entry points. The explicit
	// assembler labels bypass the <stdlib.h> C23 redirect, so these cannot
	// recurse back into the shims below.
	long int AbiFloorClassicStrtol(const char *nptr, char **endptr, int base) __asm__("strtol");
	long long int AbiFloorClassicStrtoll(const char *nptr, char **endptr, int base) __asm__("strtoll");
	unsigned long long int AbiFloorClassicStrtoull(const char *nptr, char **endptr, int base) __asm__("strtoull");
	unsigned long int AbiFloorClassicStrtoul(const char *nptr, char **endptr, int base) __asm__("strtoul");

	ABI_FLOOR_HIDDEN long int AbiFloorIsoc23Strtol(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtol");
	ABI_FLOOR_HIDDEN long long int AbiFloorIsoc23Strtoll(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoll");
	ABI_FLOOR_HIDDEN unsigned long long int AbiFloorIsoc23Strtoull(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoull");
	ABI_FLOOR_HIDDEN unsigned long int AbiFloorIsoc23Strtoul(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoul");
	ABI_FLOOR_HIDDEN uint32_t AbiFloorArc4Random(void) __asm__("arc4random");

	// The only behavioural difference between the C23 and classic strtol forms
	// is that the C23 forms also accept a "0b" binary prefix when base is 0 or
	// 2. No caller in these extensions parses binary literals.
	ABI_FLOOR_HIDDEN long int AbiFloorIsoc23Strtol(const char *nptr, char **endptr, int base)
	{
		return AbiFloorClassicStrtol(nptr, endptr, base);
	}

	ABI_FLOOR_HIDDEN long long int AbiFloorIsoc23Strtoll(const char *nptr, char **endptr, int base)
	{
		return AbiFloorClassicStrtoll(nptr, endptr, base);
	}

	ABI_FLOOR_HIDDEN unsigned long long int AbiFloorIsoc23Strtoull(const char *nptr, char **endptr, int base)
	{
		return AbiFloorClassicStrtoull(nptr, endptr, base);
	}

	ABI_FLOOR_HIDDEN unsigned long int AbiFloorIsoc23Strtoul(const char *nptr, char **endptr, int base)
	{
		return AbiFloorClassicStrtoul(nptr, endptr, base);
	}

	// arc4random is a cryptographically secure generator, so this must not
	// degrade to a weak source. getrandom(2) is the kernel CSPRNG and has been
	// available since glibc 2.25, well under the RHEL 9 floor. /dev/urandom is
	// the fallback for seccomp-restricted environments. If neither can produce
	// bytes we abort rather than return predictable output.
	ABI_FLOOR_HIDDEN uint32_t AbiFloorArc4Random(void)
	{
		// glibc's arc4random uses __getrandom_nocancel and never disturbs errno.
		// This shim binds ahead of it for everything in this shared object,
		// including std::random_device, so it must not leak errno from its own
		// syscalls to a caller that is about to inspect it.
		const int savedErrno = errno;

		uint32_t value = 0;
		unsigned char *out = reinterpret_cast<unsigned char *>(&value);
		size_t filled = 0;

		// Captured at each terminal break. Reading errno at the abort site instead would
		// report something unrelated: the received==0 seccomp case sets no errno at all,
		// a failing open() masks a preceding read() failure, and close() runs in between.
		//
		int failErrno = 0;
		const char *failStage = "none";

		while (filled < sizeof(value))
		{
			// A zero return makes no progress and would re-enter the same call
			// forever (a seccomp SECCOMP_RET_ERRNO(0) filter can produce it), so
			// it is treated as terminal exactly like the /dev/urandom loop below.
			ssize_t received = getrandom(out + filled, sizeof(value) - filled, 0);
			if (received <= 0)
			{
				if (received < 0 && errno == EINTR)
				{
					continue;
				}
				failStage = (received == 0) ? "getrandom returned 0" : "getrandom";
				failErrno = (received == 0) ? 0 : errno;
				break;
			}
			filled += static_cast<size_t>(received);
		}

		if (filled < sizeof(value))
		{
			int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
			if (fd < 0)
			{
				failStage = "open(/dev/urandom)";
				failErrno = errno;
			}
			else
			{
				while (filled < sizeof(value))
				{
					ssize_t received = read(fd, out + filled, sizeof(value) - filled);
					if (received <= 0)
					{
						if (received < 0 && errno == EINTR)
						{
							continue;
						}
						failStage = (received == 0) ? "read(/dev/urandom) returned 0" : "read(/dev/urandom)";
						failErrno = (received == 0) ? 0 : errno;
						break;
					}
					filled += static_cast<size_t>(received);
				}
				close(fd);
			}
		}

		if (filled < sizeof(value))
		{
			// Aborting is correct for a CSPRNG - returning predictable output is worse - but a
			// bare abort() leaves a DBA with nothing but SIGABRT.
			//
			// snprintf into a local buffer + write(2) rather than fprintf: write takes no FILE
			// lock, so a concurrent writer wedged inside stdio cannot turn this diagnosable
			// crash into a hang, and it needs no fflush. (write(2) is async-signal-safe;
			// snprintf is not - the reason here is the lock, not AS-safety.)
			//
			char msg[256];
			int n = snprintf(msg, sizeof(msg),
				"AbiFloorCompat: arc4random could not obtain %zu bytes of entropy "
				"(got %zu; failed at %s, errno %d). Aborting rather than returning "
				"predictable output.\n",
				sizeof(value), filled, failStage, failErrno);
			if (n > 0)
			{
				size_t remaining = (static_cast<size_t>(n) < sizeof(msg)) ? static_cast<size_t>(n) : sizeof(msg) - 1;
				const char *p = msg;
				while (remaining > 0)
				{
					ssize_t written = write(2, p, remaining);
					if (written <= 0)
					{
						if (written < 0 && errno == EINTR)
						{
							continue;
						}
						break;
					}
					p += written;
					remaining -= static_cast<size_t>(written);
				}
			}
			abort();
		}

		errno = savedErrno;
		return value;
	}

	// std::ios_base_library_init() is GLIBCXX_3.4.32, i.e. GCC 13's libstdc++.
	// GCC 13 emits a reference to it from every translation unit that includes
	// <iostream>, and RHEL9's libstdc++ 3.4.29 does not provide it.
	//
	// This is opt-in via ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT because it must
	// only be defined when libstdc++ is linked *dynamically*. libstdc++.a already
	// contains this symbol in ios_init.o, so defining it alongside a static
	// libstdc++ fails the link with "multiple definition of
	// std::ios_base_library_init()".
	//
	// The R and Python extensions are the cases that need it: neither can link
	// libstdc++ statically (libR.so already links libstdc++.so.6, and numpy's and
	// pandas' C extensions do too - two C++ runtimes in one process abort at
	// runtime), yet the include is unavoidable because it arrives through the
	// Rcpp/RInside and Boost.Python headers.
	//
	// WHAT GCC 13 ACTUALLY DOES - verified against the built binary, because an earlier
	// version of this comment asserted the opposite and was wrong:
	//
	// GCC 13's <iostream> does NOT call this function. It emits the name via a `.globl`
	// asm directive with no call site and no relocation, purely as a link-time SENTINEL:
	// the resulting undefined symbol forces a GLIBCXX_3.4.32 VERNEED entry so the object
	// REFUSES TO LOAD against a libstdc++ that would leave the standard streams
	// unconstructed. In libstdc++ 13 the symbol is not even a real function - ios_init.cc
	// aliases it to _ZNSt8ios_base4InitC1Ev - and the actual initialisation moved into the
	// library via init_priority(90) in ios_base_init.h.
	//
	// So DEFINING the symbol here satisfies the sentinel and REMOVES the version
	// dependency. Measured on the shipped libPythonExtension.so.1.2: GLIBCXX_3.4.32 is
	// absent from .gnu.version_r and MAX GLIBCXX is 3.4.29. That is the intended outcome
	// for loading on RHEL 9 - but it means a function-local static inside this function
	// would NEVER be constructed, because the function is never called. Relying on that
	// would trade a deterministic load failure for silently unconstructed streams.
	//
	// Hence two separate pieces below:
	//   1. the sentinel definition, which exists only to satisfy the link; and
	//   2. a NAMESPACE-SCOPE std::ios_base::Init, whose dynamic initialiser runs at
	//      dlopen exactly as the pre-13 per-TU __ioinit did. Its constructor is
	//      refcounted and idempotent, and std::ios_base::Init::Init is GLIBCXX_3.4 -
	//      present in every libstdc++ we target, so it does not raise the ABI floor.
#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
	ABI_FLOOR_HIDDEN void AbiFloorIosBaseLibraryInit(void) __asm__("_ZSt21ios_base_library_initv");
	ABI_FLOOR_HIDDEN void AbiFloorIosBaseLibraryInit(void)
	{
		// Deliberately empty. This is a link-time sentinel: GCC 13 emits the name but
		// never calls it. The real initialisation is the namespace-scope object below.
	}
#endif
}

#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
// This is what actually constructs std::cin/cout/cerr/clog on hosts whose libstdc++ is
// older than 3.4.32. Namespace scope, so it participates in this object's dynamic
// initialisation and runs when the extension is dlopen'd. That guarantees this object's
// constructor runs without relying on calls to _ZSt21ios_base_library_initv.
//
static std::ios_base::Init g_abiFloorIosBaseInit;
#endif

#else
#error "AbiFloorCompat_linux.cpp is compiled for Linux but __GLIBC__ is not defined; the shims would be silently empty."
#endif // __GLIBC__
#endif // __linux__
