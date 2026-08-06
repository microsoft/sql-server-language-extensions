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
//  The sentinel is never called - the compiler emits the name with no call site, purely so the
//  resulting version dependency refuses to load against a libstdc++ that would leave the standard
//  streams unconstructed. Defining it here removes that dependency, so this file must also perform
//  the initialisation the sentinel was guarding: that is the namespace-scope ios_base::Init below.
//  A function-local static would never run, because the function is never called.
//
//  extern "C" emits the mangled name verbatim; hidden visibility keeps it out of our exports.
//
//**************************************************************************************************

#include <ios>

extern "C" __attribute__((visibility("hidden"))) void _ZSt21ios_base_library_initv()
{
}

// Constructs std::cin/cout/cerr/clog at dlopen, before any extension code or statically linked
// Rcpp / RInside code can touch a stream. The constructor is refcounted and idempotent, and is old
// enough to be present in every libstdc++ we target, so it does not raise the ABI floor.
//
static std::ios_base::Init g_iosBaseInit;
