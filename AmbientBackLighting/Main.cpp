#pragma once

#include "stdafx.h"
#include <windows.h>
#include "AmbientBackLighting.h"


SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE StatusHandle = NULL;
HANDLE ServiceStopEvent = INVALID_HANDLE_VALUE;

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

const LPWSTR SERVICE_NAME = LPWSTR("Ambient Backlighting");

int _tmain(int argc, TCHAR* argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
	{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		return GetLastError();
	}

	return 0;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	DWORD Status = E_FAIL;

	OutputDebugString(_T("My Sample Service: ServiceMain: Entry"));

	StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (StatusHandle == NULL)
	{
		OutputDebugString(_T("My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error"));
		return;
	}

	// Tell the service controller we are starting
	ZeroMemory(&ServiceStatus, sizeof(ServiceStatus));
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwControlsAccepted = 0;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(StatusHandle, &ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	/*
	* Perform tasks necessary to start the service here
	*/
	OutputDebugString(_T("My Sample Service: ServiceMain: Performing Service Start Operations"));

	// Create stop event to wait on later.
	ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ServiceStopEvent == NULL)
	{
		OutputDebugString(_T("My Sample Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

		ServiceStatus.dwControlsAccepted = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = GetLastError();
		ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(StatusHandle, &ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}
		return;
	}

	// Tell the service controller we are started
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(StatusHandle, &ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	// Start the thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	OutputDebugString(_T("My Sample Service: ServiceMain: Waiting for Worker Thread to complete"));

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	OutputDebugString(_T("My Sample Service: ServiceMain: Worker Thread Stop Event signaled"));


	/*
	* Perform any cleanup tasks
	*/
	OutputDebugString(_T("My Sample Service: ServiceMain: Performing Cleanup Operations"));

	CloseHandle(ServiceStopEvent);

	ServiceStatus.dwControlsAccepted = 0;
	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(StatusHandle, &ServiceStatus) == FALSE)
	{
		OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}
	return;
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		* Perform tasks necessary to stop the service here
		*/

		ServiceStatus.dwControlsAccepted = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(StatusHandle, &ServiceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error"));
		}

		// This will signal the worker thread to start shutting down
		SetEvent(ServiceStopEvent);

		break;

	default:
		break;
	}
}



DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	Sleep(10000);

	auto BackLighting = AmbientBackLighting{};

	/*auto FailOut = [&](unsigned int i)
	{
		for (; i < 3 * 32 + 2; i += 3)
		{
			TopColorBuffer[i] = 80;
		}

		if (auto* TopDeviceHandle = hid_open(0x20a0, 0x41e5, TEXT("BS021580-3.1"))) //TODO: find these values in the config.
		{
			auto result = hid_send_feature_report(TopDeviceHandle, TopColorBuffer, 32 * 3 + 2);
			hid_close(TopDeviceHandle);
		}
	};

	//TODO: try to run as a service
	auto ThreadId = GetCurrentThreadId();
	auto CurrentDesktop = GetThreadDesktop(ThreadId);

	auto Station = OpenWindowStation(TEXT("winsta0"), false, READ_CONTROL|WRITE_DAC);
	if (GetLastError() != 0)
	{
		FailOut(2); //g
		return;
	}
	if (Station == nullptr)
	{
		FailOut(2); //g
		return;
	}

	auto SetStationSuccess = SetProcessWindowStation(Station);
	if (SetStationSuccess == false)
	{
		FailOut(3); //r
		return;
	}

	auto NewDesktop = OpenDesktop(TEXT("default"), 0, false, READ_CONTROL | WRITE_DAC |
		DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS);
	SetThreadDesktop(NewDesktop);

	if (GetLastError() != 0)
	{
		FailOut(4); //b
		return;
	}*/


	const auto RefreshTime = 1000.f / BackLighting.Config.SamplesPerSecond;

	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{

		BackLighting.Update(RefreshTime);

		//TODO: set the timeout based on config.
		Sleep(RefreshTime);
	}

	return ERROR_SUCCESS;
}
