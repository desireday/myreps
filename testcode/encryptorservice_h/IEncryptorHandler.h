#ifndef ANDROID_IENCRYPTORHANDLER_H
#define ANDROID_IENCRYPTORHANDLER_H

#include <binder/IInterface.h>

namespace android {

enum{
	PRINT_VERSION = IBinder::FIRST_CALL_TRANSACTION,
};

class IEncryptorHandler : public IInterface{
public:
		DECLARE_META_INTERFACE(EncryptorHandler);
		virtual void printVersion(String16) = 0;
};

class BnEncryptorHandler : public BnInterface<IEncryptorHandler>{
public:
	virtual status_t onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags = 0);
};

}

#endif
