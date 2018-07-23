#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>
#include <stdio.h>
#include "resource.h"

#ifndef _WIN64

BOOL Is64BitOS();
int RunPubIP64Bit(int argc, char *argv[]);

#endif

void Print(LPSTR message, BOOL error);
int PrintError();
LPSTR GetErrorMessage(DWORD error);

int main(int argc, char *argv[])
{
#ifndef _WIN64
	
	// First check if this is a 64 bit OS. If so, extract 64-bit version of pubip.exe,
	// execute it and redirect output to console.
	if (Is64BitOS())
		return RunPubIP64Bit(argc, argv);

#endif

	HINTERNET hInternetSession, hHttpSession, hHttpRequest;
	PCTSTR acceptTypes[] = { "text/*", NULL};
	char* lpBuffer[256] = { 0 };
	DWORD dwBytesRead = 0, dwBytesAvailable = 0;
	BOOL asJson = argc == 2 && _strnicmp(argv[1], "-json", 5) == 0;

	hInternetSession = InternetOpen("pubip", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (NULL == hInternetSession)
		return PrintError();

	hHttpSession = InternetConnect(hInternetSession, "api.ipify.org", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	if (NULL == hHttpSession)
		return PrintError();

	hHttpRequest = HttpOpenRequest(hHttpSession, NULL, asJson ? "?format=json" : "", NULL, NULL, acceptTypes, INTERNET_FLAG_RELOAD, 0);

	if (NULL == hHttpRequest)
		return PrintError();

	if (!HttpSendRequest(hHttpRequest, NULL, 0, NULL, 0))
		return PrintError();

	while (InternetQueryDataAvailable(hHttpRequest, &dwBytesAvailable, 0, 0) && dwBytesAvailable > 0)
	{		
		ZeroMemory(lpBuffer, 256);

		if (!InternetReadFile(hHttpRequest, (LPVOID)lpBuffer,2000, &dwBytesRead))
			return PrintError();

		Print((LPSTR)lpBuffer, FALSE);
	}

	InternetCloseHandle(hInternetSession);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hHttpRequest);

	return 0;
}

#ifndef _WIN64

BOOL Is64BitOS()
{
	BOOL f64;
	
	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
}

int RunPubIP64Bit(int argc, char *argv[])
{
	HRSRC hResource = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_PUBIP64), RT_RCDATA);
	HGLOBAL hResourceHandle = LoadResource(NULL, hResource);
	LPSTR resourceData = (LPSTR)LockResource(hResourceHandle);
	DWORD dwResourceSize = SizeofResource(NULL, hResource);
	char tempPath[MAX_PATH + 1], fullPath[MAX_PATH + 1], commandLine[MAX_PATH + 10];
	DWORD dwTempPathSize = GetTempPath(MAX_PATH, tempPath);
	HANDLE job, h64BitFile;
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli;
	STARTUPINFO psi;
	PROCESS_INFORMATION proc;
	DWORD dwBytesWritten = 0;

	PathCombine(fullPath, tempPath, "pubip64.exe");

	h64BitFile = CreateFile(fullPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (NULL == h64BitFile)
		PrintError();

	if (!WriteFile(h64BitFile, (LPCVOID)resourceData, dwResourceSize, &dwBytesWritten, NULL))
		PrintError();

	CloseHandle(h64BitFile);

	strcpy(commandLine, fullPath);
	
	for (int i = 1; i < argc; i++)
	{
		strcat(commandLine, " ");
		strcat(commandLine, argv[i]);
	}

	job = CreateJobObject(NULL, NULL);

	if (NULL == job)
		return PrintError();

	ZeroMemory(&jeli, sizeof(jeli));

	//jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_BREAKAWAY_OK;

	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		return PrintError();
	if (!AssignProcessToJobObject(job, GetCurrentProcess()))
		return PrintError();

	ZeroMemory(&psi, sizeof(psi));
	ZeroMemory(&proc, sizeof(proc));

	psi.cb = sizeof(psi);
	psi.dwFlags = STARTF_USESTDHANDLES;
	psi.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &psi, &proc))
		return PrintError();

	WaitForSingleObject(proc.hProcess, INFINITE);
	CloseHandle(proc.hThread);
	CloseHandle(proc.hProcess);

	if (NULL != job)
		CloseHandle(job);

	DeleteFile(fullPath);

	return 0;
}
#endif

void Print(LPSTR message, BOOL error)
{
	DWORD chars_written = 0;
	DWORD length = (DWORD) strlen(message);

	WriteConsole(GetStdHandle(error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE), message, length, &chars_written, NULL);
}

int PrintError()
{
	Print(GetErrorMessage(GetLastError()), TRUE);
	
	return -1;
}

LPSTR GetErrorMessage(DWORD error)
{
	LPSTR text = NULL;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
		GetModuleHandle("wininet.dll"),
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&text,
		0,
		NULL);

	return text;
}