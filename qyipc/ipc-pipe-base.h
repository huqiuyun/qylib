#pragma once

#include <windows.h>
#include "ipc-engine-protocol.h"

struct PIPEOVERLAPPED : public OVERLAPPED
{
	enum Operation {IOCP_NONE = 0 ,IOCP_CONN,IOCP_WRITE,IOCP_READ};
	int  operation;
	int  index;
	__int64 counts; ///<
	__int64 err_counts;
};

class IPCPipeReadWrite;
class IPCEngine;
class IPCPipeBase
{
public:
	enum PipeType {PType_client = 0, PType_server = 1 };

	IPCPipeBase(const IPCConnectConfig_t& cfg,PipeType type,IPCEngine* engine);
	virtual ~IPCPipeBase();

public:

	bool isCreated() const {
		return (handler_ != INVALID_HANDLE_VALUE);
	}

	bool isClient() const {
		return (type_ == PType_client);
	}

	HANDLE handler() const {
		return handler_;
	}

	const CONNECTID_PTR pipeId() const{
		return CONNECTID_TO_PTR(connCfg_.connId);
	}

	bool isWritting() const;
	bool isReading() const;
	bool isOverlappend() const {
		return (olread_ && olwrite_); 
	}

	DWORD write(const unsigned char* buffer, size_t bufflen);	
	DWORD read();

	static std::wstring pipeName(const wchar_t* name); 
public:
	virtual int  create() = 0;
	virtual void destroy() = 0;

protected:
	void destroyHandler();

	IPCEngine* engine() const;
	PIPEOVERLAPPED* overlappedWrite() const;
	PIPEOVERLAPPED* overlappedRead() const;
	void setOperation(PIPEOVERLAPPED* ol,int operation);
	void setIndex(PIPEOVERLAPPED* ol,int index);
	void overCountIncr(PIPEOVERLAPPED* ol,bool err);
	//
	DWORD nextRead();
	DWORD readFile(unsigned char* buffer, size_t bufflen);
	DWORD nextWrite();	
	DWORD writeFile(const unsigned char* buffer, size_t bufflen);

	void onStatus(long status,long err);
	bool isPeerClose() const;
	int onCompletionStatus(unsigned int numberOfBytesTransferred, PIPEOVERLAPPED* ol,unsigned long err);
private:
	void onError(unsigned long err);
	void packetIntegrity(const unsigned char* bytes , size_t bytesLen);
protected:
	friend class IOCP;
	HANDLE   handler_;
	IPCConnectConfig_t connCfg_;
	PipeType type_;
	PIPEOVERLAPPED* olwrite_;
	PIPEOVERLAPPED* olread_;
	IPCEngine* engine_;
	bool peer_close_;
	IPCPipeReadWrite* rw_;
};
