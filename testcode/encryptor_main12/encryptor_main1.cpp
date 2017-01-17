#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cutils/log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include "EncryptorRegistrar.h"
#include <encryptorservice/IEncryptorRegistrar.h>
#include <encryptorservice/IEncryptorHandler.h>
#include "encryptor_main1.h"

#define UART_DEV_PATH "/dev/ttyHSL1"

#include <pthread.h>

using namespace android;


static int uart_fd;
static pthread_t encryption_thread;
static pthread_mutex_t encryption_mutex;
static pthread_cond_t encryption_cond;
static const char* version_cmd = "\xAA\xBB\x32\x00\x04\x01\x01\xCC\xDD\x27\x0";
static unsigned int version_len = 10;

static sp<IEncryptorHandler> sdtm_ghandler;

void SetHandler(sp<IEncryptorHandler>& sdtm_handler){
	sdtm_ghandler = sdtm_handler;
}

void byte2hexString(const  char* code,int len,char * hex){
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

void *read_data(void *arg __attribute__((unused))){
	unsigned int  size = 0;
	//unsigned int  size1 = 0;
	char buf[512] = { 0 };
	char covert_buf[512] = { 0 };
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
		if(buf[1] == 0xCC)
			ALOGE("ack received\n");//ack_resp todo
		else if(buf[1] == 0xBB) {
			byte2hexString(buf,size,covert_buf);
			ALOGE("get_version covert_buf->(%s)\n", covert_buf);
			sdtm_ghandler->printVersion(String16(covert_buf));
		}
		pthread_mutex_unlock(&encryption_mutex);
	}
	ALOGE("thread quit");
	return NULL;
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

void SendCmdToSecureModule(){
		write_cmd(version_cmd,version_len);
}

int main(int argc,char *argv[]) {
	
		EncryptorRegistrar::instantiate();

	uart_fd = open_uart();
	ALOGE("Encryption uart ready");

	pthread_create(&encryption_thread, NULL, read_data, NULL);
			
		ProcessState::self()->startThreadPool();
		IPCThreadState::self()->joinThreadPool();
		return 0;
}
