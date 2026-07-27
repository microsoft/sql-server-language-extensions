//**************************************************************************************************
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
//    __isoc23_strtoll  @GLIBC_2.38  glibc 2.38 <stdlib.h> redirects the strtol
//    __isoc23_strtoull @GLIBC_2.38  family to C23 variants; the references come
//    __isoc23_strtoul  @GLIBC_2.38  from vendored headers and from libstdc++.a
//    arc4random        @GLIBC_2.36  used by libstdc++.a
//    _dl_find_object   @GLIBC_2.35  used by the libgcc.a unwinder
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
//  Enforced by build/linux/validate-native-elf.sh (MAX GLIBC <= 2.34).
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
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>

#if defined(__GLIBC__)

#define ABI_FLOOR_HIDDEN __attribute__((visibility("hidden")))

extern "C"
{
	// Bind to the classic, always-present glibc entry points. The explicit
	// assembler labels bypass the <stdlib.h> C23 redirect, so these cannot
	// recurse back into the shims below.
	long long int AbiFloorClassicStrtoll(const char *nptr, char **endptr, int base) __asm__("strtoll");
	unsigned long long int AbiFloorClassicStrtoull(const char *nptr, char **endptr, int base) __asm__("strtoull");
	unsigned long int AbiFloorClassicStrtoul(const char *nptr, char **endptr, int base) __asm__("strtoul");

	ABI_FLOOR_HIDDEN long long int AbiFloorIsoc23Strtoll(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoll");
	ABI_FLOOR_HIDDEN unsigned long long int AbiFloorIsoc23Strtoull(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoull");
	ABI_FLOOR_HIDDEN unsigned long int AbiFloorIsoc23Strtoul(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoul");
	ABI_FLOOR_HIDDEN uint32_t AbiFloorArc4Random(void) __asm__("arc4random");
	ABI_FLOOR_HIDDEN int AbiFloorDlFindObject(void *address, void *result) __asm__("_dl_find_object");

	// The only behavioural difference between the C23 and classic strtol forms
	// is that the C23 forms also accept a "0b" binary prefix when base is 0 or
	// 2. No caller in these extensions parses binary literals.
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
		uint32_t value = 0;
		unsigned char *out = reinterpret_cast<unsigned char *>(&value);
		size_t filled = 0;

		while (filled < sizeof(value))
		{
			ssize_t received = getrandom(out + filled, sizeof(value) - filled, 0);
			if (received < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				break;
			}
			filled += static_cast<size_t>(received);
		}

		if (filled < sizeof(value))
		{
			int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
			if (fd >= 0)
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
						break;
					}
					filled += static_cast<size_t>(received);
				}
				close(fd);
			}
		}

		if (filled < sizeof(value))
		{
			abort();
		}

		return value;
	}

	// _dl_find_object is the glibc 2.35+ fast path the libgcc unwinder uses to
	// locate exception-handling frames. A non-zero return means "no information
	// available", which makes libgcc fall back to its dl_iterate_phdr path. That
	// fallback is the same code path used on every glibc older than 2.35, so
	// unwinding stays correct - only marginally slower on the throw path.
	ABI_FLOOR_HIDDEN int AbiFloorDlFindObject(void *address, void *result)
	{
		(void)address;
		(void)result;
		return -1;
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
	// The R extension is the case that needs it: it cannot link libstdc++
	// statically (libR.so already links libstdc++.so.6, and two C++ runtimes in
	// one process abort at runtime), yet the include is unavoidable because it
	// arrives through the Rcpp and RInside headers.
	//
	// Defining it as a no-op is safe here: the symbol only drives iostream
	// initialisation ordering, the standard streams are still initialised by
	// libstdc++'s own machinery, and none of the shipped sources use
	// std::cout/std::cerr any more (they log through stdio). The definition is
	// hidden, so it satisfies our own translation units without changing
	// iostream initialisation for libR or anything else in the host process.
#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
	ABI_FLOOR_HIDDEN void AbiFloorIosBaseLibraryInit(void) __asm__("_ZSt21ios_base_library_initv");
	ABI_FLOOR_HIDDEN void AbiFloorIosBaseLibraryInit(void)
	{
	}
#endif
}

#else
#error "AbiFloorCompat_linux.cpp is compiled for Linux but __GLIBC__ is not defined; the shims would be silently empty."
#endif // __GLIBC__
#endif // __linux__
