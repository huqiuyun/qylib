#ifndef QY_PHSICALSOCKETFACTORY_H__
#define QY_PHSICALSOCKETFACTORY_H__

#include "qysocketfactory.h"

namespace qy 
{ 
    class QyDispatcher;
    class QyPhysicalSocketFactoryPrivate;
	// A socket server that provides the real sockets of the underlying OS.
	class QyPhysicalSocketFactory : public QySocketFactory
	{
	public:
        QyPhysicalSocketFactory(EventMode mode = EM_SELECT);
		virtual ~QyPhysicalSocketFactory();

        virtual QySocket* createSocket(int type);
        virtual QyAsyncSocket* createAsyncSocket(int type);

        virtual bool wait(unsigned long cms,bool threadrun);
        virtual void wake();

        virtual void add(QyDispatcher* dispatcher);
        virtual void remove(QyDispatcher* dispatcher);
    private:
        QyPhysicalSocketFactoryPrivate* d_ptr;
	};

} // namespace qy

#endif // QY_PHSICALSOCKETFACTORY_H__
