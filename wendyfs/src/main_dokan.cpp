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

#include <wendy/Mutex.hpp>
#include <wendy/Project.hpp>
#include <wendy/ProjectFileSystem.hpp>
#include <wendy/ScopeLock.hpp>

//#include "ProjectProxy.hpp"

//static ProjectProxy *proxy = NULL;
static wendy::ProjectFileSystem *fs = NULL;
static wendy::Mutex mutex;

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
	wendy::ScopeLock lock(&mutex);
	wprintf(L"CreateFile: %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (attributes.folder)
		info->IsDirectory = TRUE;
	
	return 0;
}

static int DOKAN_CALLBACK WendyOpenDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"OpenDir %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (!attributes.folder)
		return -ERROR_PATH_NOT_FOUND; // TODO: find the right error code for this
	
	return 0;
}

static int DOKAN_CALLBACK WendyCreateDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"CreateDirectory %s\n", filename);
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (fs->stat(path, &attributes))
		return -ERROR_ALREADY_EXISTS;
	
	fs->mkdir(path);
	
	return 0;
}

static int DOKAN_CALLBACK WendyCleanup(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"Cleanup %s\n", filename);
	
	return 0;
}

int DOKAN_CALLBACK WendyCloseFile(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"CloseFile %s\n", filename);
	
	return 0;
}

int DOKAN_CALLBACK WendyReadFile(LPCWSTR filename, LPVOID buffer, DWORD numberOfBytesToRead, LPDWORD numberOfBytesRead, LONGLONG offset, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"ReadFile %s\n", filename);
	
	char *content = "Hi from Wendy!\r\n";
	DWORD remaining = (DWORD)((LONGLONG)strlen(content) - offset);
	*numberOfBytesRead = (remaining < numberOfBytesToRead) ? remaining : numberOfBytesToRead;
	
	wprintf(L"returning %d bytes\n", *numberOfBytesRead);
	memcpy(buffer, content, *numberOfBytesRead);
	
	return 0;
}

int DOKAN_CALLBACK WendyWriteFile(LPCWSTR filename, LPCVOID buffer, DWORD numberOfBytesToWrite, LPDWORD numberOfBytesWritten, LONGLONG offset, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"WriteFile %s\n", filename);
	
	return 0;
}

static int DOKAN_CALLBACK WendyGetFileInformation(LPCWSTR filename, LPBY_HANDLE_FILE_INFORMATION buffer, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"GetFileInformation %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	ZeroMemory(buffer, sizeof(BY_HANDLE_FILE_INFORMATION));
	
	if (attributes.folder)
	{
		buffer->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM; // system enables Desktop.ini customization
	}
	else
	{
		buffer->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		buffer->nFileSizeLow = strlen("Hi from Wendy!\r\n");
	}
	
	return 0;
}

static int DOKAN_CALLBACK WendyFindFiles(LPCWSTR filename, PFillFindData fillFindData, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"FindFiles %s\n", filename);
	
	WIN32_FIND_DATAW entry;
	ZeroMemory(&entry, sizeof(WIN32_FIND_DATAW));
	
	// insert special dirs '.' and '..'
	entry.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	wcscpy(entry.cFileName, L".");
	fillFindData(&entry, info);
	wcscpy(entry.cFileName, L"..");
	fillFindData(&entry, info);
	
	std::string path = makePathStandard(filename);
	
	std::vector<std::string> files;
	fs->readdir(path, &files);
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		// filename
		const std::string &filename = files[i];
		std::wstring wfilename = utf8ToWide(filename);
		wcsncpy(entry.cFileName, wfilename.c_str(), MAX_PATH - 1);
		
		// other attributes
		wendy::ProjectFileSystem::FileAttributes attributes;
		std::string fullChildPath = files[i];
		if (path.size() > 0)
			fullChildPath = path + "/" + files[i];
		fs->stat(fullChildPath, &attributes);
		entry.dwFileAttributes = attributes.folder ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
		entry.nFileSizeLow = strlen("Hi from Wendy!\r\n");
		
		// send item back to caller
		fillFindData(&entry, info);
	}
	
	return 0;
}

static int DOKAN_CALLBACK WendyDeleteDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"DeleteDirectory %s\n", filename);
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	fs->rmdir(path);
	
	return 0;
}

static int DOKAN_CALLBACK WendyGetDiskFreeSpace(PULONGLONG freeBytesAvailable, PULONGLONG totalNumberOfBytes, PULONGLONG totalNumberOfFreeBytes, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	
	*freeBytesAvailable = 16106127360UL; // specific to the calling user, if quotas are used
	*totalNumberOfBytes = 21474836480UL;
	*totalNumberOfFreeBytes = 16106127360UL;
	return 0;
}

static int DOKAN_CALLBACK WendyGetVolumeInformation(LPWSTR volumeNameBuffer, DWORD volumeNameSize, LPDWORD volumeSerialNumber, LPDWORD maximumComponentLength, LPDWORD fileSystemFlags, LPWSTR fileSystemNameBuffer, DWORD fileSystemNameSize, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	
	wcscpy_s(volumeNameBuffer, volumeNameSize / sizeof(WCHAR), L"Wendy");
	*volumeSerialNumber = 0x42;
	*maximumComponentLength = 255;
	*fileSystemFlags = FILE_CASE_PRESERVED_NAMES | FILE_CASE_SENSITIVE_SEARCH | FILE_UNICODE_ON_DISK;
	wcscpy_s(fileSystemNameBuffer, fileSystemNameSize / sizeof(WCHAR), L"wendyfs");
	
	return 0;
}

int __cdecl wmain(ULONG argc, PWCHAR argv[])
{
	if (argc < 2)
	{
		wprintf(L"Usage: %s <drive letter>\n", argv[0]);
		return 0;
	}
	
	DOKAN_OPTIONS options;
	DOKAN_OPERATIONS operations;
	
	ZeroMemory(&options, sizeof(DOKAN_OPTIONS));
	options.Version = DOKAN_VERSION;
	options.ThreadCount = 0; // use default
	options.MountPoint = argv[1];
	options.Options = /*DOKAN_OPTION_DEBUG | DOKAN_OPTION_STDERR |*/ DOKAN_OPTION_KEEP_ALIVE;
	
	ZeroMemory(&operations, sizeof(DOKAN_OPERATIONS));
	operations.CreateFile = WendyCreateFile;
	operations.OpenDirectory = WendyOpenDirectory;
	operations.CreateDirectory = WendyCreateDirectory;
	operations.Cleanup = WendyCleanup;
	operations.CloseFile = WendyCloseFile;
	operations.ReadFile = WendyReadFile;
	operations.WriteFile = WendyWriteFile;
	operations.GetFileInformation = WendyGetFileInformation;
	operations.FindFiles = WendyFindFiles;
	operations.DeleteDirectory = WendyDeleteDirectory;
	operations.GetDiskFreeSpace = WendyGetDiskFreeSpace;
	operations.GetVolumeInformation = WendyGetVolumeInformation;
	
	fs = new wendy::ProjectFileSystem;
	
	int result = DokanMain(&options, &operations);
	
	delete fs;
	
	return result;
}

#endif // _WIN32

