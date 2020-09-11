//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
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
//  Global class to keep the global R environment.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

unique_ptr<RInside> REnvironment::sm_embeddedREnvPtr = nullptr;
unique_ptr<Rcpp::CharacterVector> REnvironment::sm_originalPath = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: GenerateScriptToSetLibPath
//
// Description:
//  Creates a script that would set the libPaths to a character vector of strings contained in the
//  given the Rcpp character vector of paths. If the given vector is empty, we simply return the
//  default value of the script which is .libPaths()
//
// Returns:
//  An R script that can be used to set the libPaths.
//
string GenerateScriptToSetLibPath(Rcpp::CharacterVector pathsVector, bool isLastElementLibPaths)
{
	// Initialize the libPaths script to default value
	//
	string setLibPaths = ".libPaths()";

	if (pathsVector.size() > 0)
	{
		// The script looks like either of the two scenarios :
		// A) .libPaths( c( 'Path1', 'Path2', .libPaths())); OR
		// B) .libPaths( c( 'Path1', 'Path2', 'Path3'));
		//
		setLibPaths = ".libPaths( c( ";

		for (int index = 0; index < pathsVector.size(); ++index)
		{
			// Do the processing only if the path is not empty
			//
			if (!pathsVector[index].empty())
			{
				string normalizedPath = Utilities::NormalizePathString(
					string(pathsVector[index]));

				if (index != pathsVector.size() - 1)
				{
					// This is the case for all paths except the last one
					//
					setLibPaths += "'" + normalizedPath + "', ";
				}
				else
				{
					if (isLastElementLibPaths)
					{
						// This is scenario A: if the last element is .libPaths()
						// then we don't need to enclose it in quotes.
						//
						setLibPaths += normalizedPath;
					}
					else
					{
						// This is scenario B: if the last element is a regular path,
						// we need to enclose it in quotes but not add a ','
						//
						setLibPaths += "'" + normalizedPath + "'";
					}
				}
			}
		}

		setLibPaths += "));";
	}

	return setLibPaths;
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
		// Rf_initEmbeddedR needs an input script parameter as the first argument even if it isn't used.
		//
		argsForR.push_back(dummyInputScriptPtr.get());
	}

	// --no-save is required to indicate that it is a non-interactive session. It can't be the first
	// argument otherwise it is considered as the script.
	//
	argsForR.push_back(noSaveOptionPtr.get());

	// Initialize the R runtime using the parameters set above.
	// Generate an unique pointer first then move it to be shared to avoid printing issues on windows.
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
	sm_originalPath = make_unique<Rcpp::CharacterVector>(static_cast<SEXP>(
		sm_embeddedREnvPtr->parseEval(".libPaths()")));

	// Add the public and private library paths to .libPaths() for use later in executing scripts.
	//
	Rcpp::CharacterVector pathsToSet(3);
	pathsToSet[0] = privateLibPath;
	pathsToSet[1] = publicLibPath;
	pathsToSet[2] = ".libPaths()";
	string prependToLibPathsScript = GenerateScriptToSetLibPath(
		pathsToSet,
		true); // isLastElementLibPaths

	// Execute prepend to library paths script.
	//
	sm_embeddedREnvPtr->parseEvalQ(prependToLibPathsScript);
}

//--------------------------------------------------------------------------------------------------
// Name: REnvironment::Cleanup
//
// Description:
//  Cleanup, reset the R libPath.
//
void REnvironment::Cleanup()
{
	LOG("REnvironment::Cleanup");

	string restoreLibPathsScript = GenerateScriptToSetLibPath(
		*sm_originalPath,
		false); // isLastElementLibPaths

	// Execute restoration of library paths script.
	//
	sm_embeddedREnvPtr->parseEvalQ(restoreLibPathsScript);
}
