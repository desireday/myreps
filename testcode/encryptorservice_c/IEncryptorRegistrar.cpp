
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <encryptorservice/IEncryptorRegistrar.h>


namespace android {

class BpEncryptorRegistrar : public BpInterface<IEncryptorRegistrar>
{
public:
    BpEncryptorRegistrar(const sp<IBinder>& impl)
        : BpInterface<IEncryptorRegistrar>(impl) {}

        void setHandler(String16 name) {
            Parcel data,reply;
            data.writeInterfaceToken(IEncryptorRegistrar::getInterfaceDescriptor());
            data.writeString16(name);
            remote()->transact(SET_HANDLER, data, &reply, IBinder::FLAG_ONEWAY);
        }
        
        void sendCmd(String16 name) {
            Parcel data,reply;
            data.writeInterfaceToken(IEncryptorRegistrar::getInterfaceDescriptor());
            data.writeString16(name);
            remote()->transact(SEND_CMD, data, &reply, IBinder::FLAG_ONEWAY);
        }

};

IMPLEMENT_META_INTERFACE(EncryptorRegistrar, "android.os.EncryptorRegistrar");

status_t BnEncryptorRegistrar::onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags){
	switch(code){
		case SET_HANDLER:{
			CHECK_INTERFACE(IEncryptorRegistrar, data, reply);
			String16 argv = data.readString16();
			setHandler(argv);
			return OK;
		}
			break;
		
		case SEND_CMD:{
			CHECK_INTERFACE(IEncryptorRegistrar, data, reply);
			String16 cmd = data.readString16();
			sendCmd(cmd);
			return OK;
		}
			break;
		default:
			return BBinder::onTransact(code,data,reply,flags);
	}
}

}

