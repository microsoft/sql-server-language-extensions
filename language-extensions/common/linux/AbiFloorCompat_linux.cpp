//**************************************************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: AbiFloorCompat_linux.cpp
//
// Purpose:
//  RHEL 9 ABI floor compatibility shims for Linux language extensions.
//
//  Built on Ubuntu 24.04 (glibc 2.38, GCC 13), but must load on RHEL 9 (glibc 2.34).
//  The toolchain - not our own code - references a few symbols above that floor, so
//  they cannot be avoided at the call site, and without local definitions the
//  extension fails to dlopen. Each is redefined in terms of an equivalent that
//  exists on the floor:
//
//    __isoc23_strtol/_strtoll/_strtoull/_strtoul  @GLIBC_2.38 -> classic strtol family
//    arc4random                                   @GLIBC_2.36 -> getrandom(2)
//
//  Hidden, so this library never interposes on glibc for the rest of the host
//  process. __asm__-labelled, because <stdlib.h> has already bound the glibc names.
//
//  _dl_find_object @GLIBC_2.35 is not shimmed: a stub silently breaks exception
//  unwinding, so the extensions link libgcc dynamically, removing the reference.
//
//  Gated by validate-*-elf.sh in the consuming superproject, which compares symbol
//  versions only - it cannot catch a local definition that is present but wrong.
//**************************************************************************************************

#if defined(__linux__)

// A libc header must come before __GLIBC__ is tested: the macro comes from glibc's
// headers, not the compiler, so testing it earlier compiles this file to nothing.
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

#if defined(__GLIBC__)

#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
#include <ios>
#endif

#define ABI_FLOOR_HIDDEN __attribute__((visibility("hidden")))

// Reports through write(2) rather than stdio, so a FILE lock held by a wedged writer
// cannot turn this abort into a hang.
static void AbiFloorAbortNoEntropy(size_t needed, size_t filled, const char *failStage, int failErrno)
{
	char msg[256];
	int n = snprintf(msg, sizeof(msg),
		"AbiFloorCompat: arc4random could not obtain %zu bytes of entropy "
		"(got %zu; failed at %s, errno %d). Aborting rather than returning "
		"predictable output.\n",
		needed, filled, failStage, failErrno);
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

extern "C"
{
	// Labelled explicitly so the C23 redirect in <stdlib.h> cannot make the shims
	// below recurse into themselves.
	long int AbiFloorClassicStrtol(const char *nptr, char **endptr, int base) __asm__("strtol");
	long long int AbiFloorClassicStrtoll(const char *nptr, char **endptr, int base) __asm__("strtoll");
	unsigned long long int AbiFloorClassicStrtoull(const char *nptr, char **endptr, int base) __asm__("strtoull");
	unsigned long int AbiFloorClassicStrtoul(const char *nptr, char **endptr, int base) __asm__("strtoul");

	ABI_FLOOR_HIDDEN long int AbiFloorIsoc23Strtol(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtol");
	ABI_FLOOR_HIDDEN long long int AbiFloorIsoc23Strtoll(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoll");
	ABI_FLOOR_HIDDEN unsigned long long int AbiFloorIsoc23Strtoull(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoull");
	ABI_FLOOR_HIDDEN unsigned long int AbiFloorIsoc23Strtoul(const char *nptr, char **endptr, int base) __asm__("__isoc23_strtoul");
	ABI_FLOOR_HIDDEN uint32_t AbiFloorArc4Random(void) __asm__("arc4random");

	// Safe because the C23 forms differ only in accepting a "0b" prefix when base is
	// 0 or 2, which no caller in these extensions parses.
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

	// Must not degrade to a weak source: if neither the kernel CSPRNG nor /dev/urandom
	// can fill the request, it aborts rather than return predictable bytes.
	ABI_FLOOR_HIDDEN uint32_t AbiFloorArc4Random(void)
	{
		// glibc's arc4random never disturbs errno, and callers may inspect it.
		const int savedErrno = errno;

		uint32_t value = 0;
		unsigned char *out = reinterpret_cast<unsigned char *>(&value);
		size_t filled = 0;

		// errno alone cannot describe the failure: a zero-length result sets none, and
		// later syscalls overwrite it.
		int failErrno = 0;
		const char *failStage = "none";

		while (filled < sizeof(value))
		{
			// A zero return makes no progress, so treat it as terminal rather than
			// spinning on the same call (seccomp filters can produce it).
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
			AbiFloorAbortNoEntropy(sizeof(value), filled, failStage, failErrno);
		}

		errno = savedErrno;
		return value;
	}

	// GCC 13 makes every TU including <iostream> depend on GLIBCXX_3.4.32, which RHEL 9's
	// libstdc++ 3.4.29 lacks. The symbol is a link-time sentinel and is never called, so
	// defining it drops that dependency; the streams are constructed by the
	// std::ios_base::Init object below instead.
	//
	// Opt-in, because libstdc++.a already provides it and a static link would collide.
	// R and Python need it: both get libstdc++.so.6 through their own dependencies, and
	// both reach <iostream> through the Rcpp/RInside and Boost.Python headers.
#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
	ABI_FLOOR_HIDDEN void AbiFloorIosBaseLibraryInit(void) __asm__("_ZSt21ios_base_library_initv");
	ABI_FLOOR_HIDDEN void AbiFloorIosBaseLibraryInit(void)
	{
		// Never called; exists only to satisfy the link.
	}
#endif
}

#ifdef ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT
// Constructs std::cin/cout/cerr/clog on hosts whose libstdc++ predates 3.4.32. At
// namespace scope so it runs at dlopen, since the sentinel above never will.
static std::ios_base::Init g_abiFloorIosBaseInit;
#endif

#else
#error "AbiFloorCompat_linux.cpp is compiled for Linux but __GLIBC__ is not defined; the shims would be silently empty."
#endif // __GLIBC__
#endif // __linux__
