
#include "EncryptorRegistrar.h"
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <encryptorservice/IEncryptorHandler.h>
#include "encryptor_main1.h"

namespace android {

void EncryptorRegistrar::instantiate() {
	ALOGE("EncryptorRegistrar::instantiate,");
	defaultServiceManager()->addService(String16("EncryptorRegistrar"), new EncryptorRegistrar());
}

void EncryptorRegistrar::setHandler(String16 name){
	ALOGE("EncryptorRegistrar,received from main2 service name:%s",String8(name).string());
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder = sm->getService(name);
	if(binder == NULL){
		ALOGE("can not get EncryptorHandler binder");
		return ;
	}
	sp<IEncryptorHandler> handler_sdtm = interface_cast<IEncryptorHandler>(binder);
	if(handler_sdtm == NULL){
		ALOGE("can not convert binder to IService");
		return ;
	}
	SetHandler(handler_sdtm);
	return ;
}

void EncryptorRegistrar::sendCmd(String16 cmd){
	ALOGE("EncryptorRegistrar,received from main2 Cmd name:%s",String8(cmd).string());
	SendCmdToSecureModule();
}

}

