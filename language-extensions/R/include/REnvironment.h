//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
//  @File: REnvironment.h
//
// Purpose:
//  Class to keep the global embedded R environment.
//
//**************************************************************************************************

#pragma once

//--------------------------------------------------------------------------------------------------
// Name: REnvironment
//
// Description:
//  Global class storing the global embedded R environment.
//
class REnvironment
{
public:

	// Initializes this global class
	//
	static void Init(SQLULEN extensionParamsLength);

	// Cleans up this global class
	//
	static void Cleanup();

	// Gets the global embedded R environment as an RInside object pointer.
	//
	static RInside* EmbeddedREnvironment()
	{
		if (sm_embeddedREnvPtr == nullptr)
		{
			throw std::runtime_error("Embedded R environment has not been initialized.");
		}

		return sm_embeddedREnvPtr.get();
	}

	// Encloses the given script in a try catch block.
	//
	static std::string GetScriptWithTryCatch(const std::string &script);

private:

	// An unique pointer to the embedded R environment via RInside.
	// We execute all R scripts in this environment and there can only be a single
	// instance of RInside in the extension.
	// Avoid using a shared pointer since it leads to data races.
	//
	static std::unique_ptr<RInside> sm_embeddedREnvPtr;

	// Original library path
	//
	static std::unique_ptr<Rcpp::CharacterVector> sm_originalPath;
};

#ifdef _WIN32
#define REXTENSION_INTERFACE __declspec(dllexport)
#elif __linux__
#define REXTENSION_INTERFACE __attribute__((visibility("default")))
#else
#define REXTENSION_INTERFACE
#endif

#ifdef __cplusplus
extern "C" {
#endif/* __cplusplus */

// Simply executes the given script.
//
REXTENSION_INTERFACE void ExecuteScript(const std::string &script);

// Executes the given script and returns the result as an SEXP pointer.
//
REXTENSION_INTERFACE SEXP ExecuteScriptAndGetResult(const std::string &script);

#ifdef __cplusplus
} /* End of extern "C" { */
#endif/* __cplusplus */
