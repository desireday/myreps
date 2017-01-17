#ifndef __ENCRYPTOR_SDTM_H__
#define __ENCRYPTOR_SDTM_H__

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "encryptor-native-service"

#include <utils/RefBase.h>

#include <utils/Log.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include <pthread.h>

using namespace android;

int is_system_ready(void);
long long current_time_millis(void);
long long timeval_to_ll(struct timeval* tv);
/*#ifdef CLOSE_UART_AFTER_ENCRYPT
void wait_if_uart_busy();
#endif //CLOSE_UART_AFTER_ENCRYPT*/
int open_uart(void);
int already_running(void);
int lockfile(int fd);
void assert_fail(const char *file, int line, const char *func, const char *expr);
void err_log_to_file(const char *fmt, ...);

//const char* uart_switch_control[4] = {"se955", "uart1", "uart2", "disable"};
#define ASSERT(e) \
	do { \
		if(!(e)) \
			assert_fail(__FILE__,__LINE__,__func__,#e); \
	} while(0)

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define LOCKFILE "/data/local/tmp/encryptor_pid"

#define ERR_LOG_PATH "/data/tmp"

/* native service aidl begain */
class IEncryptorService : public IInterface{
	public:
		enum{
			SET_HANDLE = IBinder::FIRST_CALL_TRANSACTION,
			ENABLE,
			DISABLE,
			START,
			CANCEL,
			SET_TYPE,
			RESET_TYPE
		};
		DECLARE_META_INTERFACE(EncryptorService);
		virtual int32_t setHandleService(String16) = 0;
		virtual int32_t enable(void) = 0;
		virtual void disable(void) = 0;
		virtual void startEncrypt(void) = 0;
		virtual void cancelEncrypt(void) = 0;
		virtual int32_t setCodeType(String16,int32_t) = 0;
		virtual int32_t resetCodeType(void) = 0;
};

class EncryptorService : public BnInterface<IEncryptorService>{
	virtual status_t onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags = 0);
	virtual int32_t setHandleService(String16);
	virtual int32_t enable(void);
	virtual void disable(void);
	virtual void startEncrypt(void);
	virtual void cancelEncrypt(void);
	virtual int32_t setCodeType(String16,int32_t);
	virtual int32_t resetCodeType(void);
};
/* native service aidl begain */

/* java service aidl begain */
class IEncryptorJService : public IInterface{
	public:
		DECLARE_META_INTERFACE(EncryptorJService);
		virtual void handleEncryptCode(String16) = 0;
		virtual void notifyInitFinish(int32_t) = 0;
};
class BpEncryptorJService : public BpInterface<IEncryptorJService>{
	public:
		enum{
			HANDLE_ENCRYPT_CODE = IBinder::FIRST_CALL_TRANSACTION,
			NOTIFY_INIT_FINISH
		};
		BpEncryptorJService(const sp<IBinder>& impl) : BpInterface<IEncryptorJService>(impl) { ALOGI("BpEncryptorJService::BpEncryptorJService()"); }
		virtual void handleEncryptCode(String16);
		virtual void notifyInitFinish(int32_t);
};
/* java service aidl end */
#ifdef 0
struct code_type{
	const unsigned int cmd_len;
	const char* name;
	const char* enable_cmd;
	const char* enable_retry;
	const char* disable_cmd;
	const char* disable_retry;
	const unsigned int length_cmd_len;
	const char* length_cmd;
	const char* length_retry;
};

static struct code_type types[] = {
	{
		9, /* cmd_len */
		"UPC-A", /* name */
		"\x07\xC6\x04\x08\xFF\x01\x01\xFE\x26\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x01\x01\xFE\x25\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x01\x00\xFE\x27\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x01\x00\xFE\x26\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"UPC-E", /* name */
		"\x07\xC6\x04\x08\xFF\x02\x01\xFE\x25\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x02\x01\xFE\x24\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x02\x00\xFE\x26\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x02\x00\xFE\x25\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"UPC-E1", /* name */
		"\x07\xC6\x04\x08\xFF\x0C\x01\xFE\x1B\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x0C\x01\xFE\x1A\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x0C\x00\xFE\x1C\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x0C\x00\xFE\x1B\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"EAN-8", /* name */
		"\x07\xC6\x04\x08\xFF\x04\x01\xFE\x23\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x04\x01\xFE\x22\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x04\x00\xFE\x24\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x04\x00\xFE\x23\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"EAN-13", /* name */
		"\x07\xC6\x04\x08\xFF\x03\x01\xFE\x24\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x03\x01\xFE\x23\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x03\x00\xFE\x25\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x03\x00\xFE\x24\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"Code-128", /* name */
		"\x07\xC6\x04\x08\xFF\x08\x01\xFE\x1F\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x08\x01\xFE\x1E\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x08\x00\xFE\x20\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x08\x00\xFE\x1F\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"Code-39", /* name */
		"\x07\xC6\x04\x08\xFF\x00\x01\xFE\x27\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x00\x01\xFE\x26\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x00\x00\xFE\x28\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x00\x00\xFE\x27\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"Code-93", /* name */
		"\x07\xC6\x04\x08\xFF\x09\x01\xFE\x1E\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x09\x01\xFE\x1D\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x09\x00\xFE\x1F\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x09\x00\xFE\x1E\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /*length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"Interleaved-2-of-5", /* name */
		"\x07\xC6\x04\x08\xFF\x06\x01\xFE\x21\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x06\x01\xFE\x20\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x06\x00\xFE\x22\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x06\x00\xFE\x21\x0", /* disable_retry */
		11, /* length_cmd_len */
		"\x09\xC6\x04\x08\xFF\x16\x06\x17\x0E\xFD\xE5\x0", /* length_cmd */
		"\x09\xC6\x04\x09\xFF\x16\x06\x17\x0E\xFD\xE4\x0" /* length_retry */
	},
	{
		9, /* cmd_len */
		"Discrete-2-of-5", /* name */
		"\x07\xC6\x04\x08\xFF\x05\x01\xFE\x22\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x05\x01\xFE\x21\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x05\x00\xFE\x23\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x05\x00\xFE\x22\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		10, /* cmd_len */
		"Chinese-2-of-5", /* name */
		"\x08\xC6\x04\x08\xFF\xF0\x98\x01\xFC\x9E\x0", /* enable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x98\x01\xFC\x9D\x0", /* enable_retry */
		"\x08\xC6\x04\x08\xFF\xF0\x98\x00\xFC\x9F\x0", /* disable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x98\x00\xFC\x9E\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"Codabar", /* name */
		"\x07\xC6\x04\x08\xFF\x07\x01\xFE\x20\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x07\x01\xFE\x1F\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x07\x00\xFE\x21\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x07\x00\xFE\x20\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */

	},
	{
		9, /* cmd_len */
		"MSI", /* name */
		"\x07\xC6\x04\x08\xFF\x0B\x01\xFE\x1C\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x0B\x01\xFE\x1B\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x0B\x00\xFE\x1D\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x0B\x00\xFE\x1C\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		9, /* cmd_len */
		"Code-11", /* name */
		"\x07\xC6\x04\x08\xFF\x0A\x01\xFE\x1D\x0", /* enable_cmd */
		"\x07\xC6\x04\x09\xFF\x0A\x01\xFE\x1C\x0", /* enable_retry */
		"\x07\xC6\x04\x08\xFF\x0A\x00\xFE\x1E\x0", /* disable_cmd */
		"\x07\xC6\x04\x09\xFF\x0A\x00\xFE\x1D\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */

	},
	{
		10, /* cmd_len */
		"GSI-DataBar-14", /* name */
		"\x08\xC6\x04\x08\xFF\xF0\x52\x01\xFC\xE4\x0", /* enable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x52\x01\xFC\xE3\x0", /* enable_retry */
		"\x08\xC6\x04\x08\xFF\xF0\x52\x00\xFC\xE5\x0", /* disable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x52\x00\xFC\xE4\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		10, /* cmd_len */
		"GSI-DataBar-Limited", /* name */
		"\x08\xC6\x04\x08\xFF\xF0\x53\x01\xFC\xE3\x0", /* enable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x53\x01\xFC\xE2\x0", /* enable_retry */
		"\x08\xC6\x04\x08\xFF\xF0\x53\x00\xFC\xE4\x0", /* disable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x53\x00\xFC\xE3\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	},
	{
		10, /* cmd_len */
		"GSI-DataBar-Expanded", /* name */
		"\x08\xC6\x04\x08\xFF\xF0\x54\x01\xFC\xE2\x0", /* enable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x54\x01\xFC\xE1\x0", /* enable_retry */
		"\x08\xC6\x04\x08\xFF\xF0\x54\x00\xFC\xE3\x0", /* disable_cmd */
		"\x08\xC6\x04\x09\xFF\xF0\x54\x00\xFC\xE2\x0", /* disable_retry */
		0, /* length_cmd_len */
		NULL, /* length_cmd */
		NULL /* length_retry */
	}
};

static int default_code_type[] = {
	1,/* UPC-A */
	1,/* UPC-E */
	0,/* UPC-E1 */
	1,/* EAN-8 */
	1,/* EAN-13 */
	1,/* Code-128 */
	1,/* Code-39 */
	0,/* Code-93 */
	0,/* Interleaved-2-of-5 */
	0,/* Discrete-2-of-5 */
	0,/* Chinese-2-of-5 */
	0,/* Codabar */
	0,/* MSI */
	0,/* Code-11 */
	0,/* GSI-DataBar-14 */
	0,/* GSI-DataBar-Limited */
	0 /* GSI-DataBar-Expanded */
};
#endif
//#define ENCRYPTOR_CTRL_FILE "/sys/devices/platform/scan_se955/se955_state"
//#define ENCRYPTOR_CTRL_FILE  "/sys/class/se955_scan/device/control"
#define UART_DEV_PATH "/dev/ttyHSL1"
#define MAX_CODE_LEN 580
#define ENCRYPT_TIME_OUT 4000
#define WRITE_SUCCESS_FLAG 0xD0

//this node is just for RS232-UART-SWITCH--ERJI
//#define UART_SWITCH_CONTROL_PATH  "/sys/class/tty/ttyHSL1/device/uart_switch"

class Encryptor{
	public:
		static Encryptor* getInstance();
		static void releaseInstance();
		int enable(void);
		void disable(void);
		void startEncrypt(void);
		void cancelEncrypt(void);
		int setCodeType(String16,int);
		int resetCodeType();
		void setHandler(android::sp<IEncryptorJService>&);
	private:
		int uart_fd;
		int scn_ctrl_fd;
		int enabled;
		int scanning;
		int thread_running;
		int is_resetting;
		int cached_state;
		sp<IEncryptorJService> handler;
		Encryptor();
		~Encryptor();
		void read_uart(void);
		void byte2hexString(const  char* code,int len,char * hex);
		void close_uart();
		void do_reset(void);
		void encryptor_controller(int state);
		void write_cmd(const char*cmd,unsigned int len);
		int set_code_type(unsigned int index,int state);
		int get_type_index(const char* name);
		static Encryptor* __instance;
		static pthread_mutex_t mutex;
		static pthread_cond_t cond;
		static void* __run_read_thread(void* param);
		static void* __run_reset_thread(void* param);
		enum{
			ENCRYPTOR_STATE_DISABLE = 0,
			ENCRYPTOR_STATE_ENCRYPTNING,
			ENCRYPTOR_STATE_RECV_CMD_START,
			ENCRYPTOR_STATE_RECV_CMD_END
		};
};
Encryptor* Encryptor::__instance = NULL;
pthread_mutex_t Encryptor::mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Encryptor::cond = PTHREAD_COND_INITIALIZER;

#endif
