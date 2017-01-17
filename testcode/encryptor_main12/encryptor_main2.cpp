#include <sys/types.h>
#include <unistd.h>
#include <cutils/log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include "EncryptorHandler.h"
#include <encryptorservice/IEncryptorRegistrar.h>
#include <encryptorservice/IEncryptorHandler.h>

#include "encryptor_main2.h"


using namespace android;

static sp<IEncryptorRegistrar> gEncryptorRegistrar;

	void ver_show(String16 ver_info){
		ALOGE("ver show-> %s",String8(ver_info).string());
	}

int main(int argc,char *argv[]) {
		
	EncryptorHandler::instantiate();
		
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder = sm->getService(String16("EncryptorRegistrar"));
	if(binder == NULL){
		ALOGE("can not get EncryptorHandler binder");
		return -1;
	}
	sp<IEncryptorRegistrar> gEncryptorRegistrar = interface_cast<IEncryptorRegistrar>(binder);
	if(gEncryptorRegistrar == NULL){
		ALOGE("can not convert binder to IService");
		return -1;
	}
	
	usleep(10000);
	
	gEncryptorRegistrar->setHandler(String16("EncryptorHandler"));
	
	usleep(10000);
	
	gEncryptorRegistrar->sendCmd(String16("getversion"));
	
		
		
		ProcessState::self()->startThreadPool();
		IPCThreadState::self()->joinThreadPool();
		return 0;
}
