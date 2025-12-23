#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string.h>
#include "UTFileCopy.h"

// Link required libraries
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

// Helper function: Convert ANSI string to Wide string
static WCHAR* AnsiToWide(const char* pszAnsi)
{
    if (!pszAnsi)
        return NULL;

    int len = MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, NULL, 0);
    if (len <= 0)
        return NULL;

    WCHAR* pwszWide = (WCHAR*)LocalAlloc(LPTR, len * sizeof(WCHAR));
    if (!pwszWide)
        return NULL;

    MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, pwszWide, len);
    return pwszWide;
}

// Helper function: Get filename from path
static const WCHAR* GetFilenameFromPath(const WCHAR* pwszPath)
{
    if (!pwszPath)
        return NULL;

    const WCHAR* pwszFilename = wcsrchr(pwszPath, L'\\');
    if (pwszFilename)
        return pwszFilename + 1;

    pwszFilename = wcsrchr(pwszPath, L'/');
    if (pwszFilename)
        return pwszFilename + 1;

    return pwszPath;
}

// Helper function: Build destination path using current directory
static WCHAR* BuildDestinationPath(const WCHAR* pwszSource, BOOL bIsFileCopy)
{
    WCHAR wszCurrentDir[MAX_PATH];
    DWORD dwLen = GetCurrentDirectoryW(MAX_PATH, wszCurrentDir);
    if (dwLen == 0 || dwLen >= MAX_PATH)
        return NULL;

    if (bIsFileCopy)
    {
        const WCHAR* pwszFilename = GetFilenameFromPath(pwszSource);
        if (!pwszFilename)
            return NULL;

        size_t totalLen = dwLen + 1 + wcslen(pwszFilename) + 1;
        WCHAR* pwszDest = (WCHAR*)LocalAlloc(LPTR, totalLen * sizeof(WCHAR));
        if (!pwszDest)
            return NULL;

        wcscpy(pwszDest, wszCurrentDir);
        if (wszCurrentDir[dwLen - 1] != L'\\')
            wcscat(pwszDest, L"\\");
        wcscat(pwszDest, pwszFilename);

        return pwszDest;
    }
    else
    {
        WCHAR* pwszDest = (WCHAR*)LocalAlloc(LPTR, (dwLen + 1) * sizeof(WCHAR));
        if (!pwszDest)
            return NULL;

        wcscpy(pwszDest, wszCurrentDir);
        return pwszDest;
    }
}

// Helper function: Prepare path for SHFileOperation (double-null terminated)
static WCHAR* PrepareShellPath(const WCHAR* pwszPath)
{
    if (!pwszPath)
        return NULL;

    size_t len = wcslen(pwszPath);
    WCHAR* pwszResult = (WCHAR*)LocalAlloc(LPTR, (len + 2) * sizeof(WCHAR));
    if (!pwszResult)
        return NULL;

    wcscpy(pwszResult, pwszPath);
    return pwszResult;
}

// Main export function
__declspec(dllexport) void __stdcall SilentCopy(char* pszParams)
{
    char* pszOp = NULL;
    char* pszSource = NULL;
    char* pszDest = NULL;
    char* pszParamsCopy = NULL;
    WCHAR* pwszSource = NULL;
    WCHAR* pwszDest = NULL;
    WCHAR* pwszShellSource = NULL;
    WCHAR* pwszShellDest = NULL;
    BOOL bDestAllocated = FALSE;
    BOOL bIsFileCopy = FALSE;

    if (!pszParams)
        return;

    // Make a working copy for parsing
    size_t len = strlen(pszParams);
    pszParamsCopy = (char*)LocalAlloc(LPTR, len + 1);
    if (!pszParamsCopy)
        return;

    strcpy(pszParamsCopy, pszParams);

    // Parse parameters
    pszOp = strtok(pszParamsCopy, "|");
    if (!pszOp)
        goto cleanup;

    pszSource = strtok(NULL, "|");
    if (!pszSource || strlen(pszSource) == 0)
        goto cleanup;

    pszDest = strtok(NULL, "|");

    // MKDIR operation
    if (_stricmp(pszOp, "mkdir") == 0)
    {
        char* pszFolderList = NULL;
        char* pszFolder = NULL;

        // Re-parse from original to get all folders
        char* pszPipe = strchr(pszParams, '|');
        if (pszPipe)
        {
            size_t nFolderListLen = strlen(pszPipe + 1);
            pszFolderList = (char*)LocalAlloc(LPTR, nFolderListLen + 1);
            if (pszFolderList)
                strcpy(pszFolderList, pszPipe + 1);
        }

        if (!pszFolderList)
            goto cleanup;

        // Process each folder
        pszFolder = strtok(pszFolderList, "|");
        while (pszFolder && strlen(pszFolder) > 0)
        {
            WCHAR wszFullPath[MAX_PATH];
            WCHAR* pwszFolder = AnsiToWide(pszFolder);

            if (pwszFolder)
            {
                if (GetFullPathNameW(pwszFolder, MAX_PATH, wszFullPath, NULL) > 0)
                {
                    // Create directory recursively
                    WCHAR* p;
                    WCHAR wszTemp[MAX_PATH];

                    wcscpy(wszTemp, wszFullPath);
                    p = wszTemp;

                    // Skip drive letter
                    if (p[1] == L':' && p[2] == L'\\')
                        p += 3;

                    while (*p)
                    {
                        if (*p == L'\\')
                        {
                            *p = L'\0';
                            CreateDirectoryW(wszTemp, NULL);
                            *p = L'\\';
                        }
                        p++;
                    }

                    // Create final directory
                    CreateDirectoryW(wszFullPath, NULL);
                }
                LocalFree(pwszFolder);
            }

            pszFolder = strtok(NULL, "|");
        }

        LocalFree(pszFolderList);
        goto cleanup;
    }

    // Determine operation type
    bIsFileCopy = (_stricmp(pszOp, "copy") == 0);

    // Convert source to wide string
    pwszSource = AnsiToWide(pszSource);
    if (!pwszSource)
        goto cleanup;

    // Handle destination
    if (!pszDest || strlen(pszDest) == 0)
    {
        pwszDest = BuildDestinationPath(pwszSource, bIsFileCopy);
        bDestAllocated = TRUE;
    }
    else
    {
        pwszDest = AnsiToWide(pszDest);
        bDestAllocated = TRUE;
    }

    if (!pwszDest)
        goto cleanup;

    // COPY operation
    if (bIsFileCopy)
    {
        CopyFileW(pwszSource, pwszDest, FALSE);
    }
    // XCOPY operation
    else if (_stricmp(pszOp, "xcopy") == 0)
    {
        WCHAR wszFullSource[MAX_PATH];
        WCHAR wszFullDest[MAX_PATH];
        SHFILEOPSTRUCTW fileOp = {0};

        // Get full paths
        if (GetFullPathNameW(pwszSource, MAX_PATH, wszFullSource, NULL) == 0)
            wcscpy(wszFullSource, pwszSource);

        if (GetFullPathNameW(pwszDest, MAX_PATH, wszFullDest, NULL) == 0)
            wcscpy(wszFullDest, pwszDest);

        // Prepare double-null terminated paths
        pwszShellSource = PrepareShellPath(wszFullSource);
        pwszShellDest = PrepareShellPath(wszFullDest);

        if (!pwszShellSource || !pwszShellDest)
            goto cleanup;

        fileOp.hwnd = NULL;
        fileOp.wFunc = FO_COPY;
        fileOp.pFrom = pwszShellSource;
        fileOp.pTo = pwszShellDest;
        fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
        fileOp.fAnyOperationsAborted = FALSE;
        fileOp.hNameMappings = NULL;
        fileOp.lpszProgressTitle = NULL;

        SHFileOperationW(&fileOp);
    }

cleanup:
    if (pwszShellDest)
        LocalFree(pwszShellDest);
    if (pwszShellSource)
        LocalFree(pwszShellSource);
    if (pwszDest && bDestAllocated)
        LocalFree(pwszDest);
    if (pwszSource)
        LocalFree(pwszSource);
    if (pszParamsCopy)
        LocalFree(pszParamsCopy);
}

// DLL entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(hinstDLL);

    return TRUE;
}
