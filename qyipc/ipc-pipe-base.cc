#include "ipc-pipe-base.h"
#include "ipc-engine.h"
#include "ipc-define.h"
#include "qyutil/qybytebuffer.h"
#include "qyutil/qylog.h"
#include <sddl.h>

/*
 Enabling Asynchronous Operation
 The ReadFile, WriteFile, TransactNamedPipe, and ConnectNamedPipe functions can be performed asynchronously only if you enable overlapped mode for the specified pipe handle and specify a valid pointer to an OVERLAPPED structure. If the OVERLAPPED pointer is NULL, the function return value can incorrectly indicate that the operation has been completed. Therefore, it is strongly recommended that if you create a handle with FILE_FLAG_OVERLAPPED and want asynchronous behavior, you should always specify a valid OVERLAPPED structure.
 The hEvent member of the specified OVERLAPPED structure must contain a handle to a manual-reset event object. This is a synchronization object created by the CreateEvent function. The thread that initiates the overlapped operation uses the event object to determine when the operation has finished. You should not use the pipe handle for synchronization when performing simultaneous operations on the same handle because there is no way of knowing which operation's completion caused the pipe handle to be signaled. The only reliable technique for performing simultaneous operations on the same pipe handle is to use a separate OVERLAPPED structure with its own event object for each operation. For more information about event objects, see Synchronization.
 Also, you can be notified when an overlapped operation completes by using the GetQueuedCompletionStatus or GetQueuedCompletionStatusEx functions. In this case, you do not need to assign the manual-reset event in the OVERLAPPED structure, and the completion happens against the pipe handle in the same way as an asynchronous read or write operation. For more information, see I/O Completion Ports.
 When ReadFile, WriteFile, TransactNamedPipe, and ConnectNamedPipe operations are performed asynchronously, one of the following occurs:
 If the operation is complete when the function returns, the return value indicates the success or failure of the operation. If an error occurs, the return value is zero and the GetLastError function returns something other than ERROR_IO_PENDING.
 If the operation has not finished when the function returns, the return value is zero and GetLastError returns ERROR_IO_PENDING. In this case, the calling thread must wait until the operation has finished. The calling thread must then call the GetOverlappedResult function to determine the results.
 
 @see http://msdn.microsoft.com/en-us/library/windows/desktop/aa365788(v=vs.85).aspx
 */
std::wstring IPCPipeBase::pipeName(const wchar_t* name)
{
	return std::wstring(L"\\\\.\\pipe\\") + name;
}

const int kWriteQueueCount = 20;
class IPCPipeReadWrite
{
public:
	IPCPipeReadWrite()
	{
		wq_count_ = -1;
		wq_current_ = 0;
		for (int i = 0 ; i < kWriteQueueCount;i++) {
			wq_[i] = new qy::QyByteBuffer((kPACKET_BUFFER_MAX)/4);
		}
	}
	~IPCPipeReadWrite()
	{
		for (int i = 0 ; i < kWriteQueueCount;i++) {
			delete wq_[i];
			wq_[i] = NULL;
		}
		wq_current_ = 0;
		wq_count_ = -1;
	}
	//read buffer
	unsigned char read_buffer[kPACKET_BUFFER_MAX];

	//write buffer
	qy::QyByteBuffer* wq_[kWriteQueueCount];
	int wq_current_;
	int wq_count_;
	qy::QySLCS write_crit_;
};

IPCPipeBase::IPCPipeBase(const IPCConnectConfig_t& cfg,PipeType type,IPCEngine* engine)
:type_(type)
,handler_(INVALID_HANDLE_VALUE)
,connCfg_(cfg)
,engine_(engine)
,rw_(new IPCPipeReadWrite())
,peer_close_(true)
{
	if (cfg.overlapped)
	{
		olwrite_ = (PIPEOVERLAPPED*)malloc(sizeof(PIPEOVERLAPPED));
		olread_  = (PIPEOVERLAPPED*)malloc(sizeof(PIPEOVERLAPPED));

		memset(olwrite_,0,sizeof(PIPEOVERLAPPED));
		memset(olread_ ,0,sizeof(PIPEOVERLAPPED));
	}
}

IPCPipeBase::~IPCPipeBase()
{
	destroyHandler();

	if (olread_)
	{
		free(olwrite_);
		free(olread_);
	}
	olwrite_ = olread_ = NULL;
	delete rw_;
}

void IPCPipeBase::destroyHandler()
{
	if (handler_ != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handler_);
		handler_ = INVALID_HANDLE_VALUE;
	}
}

PIPEOVERLAPPED* IPCPipeBase::overlappedWrite() const
{
	return olwrite_;
}

PIPEOVERLAPPED* IPCPipeBase::overlappedRead() const
{
	return olread_;
}

void IPCPipeBase::setOperation(PIPEOVERLAPPED* ol,int oper)
{
	if (ol)
	{
		ol->operation = oper;
	}
}

void IPCPipeBase::setIndex(PIPEOVERLAPPED* ol,int index)
{
	if (ol)
	{
		ol->index = index;
	}
}

void IPCPipeBase::overCountIncr(PIPEOVERLAPPED* ol,bool err)
{
	if (ol)
	{
		if (err)
		{
			ol->err_counts++;
		}
		else
		{
			ol->counts++;
		}
	}
}


DWORD IPCPipeBase::nextRead()
{
	PIPEOVERLAPPED* overlapped = overlappedRead();
	setOperation(overlapped,PIPEOVERLAPPED::IOCP_NONE);
	overCountIncr(overlapped,false);
	return readFile(rw_->read_buffer,kPACKET_BUFFER_MAX);
}

DWORD IPCPipeBase::readFile(unsigned char* buffer, size_t bufflen)
{
	DWORD dwError = ERROR_SUCCESS;
	DWORD cbRead = 0;
	PIPEOVERLAPPED* ol = overlappedRead();
	setOperation(ol,PIPEOVERLAPPED::IOCP_READ);
	if (!::ReadFile(
			handler(),              // Handle of the pipe
			buffer,                 // Buffer to receive the reply
			bufflen,                // Size of buffer in bytes
			&cbRead,                // Number of bytes read 
			(LPOVERLAPPED)ol        // overlapped 
			))
	{
		dwError = ::GetLastError();
		if (ERROR_IO_PENDING == dwError)
		{
			dwError = ERROR_SUCCESS;
		}
		else
		{
			setOperation(ol,PIPEOVERLAPPED::IOCP_NONE);
			overCountIncr(ol,true);
		}
	}
	return dwError;
}

DWORD IPCPipeBase::nextWrite()
{// send buffer queue to server
	PIPEOVERLAPPED* overlapped = overlappedWrite();
	setOperation(overlapped,PIPEOVERLAPPED::IOCP_NONE);
	if (rw_->wq_current_ >= kWriteQueueCount) 
	{
		rw_->wq_current_ = 0;
	}
	qy::QyByteBuffer* buffer = rw_->wq_[rw_->wq_current_];
	if (!buffer->empty())
	{
		int ret = writeFile((const unsigned char*)buffer->data(),buffer->length());		
		if (0==ret)
		{		
			QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Write data(%d,size=%d) (%d,%d)",pipeId(), buffer->length(),rw_->wq_count_, rw_->wq_current_);
			overCountIncr(overlapped,false);
		}
		else
		{
			QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Write failure data(%d,size=%d) (%d,%d)",pipeId(),buffer->length(),rw_->wq_count_, rw_->wq_current_);
		}
		return ret;
	}
	return 0;
}

DWORD IPCPipeBase::writeFile(const unsigned char* buffer, size_t bufflen)
{
	DWORD cbWritten = 0;
	DWORD dwError = ERROR_SUCCESS;
	PIPEOVERLAPPED* ol = overlappedWrite();
	setOperation(ol,PIPEOVERLAPPED::IOCP_WRITE);
	if (!::WriteFile(
		handler(),                      // Handle of the pipe
		buffer,                     // Message to be written
		bufflen,                    // Number of bytes to write
		&cbWritten,                 // Number of bytes written
		(LPOVERLAPPED)ol   // overlapped
		))
	{
		dwError = ::GetLastError();		
		if (ERROR_IO_PENDING == dwError)
		{// Need to GetOverlappedResult
			dwError = ERROR_SUCCESS;
		}
		else
		{
			setOperation(ol,PIPEOVERLAPPED::IOCP_NONE);
			overCountIncr(ol,true);
		}
	}
	return dwError;
}


DWORD IPCPipeBase::write(const unsigned char* buffer, size_t bufflen)
{// write to buffer queue
	// insert to queue
	qy::QyAutoLocker cs(&rw_->write_crit_);
	int index = rw_->wq_count_;
	if (rw_->wq_count_ >= kWriteQueueCount || rw_->wq_count_<0)
	{
		index = 0;
	}
	
	if (!rw_->wq_[index]->empty()){
		//queue full,not insert,数据丢包
		nextWrite();
		onStatus(isClient()?kIPC_CS_CLIENT_LOSTDATA:kIPC_CS_SERVER_LOSTDATA,0);
		return kIPC_RES_LOSSDATA;
	}
	// write head unsigned int 4 bytes,AND when read it, read 4 bytes = bufflen, to guarantee the integrity
	rw_->wq_[index]->writeUInt32(bufflen);
	rw_->wq_[index]->write((const char*)buffer,bufflen);
	rw_->wq_count_ = ++index;	
	
	if (isWritting())
	{	
		QY_LOG(qy::kQyLogDev, qy::kQyLogWarning_LL, L"[IPC]Push data And is Writting(%d)",pipeId());
		return 0;
	}
	return nextWrite();
}

DWORD IPCPipeBase::read()
{
	if (isReading())
	{
		return 0;
	}
	return nextRead();
}

// 工作线程流程
int IPCPipeBase::onCompletionStatus(unsigned int numberOfBytesTransferred, PIPEOVERLAPPED* ol, unsigned long err)
{
	DWORD dwErr = 0;
	if (ol && 0==err)
	{
		switch(ol->operation)
		{
		case PIPEOVERLAPPED::IOCP_WRITE://发送
			{
				qy::QyAutoLocker cs(&rw_->write_crit_);
				rw_->wq_[rw_->wq_current_]->clear();
				rw_->wq_current_++;
				dwErr = nextWrite();
			}
			break;

		case PIPEOVERLAPPED::IOCP_READ://接收
			if (numberOfBytesTransferred >0 && numberOfBytesTransferred < kPACKET_BUFFER_MAX)
			{
				rw_->read_buffer[numberOfBytesTransferred] = '\0';
				packetIntegrity(rw_->read_buffer, numberOfBytesTransferred);			
			}
			dwErr = nextRead(); // next
			break;

		case PIPEOVERLAPPED::IOCP_CONN:
			setOperation(ol,PIPEOVERLAPPED::IOCP_NONE);
			onStatus(kIPC_CS_SERVER_CONNECTED,0);
			break;
		default:
			break;
		}
	}

	if (0 != err)
	{
		onError(err);
	}
	else if(dwErr != 0)
	{
		onError(dwErr);
	}	
	return 0;
}

void IPCPipeBase::onStatus(long status,long err)
{
	switch(status)
	{
	case kIPC_CS_CLIENT_CONNECTED:
	case kIPC_CS_SERVER_CONNECTED:
		read();
		break;
	}
	engine()->onStatus(pipeId(), status, err);
}

void IPCPipeBase::onError(unsigned long err)
{
	switch(err)
	{
	case ERROR_BROKEN_PIPE:
	case ERROR_PIPE_NOT_CONNECTED:
		onStatus(isPeerClose() ? kIPC_CS_PEER_CLOSE : kIPC_CS_CLOSE, err);
		break;

	case ERROR_NO_DATA:
		onStatus(kIPC_CS_CLOSE,err);
		break;

	default:
		onStatus(kIPC_CS_ERROR,err);
		break;
	}
	CancelIoEx(handler(),NULL);
}

IPCEngine* IPCPipeBase::engine() const
{
	return engine_;
}

bool IPCPipeBase::isPeerClose() const
{
	return peer_close_;
}

bool IPCPipeBase::isWritting() const
{
	if (olwrite_)
	{
		DWORD unused = 0;
		if (!::GetOverlappedResult(handler(),olwrite_, &unused, FALSE))
		{//没有完成,还在操作
			if (GetLastError()==ERROR_IO_INCOMPLETE)
				return true;
		}
		return (olwrite_->operation != PIPEOVERLAPPED::IOCP_NONE);
	}
	return false;
}

bool IPCPipeBase::isReading() const 
{
	return olread_?(olread_->operation != PIPEOVERLAPPED::IOCP_NONE):false;
}

void IPCPipeBase::packetIntegrity(const unsigned char* bytes, size_t bytesLen)
{// 检测包的完整性
	if (bytesLen == 0 || bytesLen == (uint32)-1)
		return ;

	const unsigned char* _bytes = bytes;
	uint32 _bytesLen = bytesLen;
	uint32 _totalLen = 0;

	while (_totalLen < bytesLen)
	{
    	uint32 buffLen  = *((uint32*)_bytes);
		uint32 totalLen = _bytesLen - sizeof(uint32);
		uint32 packetLen= buffLen + sizeof(uint32);
		if (buffLen == totalLen)
		{
			engine()->onRecv(pipeId(), _bytes+sizeof(uint32), buffLen); 
		}
		else if(buffLen > totalLen)
		{//不完整,to store the data
			QY_LOG(qy::kQyLogDev, qy::kQyLogErr_LL, L"[IPC]Data packet is not integrity(%d)",pipeId());
			break;
		}
		else
		{//折包
			_bytesLen = packetLen;
			continue;
		}

		{//后移			
			_totalLen += packetLen;
			_bytes    += packetLen; 
			_bytesLen  = bytesLen - packetLen; //余下长度
		}
	}
}