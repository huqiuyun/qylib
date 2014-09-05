#include "ipc-pipe-server.h"
#include "ipc-engine.h"
#include "ipc-define.h"
#include <sddl.h>

//   FUNCTION: createPipeSecurity(PSECURITY_ATTRIBUTES *)
//
//   PURPOSE: The CreatePipeSecurity function creates and initializes a new 
//   SECURITY_ATTRIBUTES structure to allow Authenticated Users read and 
//   write access to a pipe, and to allow the Administrators group full 
//   access to the pipe.
//
//   PARAMETERS:
//   * ppSa - output a pointer to a SECURITY_ATTRIBUTES structure that allows 
//     Authenticated Users read and write access to a pipe, and allows the 
//     Administrators group full access to the pipe. The structure must be 
//     freed by calling FreePipeSecurity.
//
//   RETURN VALUE: Returns TRUE if the function succeeds.
//
//   EXAMPLE CALL:
//
//     PSECURITY_ATTRIBUTES pSa = NULL;
//     if (createPipeSecurity(&pSa))
//     {
//         // Use the security attributes
//         // ...
//
//         freePipeSecurity(pSa);
//     }
//
BOOL createPipeSecurity(PSECURITY_ATTRIBUTES *ppSa)
{
	BOOL fSucceeded = TRUE;	

	PSECURITY_DESCRIPTOR pSd = NULL;
	PSECURITY_ATTRIBUTES pSa = NULL;

	// Define the SDDL for the security descriptor.
	PCWSTR szSDDL = L"D:"       // Discretionary ACL
		L"(A;OICI;GRGW;;;AU)"   // Allow read/write to authenticated users
		L"(A;OICI;GA;;;BA)";    // Allow full control to administrators

	if (!ConvertStringSecurityDescriptorToSecurityDescriptor(szSDDL, 
		SDDL_REVISION_1, &pSd, NULL))
	{
		fSucceeded = FALSE;
		goto Cleanup;
	}

	// Allocate the memory of SECURITY_ATTRIBUTES.
	pSa = (PSECURITY_ATTRIBUTES)LocalAlloc(LPTR, sizeof(*pSa));
	if (pSa == NULL)
	{
		fSucceeded = FALSE;
		goto Cleanup;
	}

	pSa->nLength = sizeof(*pSa);
	pSa->lpSecurityDescriptor = pSd;
	pSa->bInheritHandle = FALSE;

	*ppSa = pSa;

Cleanup:
	// Clean up the allocated resources if something is wrong.
	if (!fSucceeded)
	{
		if (pSd)
		{
			LocalFree(pSd);
			pSd = NULL;
		}
		if (pSa)
		{
			LocalFree(pSa);
			pSa = NULL;
		}
	}
	return fSucceeded;
}

//
//   FUNCTION: freePipeSecurity(PSECURITY_ATTRIBUTES)
//
//   PURPOSE: The FreePipeSecurity function frees a SECURITY_ATTRIBUTES 
//   structure that was created by the CreatePipeSecurity function. 
//
//   PARAMETERS:
//   * pSa - pointer to a SECURITY_ATTRIBUTES structure that was created by 
//     the CreatePipeSecurity function. 
//
void freePipeSecurity(PSECURITY_ATTRIBUTES pSa)
{
	if (pSa)
	{
		if (pSa->lpSecurityDescriptor)
		{
			LocalFree(pSa->lpSecurityDescriptor);
		}
		LocalFree(pSa);
	}
}

IPCPipeServer::IPCPipeServer(const IPCConnectConfig_t& cfg,IPCEngine* engine)
:IPCPipeBase(cfg,PipeType::PType_server,engine)
{
	ouput_buffercounts_ = input_buffercounts_ = kPACKET_BUFFER_MAX; // 64K
}

IPCPipeServer::~IPCPipeServer()
{
}

int IPCPipeServer::create()
{
	if (isCreated())
	{
		onStatus(kIPC_CS_SERVER_CONNECTED,0);
		return 0;
	}
	peer_close_ = true;
	int status = kIPC_CS_SERVER_ERROR;

	DWORD dwError = ERROR_SUCCESS;
	PSECURITY_ATTRIBUTES pSa = NULL;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	std::wstring name = pipeName(connCfg_.name.c_str());
	// Prepare the security attributes (the lpSecurityAttributes parameter in 
	// CreateNamedPipe) for the pipe. This is optional. If the 
	// lpSecurityAttributes parameter of CreateNamedPipe is NULL, the named 
	// pipe gets a default security descriptor and the handle cannot be 
	// inherited. The ACLs in the default security descriptor of a pipe grant 
	// full control to the LocalSystem account, (elevated) administrators, 
	// and the creator owner. They also give only read access to members of 
	// the Everyone group and the anonymous account. However, if you want to 
	// customize the security permission of the pipe, (e.g. to allow 
	// Authenticated Users to read from and write to the pipe), you need to 
	// create a SECURITY_ATTRIBUTES structure.
	if (!createPipeSecurity(&pSa))
	{
		dwError = ::GetLastError();        
		goto Cleanup;
	}
	
	// Create the named pipe.
	hPipe = ::CreateNamedPipe(
		name.c_str(),             // Pipe name.
		PIPE_ACCESS_DUPLEX|(isOverlappend()?FILE_FLAG_OVERLAPPED:0),         // The pipe is duplex; both server and 
		// client processes can read from and 
		// write to the pipe
		PIPE_TYPE_BYTE |            // Message type pipe 
		PIPE_READMODE_BYTE |        // Message-read mode 
		PIPE_WAIT,                  // Blocking mode is enabled
		connCfg_.counts,            // Max. instances
		outputBufferCount(),        // Output buffer size in bytes
		inputBufferCount(),         // Input buffer size in bytes
		INFINITE,                   // Time-out interval
		pSa                         // Security attributes
		);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		dwError = ::GetLastError();        
		goto Cleanup;
	}

	// Wait for the client to connect.
	PIPEOVERLAPPED* overlapped = overlappedRead();
	setOperation(overlapped, PIPEOVERLAPPED::IOCP_CONN);
	if (!::ConnectNamedPipe(hPipe, overlapped))
	{
		dwError = ::GetLastError(); 
		switch(dwError)
		{
		case ERROR_IO_PENDING:
			dwError = ERROR_SUCCESS;
			status = kIPC_CS_SERVER_CONNECTING;
			break;

		default:
			status = kIPC_CS_SERVER_ERROR;
			break;
		}			         
	}
	else
	{
		status = kIPC_CS_SERVER_CONNECTING;
	}
	// Client is connected.
Cleanup:

	// Centralized cleanup for all allocated resources.
	if (pSa != NULL)
	{
		freePipeSecurity(pSa);
		pSa = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}
		setOperation(overlapped, PIPEOVERLAPPED::IOCP_NONE);
		onStatus(status,dwError);
	}
	else
	{
		handler_ = hPipe;
		onStatus(status,0);
	}
	return dwError;
}

void IPCPipeServer::destroy()
{
	if (isCreated()) 
	{
		peer_close_ = false;
		::DisconnectNamedPipe(handler());
	}
	destroyHandler();
}

int IPCPipeServer::inputBufferCount() const
{
	return input_buffercounts_;
}

int IPCPipeServer::outputBufferCount() const
{
	return ouput_buffercounts_;
}
