/******************************************************************************
 *
 * Wendy asset manager
 * Copyright (c) 2011-2013 Remi Papillie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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

#include <wendy/Client.hpp>
#include <wendy/RequestState.hpp>

#include <FileSystem.hpp>

static FileSystem *fs = NULL;

static CRITICAL_SECTION wendyMutex;

// utility class to automatically take and release the API mutex
class ScopeLock
{
	public:
		ScopeLock()
		{
			EnterCriticalSection(&wendyMutex);
		}
		
		~ScopeLock()
		{
			LeaveCriticalSection(&wendyMutex);
		}
};

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
	ScopeLock lock;
	
	wprintf(L"CreateFile: %s\n", filename);
	wprintf(L"Access mode: %d\n", accessMode);
	
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
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
		
		std::wstring processNameWide = utf8ToWide(processName);
		wprintf(L"%s opened by %s\n", filename, processNameWide.c_str());
		
		// translate access mode
		bool reading = false;
		bool writing = false;
		bool truncate = false;
		
		if (accessMode & FILE_GENERIC_READ) reading = true;
		if (accessMode &  FILE_GENERIC_WRITE) writing = true;
		if ((creationDisposition == TRUNCATE_EXISTING) || (creationDisposition == CREATE_NEW) || (creationDisposition == CREATE_ALWAYS)) truncate = true;
		
		// open asset
		File *file = fs->open(path, reading, writing, truncate, processName);
		if (!file)
			return -ERROR_PATH_NOT_FOUND;
		
		info->Context = (ULONG64)file; // file descriptor
		wprintf(L"CreateFile succeeded (handle = 0x%x\n", file);
	}
	
	return 0;
}

static int DOKAN_CALLBACK WendyOpenDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"OpenDir %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (!attributes.folder)
		return -ERROR_PATH_NOT_FOUND; // TODO: find the right error code for this
	
	return 0;
}

static int DOKAN_CALLBACK WendyCreateDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"CreateDirectory %s\n", filename);
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
	if (fs->stat(path, &attributes))
		return -ERROR_ALREADY_EXISTS;
	
	fs->mkdir(path);
	
	return 0;
}

static int DOKAN_CALLBACK WendyCleanup(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"Cleanup %s\n", filename);
	
	if (!info->IsDirectory)
	{
		File *file = (File *)info->Context;
		if (!fs->close(file))
			return -ERROR_ACCESS_DENIED;
	}
	
	return 0;
}

int DOKAN_CALLBACK WendyCloseFile(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"CloseFile %s\n", filename);
	
	return 0;
}

int DOKAN_CALLBACK WendyReadFile(LPCWSTR filename, LPVOID buffer, DWORD numberOfBytesToRead, LPDWORD numberOfBytesRead, LONGLONG offset, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"ReadFile %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
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
		File *file = (File *)info->Context;
		if (!fs->read(file, offset, buffer, numberOfBytesToRead))
			return -ERROR_READ_FAULT;
	}
	*numberOfBytesRead = numberOfBytesToRead;
	
	if (eof)
		return -ERROR_HANDLE_EOF;
	
	return 0;
}

int DOKAN_CALLBACK WendyWriteFile(LPCWSTR filename, LPCVOID buffer, DWORD numberOfBytesToWrite, LPDWORD numberOfBytesWritten, LONGLONG offset, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"WriteFile %s\n", filename);
	
	File *file = (File *)info->Context;
	if (!fs->write(file, (unsigned long long)offset, buffer, (unsigned long long)numberOfBytesToWrite))
		return -ERROR_WRITE_FAULT;
	
	*numberOfBytesWritten = numberOfBytesToWrite;
	wprintf(L"written %d bytes\n", *numberOfBytesWritten);
	
	return 0;
}

static int DOKAN_CALLBACK WendyGetFileInformation(LPCWSTR filename, LPBY_HANDLE_FILE_INFORMATION buffer, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"GetFileInformation %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
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
	ScopeLock lock;
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
		FileSystem::FileAttributes attributes;
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
	ScopeLock lock;
	
	wprintf(L"DeleteFile %s\n", filename);
	
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (!fs->unlink(path))
		return -ERROR_ACCESS_DENIED;
	
	return 0;
}

static int DOKAN_CALLBACK WendyDeleteDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	wprintf(L"DeleteDirectory %s\n", filename);
	std::string path = makePathStandard(filename);
	
	FileSystem::FileAttributes attributes;
	if (!fs->stat(path, &attributes))
		return -ERROR_PATH_NOT_FOUND;
	
	if (!fs->rmdir(path))
		return -ERROR_DIR_NOT_EMPTY;
	
	return 0;
}

static int DOKAN_CALLBACK WendyGetDiskFreeSpace(PULONGLONG freeBytesAvailable, PULONGLONG totalNumberOfBytes, PULONGLONG totalNumberOfFreeBytes, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	
	*freeBytesAvailable = 16106127360UL; // specific to the calling user, if quotas are used
	*totalNumberOfBytes = 21474836480UL;
	*totalNumberOfFreeBytes = 16106127360UL;
	return 0;
}

static int DOKAN_CALLBACK WendyGetVolumeInformation(LPWSTR volumeNameBuffer, DWORD volumeNameSize, LPDWORD volumeSerialNumber, LPDWORD maximumComponentLength, LPDWORD fileSystemFlags, LPWSTR fileSystemNameBuffer, DWORD fileSystemNameSize, PDOKAN_FILE_INFO info)
{
	ScopeLock lock;
	
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
		printf("Usage: %s <drive letter>\n", argv[0]);
		return 0;
	}
	
	DOKAN_OPTIONS options;
	DOKAN_OPERATIONS operations;
	
	ZeroMemory(&options, sizeof(DOKAN_OPTIONS));
	options.Version = DOKAN_VERSION;
	options.ThreadCount = 0; // use default
	std::wstring mountPoint = utf8ToWide(argv[1]);
	options.MountPoint = mountPoint.c_str();
	options.Options = DOKAN_OPTION_DEBUG | DOKAN_OPTION_STDERR | DOKAN_OPTION_KEEP_ALIVE;
	
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
	
	fs = new FileSystem;
	
	InitializeCriticalSection(&wendyMutex);
	
	int result = DokanMain(&options, &operations);
	
	DeleteCriticalSection(&wendyMutex);
	
	delete fs;
	
	return result;
}

#endif // _WIN32

