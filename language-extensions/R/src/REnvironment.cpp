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
//  @File: REnvironment.cpp
//
// Purpose:
//  Class to keep the global R environment.
//
//**************************************************************************************************

#include "Common.h"

#include "RLibrarySession.h"

using namespace std;

unique_ptr<RInside> REnvironment::sm_embeddedREnvPtr = nullptr;
unique_ptr<Rcpp::CharacterVector> REnvironment::sm_originalPath = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: GetScriptWithTryCatch
//
// Description:
//  Encloses the given script in a try catch block.
//
// Returns:
//  Modified script enclosed in try catch
//
string GetScriptWithTryCatch(const string &script)
{
	stringstream tryCatchScript;
	tryCatchScript << "tryCatch("

		<< "\n{\n  " // open try

			<< script

		<< "\n},\n" // end try

		<< "error = function(err)"
		<< "\n{\n" // open catch
			// Stop from script evaluation indicates failure
			// and RInside would throw an exception in that case
			//
			<< "  stop(err);"

		<< "\n}" // end catch
	<< "\n);"; // end tryCatch

	return tryCatchScript.str();
}

//--------------------------------------------------------------------------------------------------
// Name: ExecuteScript
//
// Description:
//  Executes the given script.
//
// Remarks:
//  This may throw an exception if embedded R environment is not initialized or
//  if evaluation of script fails.
//
void ExecuteScript(const string &script)
{
	LOG("ExecuteScript");

	RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();
	string tryCatchScript = GetScriptWithTryCatch(script);
	LOG(tryCatchScript);

	// If evaluation of script fails, this throws an exception.
	//
	embeddedREnvPtr->parseEvalQ(tryCatchScript.c_str());
}

//--------------------------------------------------------------------------------------------------
// Name: ExecuteScriptAndGetResult
//
// Description:
//  Executes the given script and returns the result as an SEXP pointer.
//
// Remarks:
//  This may throw an exception if embedded R environment is not initialized or
//  if evaluation of script fails.
//
SEXP ExecuteScriptAndGetResult(const string &script)
{
	LOG("ExecuteScriptAndGetResult");

	SEXP result = nullptr;
	RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

	string tryCatchScript = GetScriptWithTryCatch(script);
	LOG(tryCatchScript);
	result = static_cast<SEXP>(embeddedREnvPtr->parseEval(tryCatchScript));

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: REnvironment::Init
//
// Description:
//  Initialize the class
//
void REnvironment::Init(SQLULEN extensionParamsLength)
{
	LOG("REnvironment::Init");

	// Setting up the parameters to be passed to the R runtime.
	//
	vector<char*> argsForR;
	unique_ptr<char[]> dummyInputScriptPtr = Utilities::GenerateUniquePtr("dummyInputScript");
	unique_ptr<char[]> noSaveOptionPtr = Utilities::GenerateUniquePtr("--no-save");
	unique_ptr<char[]> extensionParamsPtr = Utilities::GenerateUniquePtr(RPathSettings::Params());

	if (extensionParamsLength > 0)
	{
		// Tokenize the command line to find the arguments for R.
		//
		Utilities::Tokenize(extensionParamsPtr.get(), " ", &argsForR);
	}
	else
	{
		// If no command line, pass in a dummy input script
		// Rf_initEmbeddedR needs an input script parameter as the first argument
		// even if it isn't used.
		//
		argsForR.push_back(dummyInputScriptPtr.get());
	}

	// --no-save is required to indicate that it is a non-interactive session. It can't be the first
	// argument otherwise it is considered as the script.
	//
	argsForR.push_back(noSaveOptionPtr.get());

	// Initialize the R runtime using the parameters set above.
	// Generate an unique pointer first then move it to be shared to avoid
	// printing issues on windows.
	//
	sm_embeddedREnvPtr = make_unique<RInside>(argsForR.size(), argsForR.data());

	// R_GlobalEnv is defined in Rinternals.h available for use once Rf_initEmbeddedR has been called
	//
	if (R_GlobalEnv == nullptr)
	{
		throw runtime_error("Embedded R is not initialized.");
	}

	string privateLibPath = Utilities::NormalizePathString(RPathSettings::PrivateLibraryPath());
	string publicLibPath = Utilities::NormalizePathString(RPathSettings::PublicLibraryPath());

	// Store the original path for restoring later in cleanup.
	//
	sm_originalPath = make_unique<Rcpp::CharacterVector>(
		ExecuteScriptAndGetResult(RLibrarySession::sm_ScriptToGetLibPaths));

	// Add the public and private library paths to .libPaths() for use later in executing scripts.
	//
	Rcpp::CharacterVector pathsToSet(3);
	pathsToSet[0] = privateLibPath;
	pathsToSet[1] = publicLibPath;
	pathsToSet[2] = RLibrarySession::sm_ScriptToGetLibPaths;
	string prependToLibPathsScript = RLibrarySession::GenerateScriptToSetLibPath(
		pathsToSet,
		true); // isLastElementLibPaths

	// Execute prepend to library paths script.
	//
	ExecuteScript(prependToLibPathsScript);
}

//--------------------------------------------------------------------------------------------------
// Name: REnvironment::Cleanup
//
// Description:
//  Cleans up by resetting the R libPath.
//
void REnvironment::Cleanup()
{
	LOG("REnvironment::Cleanup");

	if (sm_originalPath != nullptr)
	{
		string restoreLibPathsScript = RLibrarySession::GenerateScriptToSetLibPath(
			*sm_originalPath,
			false); // isLastElementLibPaths

		// Execute restoration of library paths script.
		//
		ExecuteScript(restoreLibPathsScript);
	}
}
