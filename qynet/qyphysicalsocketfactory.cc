#include "qyPhysicalSocketFactory.h"
#include "qyphysicalsocket.h"

#ifdef H_OS_WIN
class WinsockInitializer
{
public:
	WinsockInitializer() 
	{
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(1, 0);
		err_ = WSAStartup(wVersionRequested, &wsaData);
	}
	~WinsockInitializer() 
	{
		WSACleanup();
	}
	int error() {
		return err_;
	}
private:
	int err_;
};
WinsockInitializer g_winsockinit;
#endif

namespace qy 
{

    QyPhysicalSocketFactory::QyPhysicalSocketFactory()
    {
	}

    QyPhysicalSocketFactory::~QyPhysicalSocketFactory()
    {
	}

    QySocket* QyPhysicalSocketFactory::createSocket(int type)
	{
		PhysicalSocket* socket = new PhysicalSocket(this);
        if (socket->open(type))
		{
			return socket;
		}
		else
		{
			delete socket;
			return 0;
		}
    }

    QyAsyncSocket* QyPhysicalSocketFactory::createAsyncSocket(int type)
	{
        HUNUSED(type);
        return 0;
	}

} // namespace qy
