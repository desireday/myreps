#ifndef ANDROID_IENCRYPTORREGISTRAR_H
#define ANDROID_IENCRYPTORREGISTRAR_H

#include <binder/IInterface.h>

namespace android {

enum{
	SET_HANDLER = IBinder::FIRST_CALL_TRANSACTION,
	SEND_CMD,
};

class IEncryptorRegistrar : public IInterface{
public:
		DECLARE_META_INTERFACE(EncryptorRegistrar);
		virtual void setHandler(String16 name) = 0;
		virtual void sendCmd(String16 name) = 0;
};

class BnEncryptorRegistrar : public BnInterface<IEncryptorRegistrar>{
	public:
	virtual status_t onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags = 0);
};

}

#endif
