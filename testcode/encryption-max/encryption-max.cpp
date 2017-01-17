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

#include "encryption-max.h"

static int uart_fd;
static pthread_t encryption_thread;
static pthread_mutex_t encryption_mutex;
static pthread_cond_t encryption_cond;
static const char* version_cmd = "\xAA\xBB\x32\x00\x04\x01\x01\xCC\xDD\x27\x0";
static unsigned int version_len = 10;


void assert_fail(const char *file,int line,const char *func,const char *expr){
	ALOGE("assertion failed at file %s,line %d,function %s:",file,line,func);
	ALOGE("%s",expr);
	abort();
}

void write_cmd(const char* cmd,unsigned int len){
	int value = 0;
	int timeout = 300;
	int i;
	
	for(i=0;i<len;i++)
		ALOGE("write_cmd cmd->(%d).(%x)", i,cmd[i]);
	write(uart_fd,cmd,len);
	while(!value && timeout--){
		ioctl(uart_fd,TIOCSERGETLSR,&value);
	}
}


void *read_data(void *arg __attribute__((unused))){
	unsigned int  size = 0;
	//unsigned int  size1 = 0;
	char buf[512] = { 0 };
	//char buf1[512] = { 0 };
	int i;
	int cycle = 0;
	
	while(1){
		ALOGE("read uart data thread enter while...(%d)",cycle++);
		pthread_mutex_lock(&encryption_mutex);
		//usleep(30000);
		
		memset(buf,0,sizeof(buf)/sizeof(buf[0]));
		size = read(uart_fd,buf,sizeof(buf)/sizeof(buf[0]));
		ALOGE("get_version ack->size(%d).buf->(%s)\n", size, buf);

		if(size>0)
			for(i=0;i<size;i++)
				ALOGE("get_version onebyone->num_buf[%d]->(%x)\n", i, buf[i]);
		//ALOGE("get_version ack_total->size_total(%d).buf_total->(%s)\n", size, buf);
		pthread_mutex_unlock(&encryption_mutex);
	}
	ALOGE("thread quit");
	return NULL;
}


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
	
	uart_fd = open_uart();
	ASSERT(uart_fd > 0);
	ALOGE("Encryption uart ready");

	pthread_create(&encryption_thread, NULL, read_data, NULL);

	write_cmd(version_cmd,version_len);

	ALOGE("encryption_thread_func created");

	android::ProcessState::self()->startThreadPool();

	IPCThreadState::self()->joinThreadPool();

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


int open_uart(void)
{
	int fd = -1;
	struct termios options;

	fd = open(UART_DEV_PATH,O_RDWR | O_NOCTTY);
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

