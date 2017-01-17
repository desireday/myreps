
#include "EncryptorHandler.h"
#include "encryptor_main2.h"
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

namespace android {

void EncryptorHandler::instantiate() {
	ALOGE("EncryptorHandler::instantiate,");
	defaultServiceManager()->addService(String16("EncryptorHandler"), new EncryptorHandler());
}

void EncryptorHandler::printVersion(String16 ver_info){

	ver_show(ver_info);

}

}

