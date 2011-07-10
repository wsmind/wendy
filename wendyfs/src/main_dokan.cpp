#ifdef _WIN32

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <dokan.h>
#include <wendy/Project.hpp>

#include "ProjectProxy.hpp"

static ProjectProxy *proxy = NULL;

static int DOKAN_CALLBACK WendyCreateFile(LPCWSTR filename, DWORD accessMode, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes, PDOKAN_FILE_INFO info)
{
	wprintf(L"CreateFile: %s\n", filename);
	return 0;
}

static int DOKAN_CALLBACK WendyOpenDirectory(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wprintf(L"OpenDir %s\n", filename);
	
	if (wcscmp(filename, L"\\") == 0)
		return 0;
	else if (wcscmp(filename, L"\\plop") == 0)
		return 0;
	
	return -ERROR_PATH_NOT_FOUND;
}

static int DOKAN_CALLBACK WendyCleanup(LPCWSTR filename, PDOKAN_FILE_INFO info)
{
	wprintf(L"Cleanup %s\n", filename);
	
	return 0;
}

static int DOKAN_CALLBACK WendyFindFiles(LPCWSTR filename, PFillFindData	fillFindData, PDOKAN_FILE_INFO info)
{
	WIN32_FIND_DATAW entry;
	
	wprintf(L"FindFiles %s\n", filename);
	
	if (wcscmp(filename, L"\\") != 0)
		return -ERROR_PATH_NOT_FOUND;
	
	ZeroMemory(&entry, sizeof(WIN32_FIND_DATAW));
	
	std::vector<std::string> files = proxy->listDirectory("");
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		const std::string &filename = files[i];
		
		entry.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, entry.cFileName, 200);
		fillFindData(&entry, info);
	}
	
	/*entry.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	wcscpy(entry.cFileName, L"plop");
	fillFindData(&entry, info);
	
	entry.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	wcscpy(entry.cFileName, L"test");
	fillFindData(&entry, info);
	
	entry.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	wcscpy(entry.cFileName, L"kitten");
	fillFindData(&entry, info);*/
	
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
	
	return DokanMain(&options, &operations);
}

#endif // _WIN32

