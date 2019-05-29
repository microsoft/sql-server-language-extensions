//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaLibraryUtils.h
// @Owner: brnieb
//
// Purpose:
//	 Library utility classes used by the Java extension.
//
//*********************************************************************
#pragma once

//---------------------------------------------------------------------
// Description:
//	Library utility functions for the Java extension.
//
class JavaLibraryUtils
{
public:
	static std::string GetLibrariesClassPath();

private:
	static void FindAppendFileNames(_In_ const std::string &basePath, _Inout_ std::string &output);
};