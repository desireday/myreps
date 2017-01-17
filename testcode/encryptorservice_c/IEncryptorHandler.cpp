
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <encryptorservice/IEncryptorHandler.h>

namespace android {

class BpEncryptorHandler : public BpInterface<IEncryptorHandler>
{
public:
    BpEncryptorHandler(const sp<IBinder>& impl)
        : BpInterface<IEncryptorHandler>(impl) {}
        
        void printVersion(String16 ver_info) {
            Parcel data,reply;
            data.writeInterfaceToken(IEncryptorHandler::getInterfaceDescriptor());
            data.writeString16(ver_info);
            remote()->transact(PRINT_VERSION, data, &reply, IBinder::FLAG_ONEWAY);
        }

};

IMPLEMENT_META_INTERFACE(EncryptorHandler, "android.os.EncryptorHandler");

status_t BnEncryptorHandler::onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags){
	switch(code){
		case PRINT_VERSION:{
			CHECK_INTERFACE(IEncryptorHandler, data, reply);
			String16 ver_info = data.readString16();
			printVersion(ver_info);
			return OK;
		}
			break;
		default:
			return BBinder::onTransact(code,data,reply,flags);
	}
}

}

