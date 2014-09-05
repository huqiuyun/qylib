#pragma once

#include <stdio.h>
#include <windows.h>
#include "ipc-pipe-client.h"
#include "ipc-engine.h"
#include "ipc-define.h"

IPCPipeClient::IPCPipeClient(const IPCConnectConfig_t& cfg,IPCEngine* engine)
:IPCPipeBase(cfg,PipeType::PType_client,engine)
{
}

IPCPipeClient::~IPCPipeClient()
{
}


int IPCPipeClient::create()
{
	if (isCreated())
	{
		return 0;
	}
	peer_close_ = true;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	DWORD dwError = ERROR_SUCCESS;

	onStatus(kIPC_CS_CLIENT_CONNECTING,0);

	int reconnectCount = 0;
	std::wstring name = pipeName(connCfg_.name.c_str());
	// Try to open the named pipe identified by the pipe name.
	while (TRUE) 
	{
		hPipe = ::CreateFile( 
			name.c_str(),                 // Pipe name 
			GENERIC_READ | GENERIC_WRITE,   // Read and write access
			0,                              // No sharing 
			NULL,                           // Default security attributes
			OPEN_EXISTING,                  // Opens existing pipe
			isOverlappend()?FILE_FLAG_OVERLAPPED:0,           // Default attributes
			NULL                            // No template file
			);

		// If the pipe handle is opened successfully ...
		if (hPipe != INVALID_HANDLE_VALUE)
		{ 
			dwError = ERROR_SUCCESS;
			break;
		}

		dwError = GetLastError();
		// Exit if an error other than ERROR_PIPE_BUSY occurs.
		if (ERROR_PIPE_BUSY != dwError)
		{
			if (reconnectCount++ > 10)
			{
				break;
			}
			else
			{
				Sleep(200);
				continue;
			}
		}
        if (!WaitNamedPipe(name.c_str(),5000))
        {
            break;
        }
	}

	if (dwError != ERROR_SUCCESS)
	{	// Centralized cleanup for all allocated resources.
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}
		onStatus(kIPC_CS_CLIENT_ERROR,dwError);
	}
	else
	{//
		handler_ = hPipe;
		onStatus(kIPC_CS_CLIENT_CONNECTED,0);
	}
	return dwError;
}

void IPCPipeClient::destroy()
{
	destroyHandler();	
}
