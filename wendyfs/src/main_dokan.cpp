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

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winbase.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <dokan.h>
#include <iostream>

#include <wendy/Mutex.hpp>
#include <wendy/Project.hpp>
#include <wendy/ProjectFileSystem.hpp>
#include <wendy/ScopeLock.hpp>

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

static FILETIME unixTimeToFileTime(unsigned long long unixTimestamp)
{
	FILETIME fileTime;
	
	unsigned long long fileTimeValue = (unixTimestamp + 11644473600lu) * 10000000lu;
	fileTime.dwHighDateTime = (fileTimeValue & 0xffffffff00000000lu) >> 32;
	fileTime.dwLowDateTime = (fileTimeValue & 0x00000000fffffffflu) >> 0;
	
	return fileTime;
}

static int DOKAN_CALLBACK WendyCreateFile(LPCWSTR filename, DWORD accessMode, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"CreateFile: %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	bool exists = fs->stat(path, &attributes);
	
	// cases when the file MUST exist
	if (!exists && ((creationDisposition == OPEN_EXISTING) || (creationDisposition == TRUNCATE_EXISTING)))
		return -ERROR_PATH_NOT_FOUND;
	
	// cases when the file MUST NOT exist
	if (exists && (creationDisposition == CREATE_NEW))
		return -ERROR_FILE_EXISTS;
	
	if (attributes.folder)
	{
		// flag as directory
		info->IsDirectory = TRUE;
	}
	else
	{
		// no real open
		if (accessMode == 0)
			return 0;
		
		// retrieve opening process name
		std::string processName = "unknown application";
		HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info->ProcessId);
		if (process != NULL)
		{
			wchar_t processBaseName[256];
			DWORD length = GetModuleBaseName(process, NULL, processBaseName, 255);
			if (length > 0)
				processName = wideToUtf8(processBaseName);
			CloseHandle(process);
		}
		
		// translate access mode
		wendy::ProjectFileSystem::OpenMode mode = wendy::ProjectFileSystem::READING;
		
		if ((accessMode & FILE_GENERIC_READ) == FILE_GENERIC_READ) mode = wendy::ProjectFileSystem::READING;
		if ((accessMode & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE) mode = wendy::ProjectFileSystem::WRITING;
		
		if (mode == wendy::ProjectFileSystem::WRITING)
			return -ERROR_ACCESS_DENIED;
		
		// open asset
		long fd = fs->open(path, mode, processName);
		if (fd == -1)
			return -ERROR_PATH_NOT_FOUND;
		
		info->Context = (ULONG64)fd;
	}
	
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
	
	if (!info->IsDirectory)
	{
		long fd = (long)info->Context;
		fs->close(fd);
	}
	
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
	
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	bool eof = false;
	if ((unsigned long long)offset + (unsigned long long)numberOfBytesToRead > attributes.length)
	{
		if ((unsigned long long)offset < attributes.length)
			numberOfBytesToRead = (DWORD)(attributes.length - offset);
		else
			numberOfBytesToRead = 0;
	}
	
	if (numberOfBytesToRead > 0)
	{
		long fd = (long)info->Context;
		if (!fs->read(fd, (long)offset, buffer, numberOfBytesToRead)) // TODO: take a long long as offset
			return -ERROR_READ_FAULT;
	}
	*numberOfBytesRead = numberOfBytesToRead;
	
	if (eof)
		return -ERROR_HANDLE_EOF;
	
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
		buffer->nFileSizeLow = attributes.length;
		buffer->ftLastWriteTime = unixTimeToFileTime(attributes.date);
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
		entry.nFileSizeLow = attributes.length; // TODO: use the 64 bits
		entry.ftLastWriteTime = unixTimeToFileTime(attributes.date);
		
		// send item back to caller
		fillFindData(&entry, info);
	}
	
	return 0;
}

static int DOKAN_CALLBACK WendyDeleteFile(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wendy::ScopeLock lock(&mutex);
	wprintf(L"DeleteFile %s\n", filename);
	std::string path = makePathStandard(filename);
	
	wendy::ProjectFileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (!fs->unlink(path))
		return -ERROR_ACCESS_DENIED;
	
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
	
	if (!fs->rmdir(path))
		return -ERROR_DIR_NOT_EMPTY;
	
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
	
	wcsncpy(volumeNameBuffer, L"Wendy", volumeNameSize / sizeof(WCHAR));
	*volumeSerialNumber = 0x42;
	*maximumComponentLength = 255;
	*fileSystemFlags = FILE_CASE_PRESERVED_NAMES | FILE_CASE_SENSITIVE_SEARCH | FILE_UNICODE_ON_DISK;
	wcsncpy(fileSystemNameBuffer, L"wendyfs", fileSystemNameSize / sizeof(WCHAR));
	
	return 0;
}

int main(int argc, char **argv)
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
	std::wstring mountPoint = utf8ToWide(argv[1]);
	options.MountPoint = mountPoint.c_str();
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
	operations.DeleteFile = WendyDeleteFile;
	operations.DeleteDirectory = WendyDeleteDirectory;
	operations.GetDiskFreeSpace = WendyGetDiskFreeSpace;
	operations.GetVolumeInformation = WendyGetVolumeInformation;
	
	fs = new wendy::ProjectFileSystem;
	
	int result = DokanMain(&options, &operations);
	
	delete fs;
	
	return result;
}

#endif // _WIN32

