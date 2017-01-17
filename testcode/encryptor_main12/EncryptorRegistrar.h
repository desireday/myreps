#ifndef ANDROID_ENCRYPTORREGISTRAR_H
#define ANDROID_ENCRYPTORREGISTRAR_H

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "encryptor-registrar-service"

#include <binder/IBinder.h>
#include <utils/String16.h>
#include <encryptorservice/IEncryptorRegistrar.h>


namespace android {

class EncryptorRegistrar : public BnEncryptorRegistrar {
public:
	static void instantiate();

private:
	void setHandler(String16 name);
	void sendCmd(String16 cmd);
};

}

#endif
