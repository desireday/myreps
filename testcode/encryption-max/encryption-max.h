#ifndef __ENCRYPTION_MAX_H__
#define __ENCRYPTION_MAX_H__

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "encryption-native-service"

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
int open_uart(void);
int already_running(void);
int lockfile(int fd);
void assert_fail(const char *file, int line, const char *func, const char *expr);
void err_log_to_file(const char *fmt, ...);

#define ASSERT(e) \
	do { \
		if(!(e)) \
			assert_fail(__FILE__,__LINE__,__func__,#e); \
	} while(0)

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define LOCKFILE "/data/local/tmp/encryption_pid"

#define ERR_LOG_PATH "/data/tmp"


#define UART_DEV_PATH "/dev/ttyHSL1"




#endif
