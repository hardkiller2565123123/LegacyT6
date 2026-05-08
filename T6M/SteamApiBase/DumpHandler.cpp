#include "STDInc.h"
#include <mmsystem.h>

#pragma comment(lib, "Winmm.lib")

Hook::Stomp SetUnhandledExceptionFilter_Hook;

void DumpHandler::Initialize()
{
	SetUnhandledExceptionFilter(&DumpHandler::CustomUnhandledExceptionFilter);
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI DumpHandler::SetUnhandledExceptionFilter_Stub(
	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	UNREFERENCED_PARAMETER(lpTopLevelExceptionFilter);

	return SetUnhandledExceptionFilter(&DumpHandler::CustomUnhandledExceptionFilter);
}

LONG WINAPI DumpHandler::CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord)
		return EXCEPTION_CONTINUE_SEARCH;

	DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

	if (exceptionCode == EXCEPTION_BREAKPOINT)
		return EXCEPTION_CONTINUE_EXECUTION;

	if (exceptionCode == 0x80000004)
		return EXCEPTION_CONTINUE_EXECUTION;

	char filename[MAX_PATH] = { 0 };
	char error[1024] = { 0 };

	__time64_t rawTime = 0;
	tm* localTime = NULL;

	_time64(&rawTime);
	localTime = _localtime64(&rawTime);

	if (localTime)
	{
		strftime(
			filename,
			sizeof(filename) - 1,
			"PlusOpsII - %H-%M-%S %d.%m.%Y.dmp",
			localTime);
	}
	else
	{
		strcpy_s(filename, sizeof(filename), "PlusOpsII.dmp");
	}

	_snprintf(
		error,
		sizeof(error) - 1,
		"A minidump has been written to %s.",
		filename);

	HANDLE hFile = CreateFileA(
		filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ex = { 0 };

		ex.ThreadId = GetCurrentThreadId();
		ex.ExceptionPointers = ExceptionInfo;
		ex.ClientPointers = FALSE;

		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpNormal,
			&ex,
			NULL,
			NULL);

		CloseHandle(hFile);
	}
	else
	{
		_snprintf(
			error,
			sizeof(error) - 1,
			"An error (0x%x) occurred while creating %s.",
			GetLastError(),
			filename);
	}

	MessageBoxA(
		0,
		hString::va(
			"Fatal error (0x%08x) at 0x%08x.\n%s",
			exceptionCode,
			ExceptionInfo->ExceptionRecord->ExceptionAddress,
			error),
		"ERROR",
		MB_ICONERROR);

	TerminateProcess(GetCurrentProcess(), exceptionCode);

	return EXCEPTION_EXECUTE_HANDLER;
}

DWORD WINAPI DumpHandler::SafeTimeGetTime()
{
	return timeGetTime();
}