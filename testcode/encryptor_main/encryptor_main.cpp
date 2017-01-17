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
#include <pthread.h>
#include "encryptor_main.h"


//#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

static int uart_fd;
static pthread_t encryption_thread;
static pthread_mutex_t encryption_mutex;
static pthread_cond_t encryption_cond;
static const char* version_cmd = "\xAA\xBB\x32\x00\x04\x01\x01\xCC\xDD\x27\x0";
static unsigned int version_len = 10;
static const char* start_cmd = "\xAA\xBB\x33\x00\x04\x01\x02\xCC\xDD\x25\x0";
static unsigned int start_len = 10;
static const char* time_cmd = "\xAA\xBB\x34\x00\x04\x01\x08\xCC\xDD\x28\x0";
static unsigned int time_len = 10;
static const char* set_time_cmd = "\xAA\xBB\x36\x00\x08\x01\x09\x45\x52\x52\x02\xCC\xDD\x60\x0";
static unsigned int set_time_len = 14;
static const char* vol_cmd = "\xAA\xBB\x35\x00\x04\x01\x0B\xCC\xDD\x2A\x0";
static unsigned int vol_len = 10;
static const char* ic_exist_cmd = "\xAA\xBB\x37\x00\x04\x03\x28\xCC\xDD\x09\x0";
static unsigned int ic_exist_len = 10;
static const char* fm_down_cmd = "\xAA\xBB\xC9\x00\x0C\x01\x0C\x00\x00\x00\x02\x37\x58\x12\x3A\xCC\xDD\x9C\x0";
static unsigned int fm_down_len = 18;


static char ack_resp[6] = { 0 };
static unsigned int ack_resp_len = 6;

enum cmd_type_t {
	VER_GET = 0,
	START_PRO,
	TIME_GET,
	VOL_GET,
	TIME_SET,
	IC_EXIST,
	FM_DOWN,
};


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



int read_data(char* read_buf, int read_len){
	unsigned int  size = 0;
	//unsigned int  size1 = 0;
	char buf[512] = { 0 };
	char covert_buf[512] = { 0 };
	//char buf1[512] = { 0 };
	int i;
	//int cycle = 0;
		pthread_mutex_lock(&encryption_mutex);
		//usleep(30000);
		
		memset(buf,0,sizeof(buf)/sizeof(buf[0]));
		size = read(uart_fd,buf,read_len);
		for(i=0;i<size;i++)
			ALOGE("read_data size(%d)->(%d).buf->(%x)\n", size, i, buf[i]);
		ALOGE("read_data check if ack exist _buf[1]->(0x%x) == 0xcc\n", buf[1]);

		if (buf[1] == 0xcc){
			memcpy((void*)ack_resp,(void*)buf,ack_resp_len);
			ALOGE("read_data print  ack_resp == %d\n", size);

		}

		/*if(size>0)
			for(i=0;i<size;i++)
				ALOGE("get_version onebyone->num_buf[%d]->(%x)\n", i, buf[i]);*/

			byte2hexString(buf,size,covert_buf);
		
		ALOGE("read_data covert->size(%d).covert_buf->(%s)\n", size*2, covert_buf);




		
		memcpy((void*)read_buf,(void*)covert_buf,size*2);

		
		pthread_mutex_unlock(&encryption_mutex);
	return size*2;
}

void Check_AckRespFromSecureModule(){

	char ack_buf[512] = { 0 };
	int ack_size = 0;
	int ack_len = ack_resp_len;

	ack_size = read_data(ack_buf, ack_len);


	ALOGE("Check_AckRespFromSecureModule ->size(%d).covert_buf->(%s)\n", ack_size, ack_buf);
		//byte2hexString
}

void getver_info(){
	char ver_buf[512] = { 0 };
	int ver_size = 0;
	int ver_len = 512;
		
	ver_size = read_data(ver_buf, ver_len);

	ALOGE("getver_info ->ver_size(%d).ver_buf->(%s)\n", ver_size, ver_buf);

}

void getbatteryvol_info(){
	char batteryvol_buf[512] = { 0 };
	int batteryvol_size = 0;
	int batteryvol_len = 512;
		
	batteryvol_size = read_data(batteryvol_buf, batteryvol_len);

	ALOGE("getbatteryvol_info ->batteryvol_size(%d).batteryvol_buf->(%s)\n", batteryvol_size, batteryvol_buf);

}

void check_ic_exist_info(){
	char ic_exist_buf[512] = { 0 };
	int ic_exist_size = 0;
	int ic_exist_len = 512;
		
	ic_exist_size = read_data(ic_exist_buf, ic_exist_len);

	ALOGE("gettime_info ->ic_exist_size(%d).ic_exist_buf->(%s)\n", ic_exist_size, ic_exist_buf);

}

void gettime_info(){
	char time_buf[512] = { 0 };
	int time_size = 0;
	int time_len = 512;
		
	time_size = read_data(time_buf, time_len);

	ALOGE("gettime_info ->time_size(%d).time_buf->(%s)\n", time_size, time_buf);

}

void get_fm_info(){
	char fm_buf[512] = { 0 };
	int fm_size = 0;
	int fm_len = 512;
		
	fm_size = read_data(fm_buf, fm_len);

	ALOGE("get_fm_info ->fm_size(%d).fm_buf->(%s)\n", fm_size, fm_buf);

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

void write_uart(const char* cmd,unsigned int len){
	int value = 0;
	int timeout = 300;
	int i;
	
	for(i=0;i<len;i++)
		ALOGE("write_uart cmd(%d)->(%d).(%x)", len,i,cmd[i]);
	write(uart_fd,cmd,len);
	while(!value && timeout--){
		ioctl(uart_fd,TIOCSERGETLSR,&value);
	}
}

void SendAckReceiveToSecureModule(){
	write_uart(ack_resp,ack_resp_len);
}

void SendCmdToSecureModule(cmd_type_t cmd){

	ALOGE("SendCmdToSecureModule enter");
	switch(cmd) {
		case 0:
			write_uart(version_cmd,version_len);
		break;
		case 1:
			write_uart(start_cmd,start_len);
		break;
		case 2:
			write_uart(time_cmd,time_len);
		break;
		case 3:
			write_uart(vol_cmd,vol_len);
		break;
		case 4:
			write_uart(set_time_cmd,set_time_len);
		break;
		case 5:
			write_uart(ic_exist_cmd,ic_exist_len);
		break;
		case 6:
			write_uart(fm_down_cmd,fm_down_len);
		break;
	}
	ALOGE("SendCmdToSecureModule quit");
}
/*
func1_type(){
	SendCmdToSecureModule(cm1,cm2,...)
	Check_AckRespFromSecureModule()
	read_data()
		SendAckReceiveToSecureModule()
		//check data_no	-> error_no & show_res

}
*/


void getversion(){

	ALOGE("getversion enter");

	SendCmdToSecureModule(VER_GET);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	getver_info();
	SendAckReceiveToSecureModule();

}

void start_process(){

	ALOGE("start_process enter");

	SendCmdToSecureModule(START_PRO);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	//getver_info();
	SendAckReceiveToSecureModule();

}

void settime(){

	ALOGE("settime enter");

	SendCmdToSecureModule(TIME_SET);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	//getver_info();
	SendAckReceiveToSecureModule();

}


void gettime(){

	ALOGE("gettime enter");

	SendCmdToSecureModule(TIME_GET);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	gettime_info();
	SendAckReceiveToSecureModule();

}

void getbatteryvol(){

	ALOGE("getbatteryvol enter");

	SendCmdToSecureModule(VOL_GET);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	getbatteryvol_info();
	SendAckReceiveToSecureModule();

}

void check_ic_exist(){

	ALOGE("check_ic_exist enter");

	SendCmdToSecureModule(IC_EXIST);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	check_ic_exist_info();
	SendAckReceiveToSecureModule();

}

void firmware_download(){

	ALOGE("firmware_download enter");

	SendCmdToSecureModule(FM_DOWN);
	//write_uart(version_cmd,version_len);
	//usleep(100000);
	Check_AckRespFromSecureModule();

	get_fm_info();
	SendAckReceiveToSecureModule();

}


int main(int argc,char *argv[]) {

	//char ver_buf[512];
	int res = 0;
	
	uart_fd = open_uart();
	ALOGE("Encryption uart ready");

	//getversion();

	//start_process();

	//settime();

	//gettime();
	//check_ic_exist();
	firmware_download();
	//getbatteryvol();
	
	//bin_download();
	//func1_type

		return res;
}
