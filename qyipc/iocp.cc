#include "ipc-pipe-base.h"
#include "iocp.h"

const unsigned int kQuitIOCP = 0xFFFFFFFF;

IOCP::IOCP()
    : iocp_fd_(NULL)
	, run_flag_(false)	
{	
	thread_ = qy::QyThread::createThread((qy::QyThreadRunFunction)&IOCP::preRun,this);
}

IOCP::~IOCP()
{
	destroy();
	delete thread_;
}
    
int IOCP::open(void)
{    
	if (!iocp_fd_)
	{
		if((iocp_fd_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0)) == NULL)
		{		
			return kIPC_RES_CREATE_IO_FAIL;
		}
	}
	// to do start thread
	thread_->start();

    return kIPC_RES_OK;
}

void IOCP::close(void)
{
	if (!postQueuedCompletionStatus(kQuitIOCP, NULL))
	{//if thread is read write pipe，PostQueuedCompletionStatus will cause thread to exit.
		destroy();
	}
	// to do stop thread
	thread_->stop();
}

bool IOCP::postQueuedCompletionStatus(ULONG_PTR dwCompletionKey,LPOVERLAPPED ol)
{
	if (isRuned())
	{
		return ::PostQueuedCompletionStatus(iocp(), 0, dwCompletionKey, ol);
	}
	return false;
}

int IOCP::preRun(QyThreadObj obj,qy::eThreadEvent e)
{
	IOCP *io = (IOCP *)obj;
	return io->run(e);
}

int IOCP::run(qy::eThreadEvent e)
{    
	if (qy::kTStoped == e)
	{
		closeIocp();
		return qy::kTResExit;
	}

    run_flag_ = true;
    for(;;)
    {
		DWORD error = 0;
		DWORD numberOfBytesTransferred = 0;
		DWORD key = 0;
		PIPEOVERLAPPED* ol = NULL;
		if (!GetQueuedCompletionStatus(iocp(), &numberOfBytesTransferred,(PULONG_PTR)&key, (LPOVERLAPPED*)&ol, INFINITE))
		{
			error = ::GetLastError();
		}

        if (key == kQuitIOCP)
        {
            break;
        }
		else
		{
			int ret = onCompletionStatus(key, numberOfBytesTransferred, ol, error);
			if (ret == 0)
			{//存在相关的key
			}        
			else
			{//
			}
		}
	}
	closeIocp();
	run_flag_ = false;
	return qy::kTResExit;
}
        
int IOCP::onCompletionStatus(DWORD dwKey,DWORD numberOfBytesTransferred,PIPEOVERLAPPED* ol, unsigned long error)
{
    const CONNECTID_PTR connId = (const CONNECTID_PTR)dwKey; 
    PipeMap_t::iterator iter = pipes_.find(connId);
    if (iter != pipes_.end()) 
	{
        return iter->second->onCompletionStatus(numberOfBytesTransferred,ol,error);
    }
    return 1;
}

int IOCP::attach(IPCPipeBase *pipe)
{
    qy::QyAutoLocker cs(&crit_);
    const CONNECTID_PTR connId = pipe->pipeId();
    PipeMap_t::iterator iter = pipes_.find(connId);
    if (iter != pipes_.end())
	{
        return (iter->second != pipe)?kIPC_RES_PIPE_EXISTED:kIPC_RES_OK;
    }
    int ret = pipe->create();
    if (kIPC_RES_OK != ret)
    {
        return ret;
    }
	if (::CreateIoCompletionPort(pipe->handler(), iocp(), (ULONG_PTR)connId, 0) == NULL)
	{
		return kIPC_RES_CREATE_IO_FAIL;
	}
	pipes_.insert(std::make_pair(connId,pipe));
	return kIPC_RES_OK;
}

void IOCP::deatch(const CONNECTID_PTR id)
{
    qy::QyAutoLocker cs(&crit_);
	PipeMap_t::iterator iter = pipes_.find(id);
	if(pipes_.end() != iter)
	{
		IPCPipeBase* pipe = iter->second;
		pipes_.erase(iter);

		pipe->destroy();

		delete pipe;
	}
}

bool IOCP::check(const CONNECTID_PTR id)
{
    qy::QyAutoLocker cs(&crit_);
    PipeMap_t::iterator iter = pipes_.find(id);
    if (iter != pipes_.end())
	{
		return iter->second->isCreated();
	}
	return false;
}

void IOCP::destroy()
{
	qy::QyAutoLocker cs(&crit_);
	PipeMap_t::iterator iter = pipes_.begin();
	while(pipes_.end() != iter)
	{
		IPCPipeBase* pipe = iter->second;
		++iter;

		pipe->destroy();
		
		delete pipe;
	}
	pipes_.clear();
}
    
HANDLE IOCP::iocp() const
{
	return iocp_fd_;
}

void IOCP::closeIocp()
{
	if(iocp_fd_ != NULL) 
	{
		CloseHandle(iocp_fd_);
		iocp_fd_ = NULL;	
	}
}

bool IOCP::isRuned(void) const
{
	return run_flag_;
}

int IOCP::send(const CONNECTID_PTR id,const unsigned char* data, size_t len)
{
   qy::QyAutoLocker cs(&crit_);
    PipeMap_t::iterator iter = pipes_.find(id);
    if (iter != pipes_.end())
	{
		return iter->second->write(data,len);
	}
	return kIPC_RES_NOTFIND_CONNECTED;
}
