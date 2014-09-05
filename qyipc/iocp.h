#pragma once

#include <map>
#include <windows.h>
#include "qyutil/qylock.h"
#include "qyutil/qythread.h"

// Íê³É¶Ë¿Ú
class IOCP
{	
public:
    IOCP();
    ~IOCP();    
public:
    int  open(void);
	void close();
    
	int  attach(IPCPipeBase *pipe);    
    void deatch(const CONNECTID_PTR id);
    bool check(const CONNECTID_PTR id);
	int  send(const CONNECTID_PTR connId,const unsigned char* data,size_t len);
    bool isRuned(void) const;   	
	bool postQueuedCompletionStatus(ULONG_PTR dwCompletionKey,LPOVERLAPPED ol);

	// QyThreadRunFunction
	static int preRun(QyThreadObj obj,qy::eThreadEvent e);
private:
	int run(qy::eThreadEvent e); 

	HANDLE iocp() const;
	void   closeIocp();
	void   destroy();
    int    onCompletionStatus(DWORD key,DWORD numberOfBytesTransferred,PIPEOVERLAPPED* ol, unsigned long error);
private:    
	typedef std::map<CONNECTID, IPCPipeBase*> PipeMap_t; 
    PipeMap_t      pipes_;
    HANDLE         iocp_fd_;
    bool           run_flag_;  
	qy::QySLCS     crit_;
	qy::QyThread*  thread_;
};
