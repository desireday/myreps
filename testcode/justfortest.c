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
#include "justfortest.h"


#define TEST_BIN_FILE	"/data/load.ebin"

static int uart_fd;
//static pthread_t encryption_thread;
static pthread_mutex_t encryption_mutex;
//static pthread_cond_t encryption_cond;

void byte2hexString(const  char* code,unsigned int len,char * hex){
	unsigned int i,tmp,j=0;
	char parse[] = "0123456789abcdef";
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

#define CYCLE_TIME 1000

unsigned int read_data(char* read_buf, unsigned int read_len, unsigned int timeout){
	unsigned int size = 0;
	char buf[2048] = { 0 };
	unsigned int cycle=0;

	pthread_mutex_lock(&encryption_mutex);
	//usleep(CYCLE_TIME);
	size = read(uart_fd,buf,read_len);
	while(size<=3){
		usleep(CYCLE_TIME);
		size = read(uart_fd,buf,read_len);
		if (cycle++ > timeout)
			goto read_failed;
	}
	memcpy((void*)read_buf,(void*)buf,size);
	ALOGE("read_data  cycle .used.(%d) 	timeout.(%d)",cycle,timeout);
	pthread_mutex_unlock(&encryption_mutex);
	return size;

read_failed:
	ALOGE("read_failed  cycle .used.(%d) 	timeout.(%d)",cycle,timeout);
	pthread_mutex_unlock(&encryption_mutex);
	return size;
}


int open_uart(void){
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

	options.c_cc[VTIME]    = 0;
	options.c_cc[VMIN]     = 0;

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
/*	int value = 0;
	int timeout = 300;
	int i;*/

	write(uart_fd,cmd,len);
/*	while(!value && timeout--){
		ioctl(uart_fd,TIOCSERGETLSR,&value);
	}*/
}

int GetProgram(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x00;
	buf_cmd[2] = 0x0c;
	buf_cmd[3] = 0x1d;
	buf_cmd[4] = 0x03;
	write_uart(buf_cmd,5);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("GetProgram  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("GetProgram  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int IfBootFireware(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x00;
	buf_cmd[2] = 0x04;
	buf_cmd[3] = 0x1d;
	buf_cmd[4] = 0x0b;
	write_uart(buf_cmd,5);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("IfBootFireware  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("IfBootFireware  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int SetBand(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x01;
	buf_cmd[2] = 0x97;
	buf_cmd[3] = 0x01;
	buf_cmd[4] = 0x4b;
	buf_cmd[5] = 0xbc;
	write_uart(buf_cmd,6);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("SetBand  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("SetBand  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int SetRxPower(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x02;
	buf_cmd[2] = 0x92;
	buf_cmd[3] = 0x0b;
	buf_cmd[4] = 0xb8;
	buf_cmd[5] = 0x4a;
	buf_cmd[6] = 0xe1;
	write_uart(buf_cmd,7);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("SetRxPower  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("SetRxPower  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int SetTxPower(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x02;
	buf_cmd[2] = 0x92;
	buf_cmd[3] = 0x0b;
	buf_cmd[4] = 0xb8;
	buf_cmd[5] = 0x4a;
	buf_cmd[6] = 0xe1;
	write_uart(buf_cmd,7);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("SetTxPower  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("SetTxPower  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int SetProtocol(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x02;
	buf_cmd[2] = 0x93;
	buf_cmd[3] = 0x00;
	buf_cmd[4] = 0x05;
	buf_cmd[5] = 0x51;
	buf_cmd[6] = 0x7d;
	write_uart(buf_cmd,7);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("SetProtocol  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("SetProtocol  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int SetReadConfig(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x03;
	buf_cmd[2] = 0x9a;
	buf_cmd[3] = 0x01;
	buf_cmd[4] = 0x0c;
	buf_cmd[5] = 0x00;
	buf_cmd[6] = 0xa3;
	buf_cmd[7] = 0x5d;
	write_uart(buf_cmd,8);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("SetReadConfig  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("SetReadConfig  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}


int SetANT(){
	char buf_cmd[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	unsigned int len_res = 0;
	unsigned int time_res = 1000;
	buf_cmd[0] = 0xff;
	buf_cmd[1] = 0x03;
	buf_cmd[2] = 0x91;
	buf_cmd[3] = 0x02;
	buf_cmd[4] = 0x02;
	buf_cmd[5] = 0x02;
	buf_cmd[6] = 0x41;
	buf_cmd[7] = 0xC6;
	write_uart(buf_cmd,8);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("SetANT  len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("SetANT  ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
	return 0;
}

int ReadCard(){
	char buf_cmd_start[32] = { 0 };
	char buf_cmd_stop[16] = { 0 };
	int i;
	char buf_res[16] = { 0 };
	char buf_more[64] = { 0 };
	unsigned int len_res = 0;
	unsigned int len_more = 0;
	unsigned int time_res = 2000;
	unsigned int time_more = 1000;
	buf_cmd_start[0] = 0xff;
	buf_cmd_start[1] = 0x10;
	buf_cmd_start[2] = 0x2f;
	buf_cmd_start[3] = 0x00;
	buf_cmd_start[4] = 0x00;
	buf_cmd_start[5] = 0x01;
	buf_cmd_start[6] = 0x22;
	buf_cmd_start[7] = 0x00;
	buf_cmd_start[8] = 0x00;
	buf_cmd_start[9] = 0x05;
	buf_cmd_start[10] = 0x07;
	buf_cmd_start[11] = 0x22;
	buf_cmd_start[12] = 0x10;
	buf_cmd_start[13] = 0x00;
	buf_cmd_start[14] = 0x1b;
	buf_cmd_start[15] = 0x03;
	buf_cmd_start[16] = 0xe8;
	buf_cmd_start[17] = 0x01;
	buf_cmd_start[18] = 0xff;
	buf_cmd_start[19] = 0xdd;
	buf_cmd_start[20] = 0x2b;
	write_uart(buf_cmd_start,21);
	len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
	ALOGE("ReadCard start len_res.(%d)",len_res);
	if(!len_res)
		return -1;
	for(i=0;i<len_res;i++)
		ALOGE("ReadCard start ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
while(1){
	len_more = read_data(buf_more,sizeof(buf_more)/sizeof(buf_more[0]),time_more);
	ALOGE("ReadCard  len_more.(%d)",len_more);
	if(!len_more){
		buf_cmd_stop[0] = 0xff;
		buf_cmd_stop[1] = 0x03;
		buf_cmd_stop[2] = 0x2f;
		buf_cmd_stop[3] = 0x00;
		buf_cmd_stop[4] = 0x00;
		buf_cmd_stop[5] = 0x02;
		buf_cmd_stop[6] = 0x5e;
		buf_cmd_stop[7] = 0x86;
		write_uart(buf_cmd_stop,8);
		len_res = read_data(buf_res,sizeof(buf_res)/sizeof(buf_res[0]),time_res);
		ALOGE("ReadCard stop len_res.(%d)",len_res);
		if(!len_res)
			return -1;
		for(i=0;i<len_res;i++)
			ALOGE("ReadCard stop ->buf_res[%d]->(0x%2x)",  i, buf_res[i]);
		return 0;
	}
	for(i=0;i<len_more;i++)
		ALOGE("ReadCard  ->buf_more[%d]->(0x%2x)",  i, buf_more[i]);
	//return 0;
}

}








int get_version(int argi){
	//unsigned int convert_len;
	//char convert_buf[2048] = { 0 };
	char lrc;
	char no_p = (char)argi;
	unsigned int i;
	char buf_ack[6] = { 0 };
	char buf_info[256] = { 0 };
	char buf_more_info[1024] = { 0 };
	char buf_cmd[64] = { 0 };
	unsigned int len_ack,len_info,len_more_info;
	unsigned int timeout_ack,timeout_info,timeout_more_info;
	timeout_ack = 300;
	timeout_info = 500;
	timeout_more_info = 1000;
	buf_cmd[0]=0xaa;
	buf_cmd[1]=0xbb;
	buf_cmd[2]=no_p;
	buf_cmd[3]=0x00;
	buf_cmd[4]=0x04;
	buf_cmd[5]=0x01;
	buf_cmd[6]=0x01;
	buf_cmd[7]=0xcc;
	buf_cmd[8]=0xdd;
	lrc = buf_cmd[2]^buf_cmd[3];
	for(i=4;i<9;i++)
		lrc=lrc^buf_cmd[i];
	ALOGE("get_version lrc==0x%2x",lrc);
	buf_cmd[9]=lrc;
	//for(i=0;i<10;i++)
		//ALOGE("get_version buf_cmd[%d]==0x%2x",i,buf_cmd[i]);
	write_uart(buf_cmd,10);

	len_ack = read_data(buf_ack,sizeof(buf_ack)/sizeof(buf_ack[0]),timeout_ack);
	if(len_ack!=6 || buf_ack[1]!=0xcc || buf_ack[2]!=buf_cmd[2])
	   goto ack_rec_failed;
	ALOGE("get_version ack len_ack.(%d)",len_ack);
	len_info = read_data(buf_info,sizeof(buf_info)/sizeof(buf_info[0]),timeout_info);
	if(len_info<20 || buf_info[1]!=0xbb)
	   goto info_rec_failed;
	ALOGE("get_version	info len_info.(%d)",len_info);
	//usleep(10*1000);
	write_uart(buf_ack,len_ack);

#if 1
	printf("get_version read more version info");
	len_more_info = read_data(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	ALOGE("get_version more info len_more_info.(%d)",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("get_version  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		ALOGE("get_version no more data");
#endif
	return 0;
ack_rec_failed:
	ALOGE("	ERROR!	ack_rec_failed");
	ALOGE("get_version ack len_ack.(%d)",len_ack);
	for(i=0;i<len_ack;i++)
		ALOGE("get_version ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	len_more_info = read_data(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	ALOGE("get_version more info len_more_info.(%d)",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("get_version  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		ALOGE("get_version no more data");
	return -1;
info_rec_failed:
	ALOGE("	ERROR!	info_rec_failed");
	ALOGE("get_version  info len_info.(%d)",len_info);
	for(i=0;i<len_info;i++)
		ALOGE("get_version  ->buf_info[%d]->(0x%2x)",  i, buf_info[i]);
	len_more_info = read_data(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	ALOGE("get_version more info len_more_info.(%d)",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("get_version  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		ALOGE("get_version no more data");
	return -2;
}

int read_card(int argi){
	char lrc;
	char no_p = (char)argi;
	unsigned int i;
	char buf_ack[6] = { 0 };
	char buf_info[1024] = { 0 };
	char buf_more_info[1024] = { 0 };
	char buf_cmd[64] = { 0 };
	unsigned int len_ack,len_info,len_more_info;
	unsigned int timeout_ack,timeout_info,timeout_more_info;
	timeout_ack = 300;
	timeout_info = 20000;
	timeout_more_info = 1000;
	buf_cmd[0]=0xaa;
	buf_cmd[1]=0xbb;
	buf_cmd[2]=no_p;
	buf_cmd[3]=0x00;
	buf_cmd[4]=0x07;
	buf_cmd[5]=0x03;
	buf_cmd[6]=0x04;
	buf_cmd[7]=0x10;
	buf_cmd[8]=0x03;
	buf_cmd[9]=0x01;
	buf_cmd[10]=0xcc;
	buf_cmd[11]=0xdd;
	lrc = buf_cmd[2]^buf_cmd[3];

	for(i=4;i<12;i++)
		lrc=lrc^buf_cmd[i];
	ALOGE("read_card lrc==0x%2x",lrc);
	buf_cmd[12]=lrc;

	//for(i=0;i<13;i++)
		//ALOGE("read_card buf_cmd[%d]==0x%2x",i,buf_cmd[i]);
	write_uart(buf_cmd,13);

	len_ack = read_data(buf_ack,sizeof(buf_ack)/sizeof(buf_ack[0]),timeout_ack);

	if(len_ack!=6 || buf_ack[1]!=0xcc || buf_ack[2]!=buf_cmd[2])
	   goto ack_rec_failed;
	ALOGE("read_card ack len_ack.(%d)",len_ack);
	printf("inset card now\n");
	len_info = read_data(buf_info,sizeof(buf_info)/sizeof(buf_info[0]),timeout_info);
	if(len_info<20 || buf_info[1]!=0xbb)
	   goto info_rec_failed;
	ALOGE("read_card  info len_info.(%d)",len_info);
	//usleep(10*1000);
	write_uart(buf_ack,len_ack);

	//for(i=0;i<len_ack;i++)
	   //ALOGE("read_card ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	//for(i=0;i<len_info;i++)
	   //ALOGE("read_card  ->buf_info[%d]->(0x%2x)",  i, buf_info[i]);

#if 1
	printf("read_card read more card info\n");
	len_more_info = read_data(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	ALOGE("read_card  more_info len_more_info.(%d)",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("read_card  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		ALOGE("read_card no more data");
#endif
	return 0;
ack_rec_failed:
	ALOGE("	ERROR!	ack_rec_failed");
	ALOGE("read_card ack len_ack.(%d)",len_ack);
	for(i=0;i<len_ack;i++)
		ALOGE("read_card ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	len_more_info = read_data(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	ALOGE("read_card more info len_more_info.(%d)",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("read_card  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		ALOGE("read_card no more data");
	return -1;
info_rec_failed:
	ALOGE("	ERROR!	info_rec_failed");
	ALOGE("read_card  info len_info.(%d)",len_info);
	for(i=0;i<len_info;i++)
		ALOGE("read_card  ->buf_info[%d]->(0x%2x)",  i, buf_info[i]);
	len_more_info = read_data(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	ALOGE("read_card more info len_more_info.(%d)",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("read_card  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		ALOGE("read_card no more data");
	return -2;

}


int send_binfile(int argi){
	FILE *file;
	unsigned int i;
	char no_p = (char)argi;
	char num,lrc;
	char buf_ack[6] = { 0 };
	char buf_info[64] = { 0 };
	unsigned int len_ack,len_info;
	unsigned int timeout_ack,timeout_info;
	timeout_ack = 300;
	timeout_info = 5000;
	char *buffer;
	char buf_cmd_first[64] = { 0 };
	char buf_cmd_middle[1048] = { 0 };
	char buf_cmd_last[1048] = { 0 };
	unsigned int fileLen;
	unsigned int len_has_send = 0;
	unsigned short len_rem;
	char len_quo;
	unsigned int total_lrc = 0;
	unsigned short final_lrc = 0;

	file = fopen(TEST_BIN_FILE, "rb");
	if (!file){
		printf("no such bin file\n");
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);
	printf("fileLen==0x%8x\n",fileLen);
	buffer=(char *)malloc(fileLen+1);
	memset(buffer,0,sizeof(buffer)/sizeof(buffer[0]));
	fread(buffer, 1, fileLen+1, file);
	fclose(file);
	len_quo = fileLen/1024;
	len_rem = fileLen%1024;
	printf("len_quo==0x%2x\n",len_quo);
	printf("len_rem==0x%4x\n",len_rem);
	for(i=0;i<fileLen;i++){
	 total_lrc+=buffer[i];
	 if (total_lrc >= 0x0000ffff)
		 total_lrc&=0x0000ffff;
		 //total_lrc=0x0;
	}
	final_lrc = (unsigned short)total_lrc;
	printf("total_lrc==0x%8x\n",total_lrc);
	printf("final_lrc==0x%4x\n",final_lrc);

//package0 begin
#if 1
	buf_cmd_first[0]=0xaa;
	buf_cmd_first[1]=0xbb;
	buf_cmd_first[2]=no_p;
	buf_cmd_first[3]=0x00;
	buf_cmd_first[4]=0x0c;
	buf_cmd_first[5]=0x01;
	buf_cmd_first[6]=0x0c;
	buf_cmd_first[7]=0x00;
	buf_cmd_first[8]=0x00;
	buf_cmd_first[9]=(fileLen>>24)&0xff;
	buf_cmd_first[10]=(fileLen>>16)&0xff;
	buf_cmd_first[11]=(fileLen>>8)&0xff;
	buf_cmd_first[12]=fileLen&0xff;
	buf_cmd_first[13]=(final_lrc>>8)&0xff;
	buf_cmd_first[14]=final_lrc&0xff;
	buf_cmd_first[15]=0xcc;
	buf_cmd_first[16]=0xdd;
	lrc = buf_cmd_first[2]^buf_cmd_first[3];
	for(i=4;i<17;i++)
		lrc=lrc^buf_cmd_first[i];
	ALOGE("send_binfile lrc_first==0x%2x",lrc);
	buf_cmd_first[17]=lrc;
	//for(i=0;i<18;i++)
		//ALOGE("buf_cmd_first[%d]==0x%2x",i,buf_cmd_first[i]);
	write_uart(buf_cmd_first,18);
	//sleep(2);
	len_ack = read_data(buf_ack,sizeof(buf_ack)/sizeof(buf_ack[0]),timeout_ack);
	//ALOGE("send_binfile ack len_ack.(%d)",len_ack);
	if(len_ack!=6 || buf_ack[1]!=0xcc || buf_ack[2]!=buf_cmd_first[2])
	   goto packagefirst_ack_failed;

	len_info = read_data(buf_info,sizeof(buf_info)/sizeof(buf_info[0]),timeout_info);
	//ALOGE("send_binfile  info len_info.(%d)",len_info);
	if(len_info!=11 || buf_info[1]!=0xbb || buf_info[7]!=0x0)
	   goto packagefirst_info_failed;

	//usleep(10*1000);
	write_uart(buf_ack,len_ack);

	//for(i=0;i<len_ack;i++)
		//ALOGE("package0 buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	//for(i=0;i<len_info;i++)
		//ALOGE("package0  buf_info[%d]->(0x%2x)", i, buf_info[i]);
	usleep(100*1000);
#endif
//package0 end

//package_middle begin
#if 1
	for(num=0;num<len_quo;num++){
		total_lrc=0x0;
		final_lrc=0x0;
		for(i=num*1024;i<(num+1)*1024;i++){
			total_lrc+=buffer[i];
			if (total_lrc >= 0x0000ffff)
				total_lrc&=0x0000ffff;
				 //total_lrc=0x0;
		}
		final_lrc = (unsigned short)total_lrc;
		//printf("total_lrc[0x%2x]==0x%8x\n",num,total_lrc);
		//printf("final_lrc[0x%2x]==0x%4x\n",num,final_lrc);
		len_has_send = 1024*(num+1);
		//len_has_send = 1024*num;
		buf_cmd_middle[0]=0xaa;
		buf_cmd_middle[1]=0xbb;
		buf_cmd_middle[2]=no_p+num+1;
		buf_cmd_middle[3]=0x04;
		//buf_cmd_middle[4]=0x0a;
		buf_cmd_middle[4]=0x0c;
		buf_cmd_middle[5]=0x01;
		buf_cmd_middle[6]=0x0c;
		buf_cmd_middle[7]=0x00;
		buf_cmd_middle[8]=num+1;
		buf_cmd_middle[9]=(len_has_send>>24)&0xff;
		buf_cmd_middle[10]=(len_has_send>>16)&0xff;
		buf_cmd_middle[11]=(len_has_send>>8)&0xff;
		buf_cmd_middle[12]=len_has_send&0xff;
		buf_cmd_middle[13]=(final_lrc>>8)&0xff;
		buf_cmd_middle[14]=final_lrc&0xff;
		buf_cmd_middle[1039]=0xcc;
		buf_cmd_middle[1040]=0xdd;
		//buf_cmd_middle[1037]=0xcc;
		//buf_cmd_middle[1038]=0xdd;
		//memcpy((void*)(buf_cmd_middle+13),(void*)(buffer+num*1024),1024);
		memcpy((void*)(buf_cmd_middle+15),(void*)(buffer+num*1024),1024);
		memset(buf_ack,0,sizeof(buf_ack)/sizeof(buf_ack[0]));
		memset(buf_info,0,sizeof(buf_info)/sizeof(buf_info[0]));
		lrc = buf_cmd_middle[2]^buf_cmd_middle[3];
		for(i=4;i<1041;i++){
		//for(i=4;i<1039;i++){
			lrc = lrc^buf_cmd_middle[i];
		}
		ALOGE("send_binfile lrc_num[0x%2x]==0x%2x",num,lrc);
		buf_cmd_middle[1041]=lrc;
		//buf_cmd_middle[1039]=lrc;
		//for(i=0;i<20;i++)
			//ALOGE("buf_cmd_middle[%d]==0x%2x",i,buf_cmd_middle[i]);
		write_uart(buf_cmd_middle,1042);
		//write_uart(buf_cmd_middle,1040);
		//sleep(2);
		len_ack = read_data(buf_ack,sizeof(buf_ack)/sizeof(buf_ack[0]),timeout_ack);
		//ALOGE("package_num[0x%2x] ack len_ack.(%d) ",num,len_ack);
		if(len_ack!=6 || buf_ack[1]!=0xcc || buf_ack[2]!=buf_cmd_middle[2])
			goto packagemiddle_ack_failed;

		len_info = read_data(buf_info,sizeof(buf_info)/sizeof(buf_info[0]),timeout_info);
		//ALOGE("package_num[0x%2x] info len_info.(%d) ",num,len_info);
		if(len_info!=11 || buf_info[1]!=0xbb || buf_info[7]!=0x0)
			goto packagemiddle_info_failed;

		//usleep(10*1000);
		write_uart(buf_ack,len_ack);

		//for(i=0;i<len_ack;i++)
			//ALOGE("package_middle ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
		//for(i=0;i<len_info;i++)
			//ALOGE("package_middle  ->buf_info[%d]->(0x%2x)",	i, buf_info[i]);
		usleep(80*1000);
		}
		printf("package_middle EXIT\n");
#endif
//package middle end

//package last begin
#if 1
	printf("package_last ENTER\n");
	total_lrc=0x0;
	final_lrc=0x0;
	for(i=len_quo*1024;i<fileLen;i++){
		total_lrc+=buffer[i];
		if (total_lrc >= 0x0000ffff)
			total_lrc&=0x0000ffff;
			 //total_lrc=0x0;
	}
	final_lrc = (unsigned short)total_lrc;
	//printf("total_lrc[0x%2x]==0x%8x\n",num,total_lrc);
	//printf("final_lrc[0x%2x]==0x%4x\n",num,final_lrc);

	buf_cmd_last[0]=0xaa;
	buf_cmd_last[1]=0xbb;
	buf_cmd_last[2]=no_p+len_quo+1;
	//buf_cmd_last[3]=((len_rem+0x0a)>>8)&0xff;
	//buf_cmd_last[4]=(len_rem+0x0a)&0xff;
	buf_cmd_last[3]=((len_rem+0x0c)>>8)&0xff;
	buf_cmd_last[4]=(len_rem+0x0c)&0xff;
	buf_cmd_last[5]=0x01;
	buf_cmd_last[6]=0x0c;
	buf_cmd_last[7]=0xff;
	buf_cmd_last[8]=0xff;
	buf_cmd_last[9]=(fileLen>>24)&0xff;
	buf_cmd_last[10]=(fileLen>>16)&0xff;
	buf_cmd_last[11]=(fileLen>>8)&0xff;
	buf_cmd_last[12]=fileLen&0xff;
	buf_cmd_last[13]=(final_lrc>>8)&0xff;
	buf_cmd_last[14]=final_lrc&0xff;
	buf_cmd_last[15+len_rem]=0xcc;
	buf_cmd_last[16+len_rem]=0xdd;
	//buf_cmd_last[13+len_rem]=0xcc;
	//buf_cmd_last[14+len_rem]=0xdd;
	memcpy((void*)(buf_cmd_last+15),(void*)(buffer+len_quo*1024),len_rem);
	//memcpy((void*)(buf_cmd_last+13),(void*)(buffer+len_quo*1024),len_rem);
	memset(buf_ack,0,sizeof(buf_ack)/sizeof(buf_ack[0]));
	memset(buf_info,0,sizeof(buf_info)/sizeof(buf_info[0]));
	lrc = buf_cmd_last[2]^buf_cmd_last[3];
	for(i=4;i<len_rem+17;i++)
	//for(i=4;i<len_rem+15;i++)
		lrc=lrc^buf_cmd_last[i];
	ALOGE("send_binfile last_lrc==0x%2x",lrc);
	//printf("num==0x%2x\n",num);
	buf_cmd_last[17+len_rem]=lrc;
	//buf_cmd_last[15+len_rem]=lrc;
	//for(i=0;i<20;i++)
		//ALOGE("buf_cmd_last[%d]==0x%2x",i,buf_cmd_last[i]);
	//for(i=len_rem;i<len_rem+18;i++)
	//for(i=len_rem;i<len_rem+16;i++)
		//ALOGE("buf_cmd_last[%d]==0x%2x",i,buf_cmd_last[i]);
	//write_uart(buf_cmd_last,len_rem+16);
	write_uart(buf_cmd_last,len_rem+18);
	//sleep(2);
	len_ack = read_data(buf_ack,sizeof(buf_ack)/sizeof(buf_ack[0]),timeout_ack);
	//ALOGE("package_last ack len_ack.(%d) ",len_ack);
	if(len_ack!=6 || buf_ack[1]!=0xcc || buf_ack[2]!=buf_cmd_last[2])
		goto packagelast_ack_failed;
	len_info = read_data(buf_info,sizeof(buf_info)/sizeof(buf_info[0]),timeout_info);
	//ALOGE("package_last info len_info.(%d) ",len_info);
	if(len_info!=11 || buf_info[1]!=0xbb || buf_info[7]!=0x0)
		goto packagelast_info_failed;

	//usleep(10*1000);
	write_uart(buf_ack,len_ack);

	//for(i=0;i<len_ack;i++)
		//ALOGE("package_last ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	//for(i=0;i<len_info;i++)
		//ALOGE("package_last  ->buf_info[%d]->(0x%2x)",	i, buf_info[i]);
#endif
//package last end
	printf("binfile send done!\n");
	free(buffer);
	sleep(10);
	printf("sendbinfile exit\n");
	return 0;

packagefirst_ack_failed:
	ALOGE("	ERROR!	packagefirst_ack_failed");
	ALOGE("package_first ack len_ack.(%d)",len_ack);
	for(i=0;i<len_ack;i++)
		ALOGE("package_first ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	return -1;

packagefirst_info_failed:
	ALOGE("	ERROR!	packagefirst_info_failed");
	ALOGE("package_first  info len_info.(%d)",len_info);
	for(i=0;i<len_info;i++)
		ALOGE("package_first ->buf_info[%d]->(0x%2x)", i, buf_info[i]);
	return -2;

packagemiddle_ack_failed:
	ALOGE("	ERROR!	packagemiddle_ack_failed");
	ALOGE("package_middle ack len_ack.(%d)",len_ack);
	for(i=0;i<len_ack;i++)
		ALOGE("package_middle ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	return -3;

packagemiddle_info_failed:
	ALOGE("	ERROR!	packagemiddle_info_failed");
	ALOGE("package_middle  info len_info.(%d)",len_info);
	for(i=0;i<len_info;i++)
		ALOGE("package_middle ->buf_info[%d]->(0x%2x)", i, buf_info[i]);
	return -4;

packagelast_ack_failed:
	ALOGE("	ERROR!	packagelast_ack_failed");
	ALOGE("package_last ack len_ack.(%d)",len_ack);
	for(i=0;i<len_ack;i++)
		ALOGE("package_last ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	return -5;

packagelast_info_failed:
	ALOGE("	ERROR!	packagelast_info_failed");
	ALOGE("package_last  info len_info.(%d)",len_info);
	for(i=0;i<len_info;i++)
		ALOGE("package_last ->buf_info[%d]->(0x%2x)", i, buf_info[i]);
	return -6;
}


int main(int argc, char *argv[]) {

	int res = 0;
	int arg = atoi(argv[1]);
	printf("get arg.(%d)\n",arg);
	printf("arg tran to hex.(0x%2x)\n",arg);
	uart_fd = open_uart();
	printf(" uart ready \n");
	//sleep(1);

#if 1
	printf("get GetProgram\n");
	res = GetProgram();
	if(res){
		printf(" GetProgram  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	GetProgram \n");

	printf("get IfBootFireware\n");
	res = IfBootFireware();
	if(res){
		printf(" IfBootFireware  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	IfBootFireware \n");
	sleep(1);

	printf("get SetBand\n");
	res = SetBand();
	if(res){
		printf(" SetBand  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	SetBand \n");

	printf("get SetRxPower\n");
	res = SetRxPower();
	if(res){
		printf(" SetRxPower  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	SetRxPower \n");

	printf("get SetProtocol\n");
	res = SetProtocol();
	if(res){
		printf(" SetProtocol  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	SetProtocol \n");

	printf("get SetReadConfig\n");
	res = SetReadConfig();
	if(res){
		printf(" SetReadConfig  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	SetReadConfig \n");

	printf("get SetANT\n");
	res = SetANT();
	if(res){
		printf(" SetANT  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	SetANT \n");

	printf("get ReadCard\n");
	res = ReadCard();
	if(res){
		printf(" ReadCard  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	ReadCard \n");

#endif
#if 0
	sleep(1);
	printf("get_version\n");
	res = get_version(arg);
	if(res){
		printf(" get_version  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	get_version \n");
#endif
#if 0
	sleep(1);
	printf("read_card\n");
	res = read_card(arg);
	if(res){
		printf(" read_card	failed.(%d) \n", res);
		return -1;
	}
	printf("   succeed	read_card \n");
#endif

#if 0
	sleep(1);
	printf("send_binfile\n");
	res = send_binfile(arg);
	if(res){
		printf(" send_binfile  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	send_binfile \n");
#endif

	printf(" main	test	quit	->>> [%d]	\n",res);
	return res;

}

