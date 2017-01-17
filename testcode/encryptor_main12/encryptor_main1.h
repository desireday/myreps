#ifndef ENCRYPTOR_MAIN1_H
#define ENCRYPTOR_MAIN1_H

#include <sys/types.h>
#include <unistd.h>
#include <encryptorservice/IEncryptorHandler.h>

using namespace android;

void SetHandler(sp<IEncryptorHandler>& handler);
void SendCmdToSecureModule();

#endif

