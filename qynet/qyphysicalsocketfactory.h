#ifndef QY_PHSICALSOCKETFACTORY_H__
#define QY_PHSICALSOCKETFACTORY_H__

#include "qysocketfactory.h"

namespace qy 
{ 
	// A socket server that provides the real sockets of the underlying OS.
	class QyPhysicalSocketFactory : public QySocketFactory
	{
	public:
		QyPhysicalSocketFactory();
		virtual ~QyPhysicalSocketFactory();

        virtual QySocket* createSocket(int type);
        virtual QyAsyncSocket* createAsyncSocket(int type);
	};

} // namespace qy

#endif // QY_PHSICALSOCKETFACTORY_H__
