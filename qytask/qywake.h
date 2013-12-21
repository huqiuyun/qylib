#ifndef QY_WAKE_H__
#define QY_WAKE_H__

namespace qy
{
    class QyWake
	{
	public:
        virtual ~QyWake(){}
        virtual void wake(void) = 0;
		virtual bool wait(unsigned long cms,bool threadrun) = 0;
	};
}

#endif //QY_WAKE_H__

