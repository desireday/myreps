#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>

#include <linux/input.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include "encryptor-svr.h"

static Encryptor* mEncryptor;

void assert_fail(const char *file,int line,const char *func,const char *expr){
	ALOGE("assertion failed at file %s,line %d,function %s:",file,line,func);
	ALOGE("%s",expr);
	abort();
}

//int uart_switch_choose(int control);
/* native service impl begin */
const android::String16 IEncryptorService::descriptor("com.sdtm.encryptor.IEncryptorService");

const android::String16& IEncryptorService::getInterfaceDescriptor() const{
	return IEncryptorService::descriptor;
}

IEncryptorService::IEncryptorService(){ ALOGI("IEncryptorService::IEncryptorService()"); }
IEncryptorService::~IEncryptorService(){ ALOGI("IEncryptorService::~IEncryptorService()"); }

status_t EncryptorService::onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags){
	data.checkInterface(this);
	switch(code){
		case SET_HANDLE:{
			String16 argv = data.readString16();
			int ret = setHandleService(argv);
			ASSERT(reply != 0);
			reply->writeInt32(ret);
			return NO_ERROR;
		}
			break;
		case ENABLE:{
			int32_t ret = enable();
			ASSERT(reply != 0);
			reply->writeInt32(ret);
			return NO_ERROR;
		}
			break;
		case DISABLE:
			disable();
			return NO_ERROR;
			break;
		case START:
			startEncrypt();
			return NO_ERROR;
			break;
		case CANCEL:
			cancelEncrypt();
			return NO_ERROR;
			break;
		case SET_TYPE:{
			String16 name = data.readString16();
			int32_t state = data.readInt32();
			int ret = setCodeType(name,state);
			ASSERT(reply != 0);
			reply->writeInt32(ret);
			return NO_ERROR;
		}
			break;
		case RESET_TYPE:{
			int ret = resetCodeType();
			ASSERT(reply != 0);
			reply->writeInt32(ret);
			return NO_ERROR;
		}
			break;
		default:
			return BBinder::onTransact(code,data,reply,flags);
	}
}

int32_t EncryptorService::setHandleService(String16 name){
	ALOGE("encryptor native service,received java service name:%s",String8(name).string());
	sp<IServiceManager> sm = defaultServiceManager();
	ASSERT(sm != 0);
	sp<IBinder> binder = sm->getService(name);
	if(binder == NULL){
		ALOGE("can not get jservice binder");
		return -1;
	}
	sp<IEncryptorJService> handleSvr = interface_cast<IEncryptorJService>(binder);
	if(handleSvr == NULL){
		ALOGE("can not convert binder to IService");
		return -1;
	}
	if(mEncryptor){
		mEncryptor->setHandler(handleSvr);
	}
	return 0;
}

int32_t EncryptorService::enable(void){
	if(mEncryptor){
		return mEncryptor->enable();
	}
	ALOGE("EncryptorService::enable,mEncryptor is NULL?");
	return -1;
}

void EncryptorService::disable(void){
	if(mEncryptor){
		mEncryptor->disable();
		return;
	}
	ALOGE("EncryptorService::disable,mEncryptor is NULL?");
}

void EncryptorService::startEncrypt(void){
	if(mEncryptor){
		mEncryptor->startEncrypt();
		return;
	}
	ALOGE("EncryptorService::startEncrypt,mEncryptor is NULL?");
}

void EncryptorService::cancelEncrypt(void){
	if(mEncryptor){
		mEncryptor->cancelEncrypt();
		return;
	}
	ALOGE("EncryptorService::cancelEncrypt,mEncryptor is NULL?");
}

int32_t EncryptorService::setCodeType(String16 name,int32_t state){
	if(mEncryptor){
		return mEncryptor->setCodeType(name,state);
	}
	ALOGE("EncryptorService::setCodeType,mEncryptor is NULL?");
	return -1;
}

int32_t EncryptorService::resetCodeType(void){
	if(mEncryptor){
		return mEncryptor->resetCodeType();
	}
	ALOGE("EncryptorService::resetCodeType,mEncryptor is NULL?");
	return -1;
}
/* native service impl end */

/* java service remote call impl begin */
const android::String16 IEncryptorJService::descriptor("com.sdtm.encryptor.IEncryptorJService");

const android::String16& IEncryptorJService::getInterfaceDescriptor() const{
	return IEncryptorJService::descriptor;
}

IEncryptorJService::IEncryptorJService(){ ALOGE("IEncryptorJService::IEncryptorJService()"); }
IEncryptorJService::~IEncryptorJService(){
	ALOGE("IEncryptorJService::~IEncryptorJService()");
	if(mEncryptor){
		ALOGE("EncryptorJService may be killed,disbale encryptor automatically");
		mEncryptor->disable();
	}
}

android::sp<IEncryptorJService> IEncryptorJService::asInterface(const android::sp<android::IBinder>& obj){
	android::sp<IEncryptorJService> intr;
	if(obj != NULL){
		intr = static_cast<IEncryptorJService*>(obj->queryLocalInterface(IEncryptorJService::descriptor).get());
		if(intr == NULL){
			intr = new BpEncryptorJService(obj);
		}
	}
	return intr;
}

void BpEncryptorJService::handleEncryptCode(String16 code){
	Parcel data,reply;
	data.writeInterfaceToken(IEncryptorJService::getInterfaceDescriptor());
	data.writeString16(code);
	remote()->transact(HANDLE_ENCRYPT_CODE,data,&reply,IBinder::FLAG_ONEWAY);
}

void BpEncryptorJService::notifyInitFinish(int32_t state){
	Parcel data,reply;
	data.writeInterfaceToken(IEncryptorJService::getInterfaceDescriptor());
	data.writeInt32(state);
	remote()->transact(NOTIFY_INIT_FINISH,data,&reply,IBinder::FLAG_ONEWAY);
}
/* java service remote call impl end */

/* Encryptor implementation begin */
Encryptor::Encryptor() : uart_fd(-1),scn_ctrl_fd(-1),enabled(0),encryptning(0),thread_running(0),is_resetting(0),handler(NULL){ ALOGE("Encryptor::Encryptor()"); }
Encryptor::~Encryptor(){
	ALOGE("Encryptor::~Encryptor()");
	close_uart();
	if(scn_ctrl_fd > 0){
		close(scn_ctrl_fd);
	}
}

Encryptor* Encryptor::getInstance(){
	if(NULL == __instance){
		__instance = new Encryptor();
	}
	return __instance;
}

void Encryptor::releaseInstance(){
	if(NULL != __instance){
		delete __instance;
		__instance = NULL;
	}
}

int Encryptor::enable(){
	if(enabled){
		ALOGE("Encryptor::enable,encryptor is already enabled");
		return 0;
	}
	if(is_resetting){
		cached_state = 1;
		ALOGE("Encryptor::enable,encryptor is being resetted,will enable encryptor later");
		return 0;
	}
	if(-1 == uart_switch_choose(1))
		ALOGE("can't config this switch\n");
	uart_fd = open_uart();
	ASSERT(uart_fd > 0);
	scn_ctrl_fd = open(ENCRYPTOR_CTRL_FILE,O_RDWR);
	ALOGE("Encryptor::enable,scn_ctrl_fd=%d",scn_ctrl_fd);
	ASSERT(scn_ctrl_fd > 0);
	thread_running = 1;
	pthread_t t;
	int ret = pthread_create(&t,NULL,__run_read_thread,this);
	ASSERT(!ret);
	enabled = 1;
	return 0;
}

void Encryptor::disable(){
	if(is_resetting){
		cached_state = 0;
		ALOGE("Encryptor::disable,encryptor is being resetted,will disable encryptor later");
		return;
	}
	close_uart();
	if(scn_ctrl_fd > 0){
		close(scn_ctrl_fd);
		scn_ctrl_fd = -1;
	}
    uart_switch_choose(0);
		
	thread_running = 0;
	pthread_cond_broadcast(&cond);
	enabled = 0;
}

void Encryptor::startEncrypt(){
	if(enabled && !is_resetting){
		pthread_mutex_lock(&mutex);
		if(!encryptning){
			encryptning = 1;
		}
		#ifdef CLOSE_UART_AFTER_ENCRYPT
		uart_fd = open_uart();
		ASSERT(uart_fd > 0);
		#endif //CLOSE_UART_AFTER_ENCRYPT
		pthread_mutex_unlock(&mutex);
		pthread_cond_broadcast(&cond);
		return;
	}
	ALOGE("Encryptor::startEncrypt():encryptor have not been enabled,or encryptor is being resetted");
}

void Encryptor::cancelEncrypt(){
	if(enabled && !is_resetting){
		pthread_mutex_lock(&mutex);
		if(encryptning){
			encryptning = 0;
		}
		pthread_mutex_unlock(&mutex);
		pthread_cond_broadcast(&cond);
		return;
	}
	ALOGE("Encryptor::cancelEncrypt():encryptor have not been enabled,or encryptor is being resetted");
}

int Encryptor::setCodeType(String16 typeName,int state){
	if(enabled){
		int index = get_type_index(String8(typeName).string());
		if(index < 0){
			ALOGE("Encryptor::setCodeType:unkonwn code type name:%s",(String8(typeName).string()));
			return -1;
		}
		return set_code_type(index,state);
	}
	ALOGE("Encryptor::setCodeType():encryptor have not been enabled");
	return -1;
}

void Encryptor::write_cmd(const char* cmd,unsigned int len){
	int value = 0;
	int timeout = 300;
	encryptor_controller(ENCRYPTOR_STATE_RECV_CMD_START);
	write(uart_fd,cmd,len);
	while(!value && timeout--){
		ioctl(uart_fd,TIOCSERGETLSR,&value);
	}
	encryptor_controller(ENCRYPTOR_STATE_RECV_CMD_END);
}

int Encryptor::set_code_type(unsigned int index,int state){
	char ret_buf[128] = { 0 };
	unsigned int tv_len = sizeof(struct timeval);
	const char* cmd = state ? types[index].enable_cmd : types[index].disable_cmd;

	#ifdef CLOSE_UART_AFTER_ENCRYPT
	uart_fd = open_uart();
	ASSERT(uart_fd > 0);
	#endif //CLOSE_UART_AFTER_ENCRYPT
	write_cmd(cmd,types[index].cmd_len);
	read(uart_fd,ret_buf,sizeof(ret_buf) / sizeof(ret_buf[0]));

	if(ret_buf[tv_len + 1] != WRITE_SUCCESS_FLAG){
		ALOGE("Encryptor::set_code_type() failed to set code type,index:%d state:%d,will retry.(%x)",index,state,ret_buf[tv_len + 1]);
		cmd = state ? types[index].enable_retry : types[index].disable_retry;
		write_cmd(cmd,types[index].cmd_len);
		read(uart_fd,ret_buf,sizeof(ret_buf) / sizeof(ret_buf[0]));
	}

	if(ret_buf[tv_len + 1] != WRITE_SUCCESS_FLAG){
		ALOGE("Encryptor::set_code_type() failed to retry.(%x)", ret_buf[tv_len + 1]);
		#ifdef CLOSE_UART_AFTER_ENCRYPT
		close_uart();
		#endif //CLOSE_UART_AFTER_ENCRYPT
		return -1;
	}

	if(state && types[index].length_cmd && (types[index].length_cmd_len > 0)){

		memset(ret_buf,0,sizeof(ret_buf) / sizeof(ret_buf[0]));

		write_cmd(types[index].length_cmd,types[index].length_cmd_len);
		read(uart_fd,ret_buf,sizeof(ret_buf) / sizeof(ret_buf[0]));

		if(ret_buf[tv_len + 1] != WRITE_SUCCESS_FLAG){
			ALOGE("Encryptor::set_code_type() failed to set code length,index:%d state:%d,will retry.",index,state);
			write_cmd(types[index].length_retry,types[index].length_cmd_len);
			read(uart_fd,ret_buf,sizeof(ret_buf) / sizeof(ret_buf[0]));
		}
		if(ret_buf[tv_len + 1] != WRITE_SUCCESS_FLAG){
			ALOGE("Encryptor::set_code_type() failed to retry code length setting");
			#ifdef CLOSE_UART_AFTER_ENCRYPT
			close_uart();
			#endif //CLOSE_UART_AFTER_ENCRYPT
			return -1;
		}
	}
	#ifdef CLOSE_UART_AFTER_ENCRYPT
	close_uart();
	#endif //CLOSE_UART_AFTER_ENCRYPT
	return 0;
}

int Encryptor::get_type_index(const char*name){
	for(unsigned int i = 0;i < sizeof(types) / sizeof(types[0]);i++){
		if(!strcmp(name,types[i].name)){
			return i;
		}
	}
	return -1;
}

int Encryptor::resetCodeType(){
	cached_state = enabled;
	if(!enabled){
		ALOGE("Encryptor::resetCodeType():encryptor have not been enabled,will enbale it augomatic for reset code type");
		enable();
	}
	is_resetting = 1;
	pthread_t t;
	int ret = pthread_create(&t,NULL,__run_reset_thread,this);
	ASSERT(!ret);
	return 0;
}

void Encryptor::do_reset(){
	int ret = 0;
	for(unsigned int i = 0;i < sizeof(default_code_type) / sizeof(default_code_type[0]);i++){
		if(set_code_type(i,default_code_type[i])){
			ALOGE("Encryptor::do_reset():failed to reset code type,failed at index:%d",i);
			err_log_to_file("Encryptor::do_reset():failed to reset code type,failed at index:%d",i);
			ret = -1;
		}
	}
	#ifdef CLOSE_UART_AFTER_ENCRYPT
	uart_fd = open_uart();
	ASSERT(uart_fd > 0);
	#endif //CLOSE_UART_AFTER_ENCRYPT
	tcflush(uart_fd,TCIOFLUSH);
	#ifdef CLOSE_UART_AFTER_ENCRYPT
	close_uart();
	#endif //CLOSE_UART_AFTER_ENCRYPT
	is_resetting = 0;
	if(!cached_state){
		ALOGE("Encryptor::do_reset():perform cached dsiable");
		disable();
	}
	if(handler != NULL){
		handler->notifyInitFinish(ret);
	}
}

void Encryptor::setHandler(android::sp<IEncryptorJService>& handler){
	this->handler = handler;
}

void Encryptor::byte2hexString(const  char* code,int len,char * hex){
	int i,tmp,j=0;
	char parse[] = "0123456789ABCDEF"; 
	//ALOGE("##@@len=%d,code=%s\n",len,code);
	for(i=0;i<len;i++){
		tmp = code[i]/16;
		hex[j++] = parse[tmp];
		tmp = code[i]%16;
		hex[j++] = parse[tmp];
	}
	hex[j] = 0;
	//ALOGE("##@@len=%d,code=%s\n",len,code);
}
void Encryptor::read_uart(){
	ssize_t size,size1;
	char buf[MAX_CODE_LEN + sizeof(struct timeval)] = { 0 };
	char buf1[MAX_CODE_LEN + sizeof(struct timeval)] = { 0 };
	char hexCode[2*(MAX_CODE_LEN + sizeof(struct timeval))] = { 0 };
	int old_state = encryptning;
	long long start_time = 0;
	int readNum;
	char code[MAX_CODE_LEN + sizeof(struct timeval)] = { 0 };
	int i,k,j=0;
	while(1){
		//ALOGE("##@@swhile(1)\n");
		pthread_mutex_lock(&mutex);
		//ALOGE("##@@encryptning=%d\n",encryptning);
		if(encryptning){
			//close_uart();
			//ALOGE("##@@old_state=%d\n",old_state);
			if(old_state != encryptning){
				encryptor_controller(ENCRYPTOR_STATE_ENCRYPTNING);
				start_time = current_time_millis();
			}
			
			memset(buf,0,sizeof(buf)/sizeof(buf[0]));
			size = read(uart_fd,buf,sizeof(buf)/sizeof(buf[0]));
			ALOGE("size = %d, buf:%s\n", size, buf);

			if(size>0){
				for(i=0;i<2;i++){//Continuous read 2 times to get a string completely
					usleep(30000);
					memset(buf1,0,sizeof(buf1)/sizeof(buf1[0]));
					size1 = read(uart_fd,buf1,sizeof(buf1)/sizeof(buf[0]));
					ALOGE("i = %d,size1 = %d, buf1:%s\n", i,size1, buf1);
					memcpy(buf+size,buf1,size1);
					size = size+size1;
				}
			}

			if (size > 0) {
				ALOGE("size = %d, buf:%s\n", size, buf);
				memset(hexCode,0,sizeof(hexCode)/sizeof(hexCode[0]));

#ifdef REMOVE_TIME_STAMP
				//for(i = 0;i < size+2;i++)
					//ALOGE("buf[%d] = %d(%c)\n", i, buf[i],buf[i]);
				i = 0;
				j = 0;
				while(i<size){
					k = i/57;
					if(i > 57*k+7){
						code[j++] = buf[i];
						//ALOGE("code[%d] = %d(%c)\n", j-1, code[j-1],code[j-1]);
					}
					i++;
				}
				code[j] = 0;
				ALOGE("strlen(code)=%d,code = %s\n",strlen(code), code);
				byte2hexString(code,strlen(code),hexCode);
#else
				byte2hexString(buf,strlen(buf),hexCode);
#endif //REMOVE_TIME_STAMP
				ALOGE("handle hexCode:%s\n",hexCode);
				if(handler != NULL){
						handler->handleEncryptCode(String16(hexCode));
				}

				encryptning = 0;
				encryptor_controller(ENCRYPTOR_STATE_DISABLE);
			}
			if(size > (ssize_t)sizeof(struct timeval) + 1){
				//char* code = buf + sizeof(struct timeval);
				//memcpy(code+size-sizeof(struct timeval),buf1 + sizeof(struct timeval),size1-sizeof(struct timeval));
				/*struct timeval encryptned_time;
				memcpy(&encryptned_time,buf,sizeof(encryptned_time));
				do{
					if(timeval_to_ll(&encryptned_time) < start_time){
						ALOGE("discard overdue code:%s",code);
						break;
					}
					ALOGE("sizeof(struct timeval) = %d , size = %d, origin encryptned code:%s\n", sizeof(struct timeval), size, code);

					//ALOGE("sizeof(char) = %d,sizeof(code)=%d\n",  sizeof(char),sizeof(code[i]));
					//write_encryptCode_to_file(code,strlen(code));
					for(i = 0;i < size;i++){
						if((code[i] == '\r') || (code[i] == '\n')){
							code[i] = '\0';
							ALOGE("code[%d] = r or t , size = %d\n",i,size);
							break;
						}
					}
					
					if(handler != NULL){
						ALOGE("handler strlen(code)   = %d , code:%s\n", strlen(code), code);
						byte2hexString(code,strlen(code),hexCode);
						ALOGE("handle hexCode:%s\n",hexCode);			
						handler->handleEncryptCode(String16(hexCode));
					}

					encryptning = 0;
					encryptor_controller(ENCRYPTOR_STATE_DISABLE);
				}while(0);*/
			
				//memset(buf,0,sizeof(buf)/sizeof(buf[0]));

			}else if(start_time && ((current_time_millis() - start_time) > ENCRYPT_TIME_OUT)){
				//cancelled by timeout 
				encryptning = 0;
				start_time = 0;
				encryptor_controller(ENCRYPTOR_STATE_DISABLE);
				ALOGE("encryptn time out");
			}
		}else{
			//cancelled by key release 
			if(old_state != encryptning){
				start_time = 0;
				encryptor_controller(ENCRYPTOR_STATE_DISABLE);
			}
		}
		old_state = encryptning;
		if(!encryptning){
			#ifdef CLOSE_UART_AFTER_ENCRYPT
			close_uart();
			#endif //CLOSE_UART_AFTER_ENCRYPT
			pthread_cond_wait(&cond,&mutex);
		}
		pthread_mutex_unlock(&mutex);
		if(!thread_running){
			ALOGE("read_uart exits");
			return;
		}
	}
}

void Encryptor::encryptor_controller(int state){
	char value[2] = { 0 };
	int ret;
	snprintf(value,2,"%d",state);
	ret = write(scn_ctrl_fd,value,2);
	ASSERT(ret > 0);
}

void Encryptor::close_uart(){
	if (uart_fd > 0) {
		close(uart_fd);
		uart_fd = -1;
	}
}

void* Encryptor::__run_read_thread(void* param){
	((Encryptor*)param)->read_uart();
	return NULL;
}

void* Encryptor::__run_reset_thread(void* param){
	((Encryptor*)param)->do_reset();
	return NULL;
}
/* Encryptor implementation end */

int main(int argc,char** argv)
{
	if(already_running()){
		ALOGE("this is a single instance process,it's already running,dont'run again");
		printf("this is a single instance process,it's already running,dont'run again\n");
		return 0;
	}
	while(!is_system_ready()){
		usleep(300);
	}
	mEncryptor = Encryptor::getInstance();
	defaultServiceManager()->addService(String16("EncryptorService"), new EncryptorService());
	android::ProcessState::self()->startThreadPool();
	ALOGI("Encryptor native service is now ready");

	IPCThreadState::self()->joinThreadPool();
	Encryptor::releaseInstance();
	return 0;
}

int is_system_ready(void)
{
	char value[PROPERTY_VALUE_MAX] = { 0 };
	property_get("sys.boot_completed",value,"");
	return !strcmp("1",value);
}

long long timeval_to_ll(struct timeval* tv)
{
	if(!tv){
		return 0;
	}
	return tv->tv_sec * 1000LL + tv->tv_usec / 1000;
}

long long current_time_millis(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return(fcntl(fd,F_SETLK,&fl));
}

int already_running(void)
{
	int fd;
	char buf[16];
	fd = open(LOCKFILE,O_RDWR|O_CREAT,LOCKMODE);
	if(fd < 0){
		ALOGE("can't open %s: %s",LOCKFILE,strerror(errno));
		exit(1);
	}
	
	if(lockfile(fd) < 0){
		if(errno == EACCES || errno == EAGAIN){
			close(fd);
			return 1;
		}
		ALOGE("can't lock %s: %s",LOCKFILE,strerror(errno));
		exit(1);
	}
	ftruncate(fd,0);
	sprintf(buf,"%ld",(long)getpid());
	write(fd,buf,strlen(buf) + 1);
	return 0;
}

#ifdef 0//CLOSE_UART_AFTER_ENCRYPT
#define LENGTH 24
#define UART_PATH_FILE1 "/sys/devices/platform/msm_serial_hsl.0/uart_switch"
#define string1 "FREE_OFF" /* can use the uart*/
#define string2 "POS_ON"
#define string3 "CLOSE"
#define string4 "OTHER_ON"

void wait_if_uart_busy()
{
	char buf[LENGTH] = {0};
	int fd1;
	int ret;
	int byte;
	while(1) {
		//sleep(1);
		memset(buf,0,LENGTH);
		fd1 = open(UART_PATH_FILE1, O_RDWR);
		ret = read(fd1, buf, LENGTH);
		ALOGE("ENCRYPTOR read ret = %d, buf = %s\n", ret, buf);
		if ((ret > 0) && ((strncmp(string1, buf, (ret-1)) == 0))) {
			memset(buf, 0, LENGTH);
			byte = sprintf(buf, "FREE0\n");
			ALOGE("ENCRYPTOR write byte =  %d, buf = %s\n",byte,buf);
			ret = write(fd1, buf, byte);
			close(fd1);
			ALOGE("ENCRYPTOR write %s, ret = %d\n", UART_PATH_FILE1, ret);
			if(ret < 0)
				continue;
			else
				break;
		} else {
			memset(buf, 0, LENGTH);
			byte = sprintf(buf, "ENCRYPTOR\n");
			ALOGE("ENCRYPTOR write byte =  %d, buf = %s\n", byte, buf);
			ret = write(fd1, buf, byte);
			//close(fd1);
			ALOGE("ENCRYPTOR write %s, ret = %d\n", UART_PATH_FILE1, ret);
		}
		close(fd1);
		usleep(500);
	}
}
#endif //CLOSE_UART_AFTER_ENCRYPT

int open_uart(void)
{
	#ifdef 0//CLOSE_UART_AFTER_ENCRYPT
	wait_if_uart_busy();
	#endif //CLOSE_UART_AFTER_ENCRYPT
	int fd = -1;
	struct termios options;

	fd = open(UART_DEV_PATH,O_RDWR | O_NOCTTY | O_NDELAY);
	if(-1 == fd){
		ALOGE("Could not open uart port \n");
		return(-1);
	}
	if(tcflush(fd, TCIOFLUSH) < 0){
		ALOGE("Could not flush uart port");
		close(fd);
		return(-1);
	}

	if(tcgetattr(fd, &options) < 0){
		ALOGE("Can't get port settings \n");
		close(fd);
		return(-1);
	}

	options.c_cc[VTIME]    = 10;   /* inter-character timer unused */
	options.c_cc[VMIN]     = 512;   /* blocking read until 5 chars received */

	options.c_cflag &= ~(CSIZE | CRTSCTS);
	options.c_cflag |= (CS8 | CLOCAL | CREAD);
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfsetospeed(&options, B115200);
	cfsetispeed(&options, B115200);

	if(tcsetattr(fd, TCSANOW, &options) < 0){
		ALOGE("Can't set port setting \n");
		close(fd);
		return(-1);
	}
	return fd;
}
/*
int uart_switch_choose(int control)
{
	int fd = -1;
	int ret = 0;
	fd = open(UART_SWITCH_CONTROL_PATH, O_RDWR|O_TRUNC); 
	if(-1 == fd)
		ALOGE("can't open uart switch path\n");
	switch(control){
	case 1:
		ret = write(fd, uart_switch_control[0], strlen(uart_switch_control[0])); //enable se955
		if(-1 == ret){
			ALOGE("can't write uart switch path\n");
			close(fd);
			}
		break;
	case 0:
		ret = write(fd, uart_switch_control[3], strlen(uart_switch_control[3]));// close uart
		if(-1 == ret){
			ALOGE("can't write uart switch path\n");
			close(fd);
			}
		break;
	default:
		ALOGE("No operat on se955 uart\n");
	}
	return ret;
}*/

void err_log_to_file(const char *fmt, ...)
{
	FILE * log_file = fopen(ERR_LOG_PATH,"a");
	if(!log_file){
		return;
	}
	va_list ap;
	va_start(ap,fmt);
	vfprintf(log_file,fmt,ap);
	va_end(ap);
	fclose(log_file);
}

