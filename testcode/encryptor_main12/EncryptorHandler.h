#ifndef ANDROID_ENCRYPTORHANDLER_H
#define ANDROID_ENCRYPTORHANDLER_H

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "encryptor-handler-service"

#include <binder/IBinder.h>
#include <utils/String16.h>
#include <encryptorservice/IEncryptorHandler.h>

namespace android {

class EncryptorHandler : public BnEncryptorHandler {
public:
	static void instantiate();
	
private:
	void printVersion(String16 ver_info);
};

}

#endif

