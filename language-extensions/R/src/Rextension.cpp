//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: Rextension.cpp
// @Owner: anmunde
//
// Purpose:
//	 R extension DLL that can be loaded by ExtHost. This library inits embedded R,
//	 handles communication with ExtHost, and executes user-specified
//	 R script
//
//*********************************************************************
#include <stdio.h>
#include <R.h>
#include <Rembedded.h>
#include <Rdefines.h>
#include <R_ext/Parse.h>
#include "Rextension.h"
