#include <windows.h>
#include <wininet.h>
#include <stdio.h>

void Print(LPSTR message, BOOL error);
void PrintError();
LPSTR GetErrorMessage(DWORD error);

int main(int argc, char *argv[])
{
	HINTERNET hInternetSession, hHttpSession, hHttpRequest;
	PCTSTR acceptTypes[] = { "text/*", NULL};
	char* lpBuffer[256] = { 0 };
	DWORD dwBytesRead = 0, dwBytesAvailable = 0;
	BOOL asJson = argc == 2 && strcmp(tolower(argv[1]), "-json\n");

	hInternetSession = InternetOpen("pubip", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (NULL == hInternetSession)
		PrintError();

	hHttpSession = InternetConnect(hInternetSession, "api.ipify.org", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	if (NULL == hHttpSession)
		PrintError();

	hHttpRequest = HttpOpenRequest(hHttpSession, NULL, asJson ? "?format=json" : "", NULL, NULL, acceptTypes, INTERNET_FLAG_RELOAD, 0);

	if (NULL == hHttpRequest)
		PrintError();

	if (!HttpSendRequest(hHttpRequest, NULL, 0, NULL, 0))
		PrintError();

	while (InternetQueryDataAvailable(hHttpRequest, &dwBytesAvailable, 0, 0) && dwBytesAvailable > 0)
	{		
		ZeroMemory(lpBuffer, 256);

		if (!InternetReadFile(hHttpRequest, (LPVOID)lpBuffer,2000, &dwBytesRead))
			PrintError();

		Print((LPSTR)lpBuffer, FALSE);
	}

	InternetCloseHandle(hInternetSession);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hHttpRequest);

	return 0;
}

void Print(LPSTR message, BOOL error)
{
	DWORD chars_written = 0;
	DWORD length = (DWORD) strlen(message);

	WriteConsole(GetStdHandle(error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE), message, length, &chars_written, NULL);
}

void PrintError()
{
	Print(GetErrorMessage(GetLastError()), TRUE);
	
	ExitProcess(-1);
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