//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
//
// @File: IosBaseSentinel_linux.cpp
//
// Purpose:
//  Defines the <iostream> initialisation sentinel locally so libRExtension does not import it from
//  the host's libstdc++. Recent GCC releases version that symbol above what the oldest supported
//  hosts provide, so importing it makes the library fail to load there.
//
//  The other extensions drop the include instead. R cannot: Rcpp.h and RInside.h include
//  <iostream>, and every R translation unit reaches them through Common.h.
//
//  The sentinel has no call site - the compiler emits the reference purely so the resulting
//  version dependency refuses to load against a libstdc++ that would leave the standard streams
//  unconstructed. Defining it here removes that dependency, so this file must also perform the
//  initialisation the sentinel was guarding: that is the namespace-scope ios_base::Init below.
//  A function-local static would never run, because the function is never called.
//
//  extern "C" emits the mangled name verbatim; hidden visibility keeps it out of our exports.
//
//**************************************************************************************************

#include <ios>

extern "C" __attribute__((visibility("hidden"))) void _ZSt21ios_base_library_initv()
{
}

// Constructs std::cin/cout/cerr/clog before any other global constructor in this library can touch
// a stream. The constructor is refcounted and idempotent, and is old enough to be present in every
// libstdc++ we target, so it does not raise the ABI floor.
//
// init_priority is required, not cosmetic: dynamic initialisation order between translation units
// is unspecified, so at default priority an Rcpp / RInside global constructor could run first and
// touch a stream that is not yet constructed - exactly the case the sentinel we just defined away
// was guarding against. libstdc++ gives its own centralised initialiser priority 90 for this
// reason. Priorities 0-100 are reserved and warn, so use the lowest available, 101.
//
static std::ios_base::Init g_iosBaseInit __attribute__((init_priority(101)));
