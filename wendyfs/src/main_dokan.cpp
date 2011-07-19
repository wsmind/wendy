/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

#ifdef _WIN32

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <dokan.h>
#include <iostream>

#include <wendy/Project.hpp>

#include "ProjectProxy.hpp"

static ProjectProxy *proxy = NULL;

static std::string wideToUtf8(std::wstring wide)
{
	// passing no output buffer return required buffer size
	int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
	
	// convert into a temporary buffer
	char *outputBuffer = new char[size];
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, outputBuffer, size, NULL, NULL);
	
	// create the equivalent string object
	std::string utf = outputBuffer;
	
	// destroy temporary buffer
	delete outputBuffer;
	
	return utf;
}

static std::wstring utf8ToWide(std::string utf)
{
	// passing no output buffer return required buffer size
	int size = MultiByteToWideChar(CP_UTF8, 0, utf.c_str(), -1, NULL, 0);
	
	// convert into a temporary buffer
	wchar_t *outputBuffer = new wchar_t[size];
	MultiByteToWideChar(CP_UTF8, 0, utf.c_str(), -1, outputBuffer, size);
	
	// create the equivalent wstring object
	std::wstring wide = outputBuffer;
	
	// destroy temporary buffer
	delete outputBuffer;
	
	return wide;
}

// convert to utf-8, change '\\' to '/' and remove leading slash
static std::string makePathStandard(LPCWSTR path)
{
	std::string utfPath = wideToUtf8(std::wstring(path));
	
	// change backslashes to forward slashes
	size_t slashPos;
	while ((slashPos = utfPath.find('\\')) != std::string::npos)
		utfPath.replace(slashPos, 1, "/");
	
	// remove leading slash
	return utfPath.substr(1);
}

static int DOKAN_CALLBACK WendyCreateFile(LPCWSTR filename, DWORD accessMode, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes, PDOKAN_FILE_INFO info)
{
	wprintf(L"CreateFile: %s\n", filename);
	return 0;
}

static int DOKAN_CALLBACK WendyOpenDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wprintf(L"OpenDir %s\n", filename);
	std::cout << "OpenDir: " << makePathStandard(filename) << std::endl;
	
	std::string path = makePathStandard(filename);
	
	ProjectProxy::FileAttributes attributes;
	if (!proxy->getFileAttributes(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (!attributes.folder)
		return -ERROR_PATH_NOT_FOUND; // TODO: find the right error code for this
	
	return 0;
	
	/*if (wcscmp(filename, L"\\") == 0)
		return 0;
	else if (wcscmp(filename, L"\\plop") == 0)
		return 0;
	
	return -ERROR_PATH_NOT_FOUND;*/
}

static int DOKAN_CALLBACK WendyCleanup(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wprintf(L"Cleanup %s\n", filename);
	
	return 0;
}

static int DOKAN_CALLBACK WendyFindFiles(LPCWSTR filename, PFillFindData fillFindData, PDOKAN_FILE_INFO info)
{
	wprintf(L"FindFiles %s\n", filename);
	
	WIN32_FIND_DATAW entry;
	ZeroMemory(&entry, sizeof(WIN32_FIND_DATAW));
	
	std::string path = makePathStandard(filename);
	
	std::vector<std::string> files = proxy->listFolder(path);
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		// filename
		const std::string &filename = files[i];
		std::wstring wfilename = utf8ToWide(filename);
		wcsncpy(entry.cFileName, wfilename.c_str(), MAX_PATH - 1);
		
		// other attributes
		ProjectProxy::FileAttributes attributes;
		proxy->getFileAttributes(files[i], &attributes);
		entry.dwFileAttributes = attributes.folder ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
		
		// send item back to caller
		fillFindData(&entry, info);
	}
	
	return 0;
}

int __cdecl wmain(ULONG argc, PWCHAR argv[])
{
	if (argc < 3)
	{
		wprintf(L"Usage: %s <drive letter> <project name>\n", argv[0]);
		return 0;
	}
	
	DOKAN_OPTIONS options;
	DOKAN_OPERATIONS operations;
	
	ZeroMemory(&options, sizeof(DOKAN_OPTIONS));
	options.Version = DOKAN_VERSION;
	options.ThreadCount = 0; // use default
	options.MountPoint = argv[1];
	options.Options = DOKAN_OPTION_KEEP_ALIVE;
	
	ZeroMemory(&operations, sizeof(DOKAN_OPERATIONS));
	operations.CreateFile = WendyCreateFile;
	operations.OpenDirectory = WendyOpenDirectory;
	operations.Cleanup = WendyCleanup;
	operations.FindFiles = WendyFindFiles;
	
	//char projectName[500];
	//WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, projectName, 500, NULL, NULL);
	proxy = new ProjectProxy();
	
	int result = DokanMain(&options, &operations);
	
	delete proxy;
	
	return result;
}

#endif // _WIN32

