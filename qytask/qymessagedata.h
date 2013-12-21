#ifndef QY_MESSAGEDATA_H__
#define QY_MESSAGEDATA_H__

#include "qytask-config.h"

DEFINE_NAMESPACE(qy)

// Derive from this for specialized data
// App manages lifetime, except when messages are purged
class QyMessageData
{
public:
    QyMessageData() { mType = 0;}
    virtual ~QyMessageData() {}
    unsigned int mType;
};

template <class T>
class QyTypedMessageData : public QyMessageData
{
public:
    QyTypedMessageData(const T& data) : mData(data) { }
    const T& data() const { return mData; }
    T& data() { return mData; }
private:
    T mData;
};

template<class T>
inline QyMessageData* WrapMessageData(const T& data) {
    return new QyTypedMessageData<T>(data);
}

template<class T>
inline const T& UseMessageData(QyMessageData* data) {
    return static_cast<QyTypedMessageData<T>* >(data)->data();
}

template<class T>
class QyDisposeData : public QyMessageData
{
public:
    QyDisposeData(T* data) : mData(data) { }
    virtual ~QyDisposeData() { delete mData; }
    const T* data() const { return mData; }
    T* data() { return mData; }
private:
    T* mData;
};

template<class T>
class QyReleaseData : public QyMessageData
{
public:
    QyReleaseData(T* data) {
        mData = data;
        if (mData) mData->AddRef();
	}
    virtual ~QyReleaseData(){
        if (mData){
            mData->Release();
            mData=NULL;
		} 
	}
    const T* data() const { return mData; }
    T* data() { return mData; }
private:
    T* mData;
};

template<class T>
inline T* UseMessageRData(QyMessageData* data) {
    return static_cast<QyReleaseData<T>* >(data)->data();
}

template<class T>
inline const T* UseMessageCRData(QyMessageData* data) {
    return static_cast<QyReleaseData<T>* >(data)->data();
}

END_NAMESPACE(qy)

#endif // QY_MESSAGEDATA_H__
