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
#include "secure_uart.h"


#define TEST_BIN_FILE	"/data/load.ebin"

static int serial_fd;
//static pthread_t securemodule_thread;
//static pthread_mutex_t securemodule_mutex;
//static pthread_cond_t securemodule_cond;

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

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;

    if ( tcgetattr(fd,  &oldtio) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }
     bzero( &newtio, sizeof( newtio ) );
     //设置字符大小
     newtio.c_cflag |= CLOCAL | CREAD;
     newtio.c_cflag &= ~CSIZE;
     //设置数据位
     switch( nBits )
     {
        case 7:
         newtio.c_cflag |= CS7;
        break;
        case 8:
         newtio.c_cflag |= CS8;
        break;
     }
     //设置校验位
     switch( nEvent )
     {
         case 'O':
          newtio.c_cflag |= PARENB;
          newtio.c_cflag |= PARODD;
          newtio.c_iflag |= (INPCK | ISTRIP);
         break;
         case 'E':
          newtio.c_iflag |= (INPCK | ISTRIP);
          newtio.c_cflag |= PARENB;
          newtio.c_cflag &= ~PARODD;
         break;
         case 'N':
          newtio.c_cflag &= ~PARENB;
         break;
     }
     //设置波特率
     switch( nSpeed )
     {
         case 2400:
               cfsetispeed(&newtio, B2400);
               cfsetospeed(&newtio, B2400);
               break;
         case 4800:
             cfsetispeed(&newtio, B4800);
             cfsetospeed(&newtio, B4800);
             break;
         case 9600:
             cfsetispeed(&newtio, B9600);
             cfsetospeed(&newtio, B9600);
             break;
         case 115200:
             cfsetispeed(&newtio, B115200);
             cfsetospeed(&newtio, B115200);
             break;
         default:
             cfsetispeed(&newtio, B9600);
             cfsetospeed(&newtio, B9600);
             break;
     }
     //设置停止位
     if( nStop == 1 )
         newtio.c_cflag &= ~CSTOPB;
     else if ( nStop == 2 )
         newtio.c_cflag |= CSTOPB;

     //设置等待时间和最小接收字符
     newtio.c_cc[VTIME] = 0;
     newtio.c_cc[VMIN] = 0;

     tcflush(fd,TCIFLUSH);

     if((tcsetattr(fd,TCSANOW,&newtio))!=0)
     {
         perror("tty set error");
         return -1;
     }

     return 0;
}

int serial_init(void)
{
    int ret;

    serial_fd = open(SERIAL_DEV, O_RDWR|O_NOCTTY|O_NDELAY);

    if (serial_fd <= 0)
    {
        printf(" + open tty error + ");
        return -1;
    }

    printf(" + open tty success + ");

    ret = set_opt(serial_fd, 115200, 8, 'N', 1);

    if (ret == 0)
    {
        printf(" + set tty success + ");
    }

    return ret;
}

int RecvData(unsigned char *data, int len, int timeout)
{
    int ret;
    fd_set fdset;
    struct timeval tv;

    if (serial_fd < 0)
    {
        ret = serial_init();
        if (ret < 0)
        {
            printf(" + set tty opt error + ");
            return -1;
        }
    }

    FD_ZERO(&fdset);
    FD_SET(serial_fd, &fdset);

    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout%1000) * 1000;

    ret = select(serial_fd+1, &fdset, 0, 0, &tv);

    if (ret > 0)
    {
        ret = read(serial_fd, data, len);

        printf(" + tty RecvData success + ");
    }
    else if (ret == 0)
    {
        printf(" + read tty timeout + ");
    }
    else
    {
        printf(" + read tty error + ");
    }

    return ret;
}


void write_uart(const char* cmd,unsigned int len){
//	int value = 0;
//	int timeout = 300;

	write(serial_fd,cmd,len);
//	while(!value && timeout--){
//		ioctl(serial_fd,TIOCSERGETLSR,&value);
//	}
}

int get_version(int argi){
	//unsigned int convert_len;
	char convert_buf[2048] = { 0 };
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
	printf("get_version lrc==0x%2x\n",lrc);
	buf_cmd[9]=lrc;
	//for(i=0;i<10;i++)
		//ALOGE("get_version buf_cmd[%d]==0x%2x",i,buf_cmd[i]);
	write_uart(buf_cmd,10);

	len_ack = RecvData(buf_ack,sizeof(buf_ack)/sizeof(buf_ack[0]),timeout_ack);
	if(len_ack!=6 || buf_ack[1]!=0xcc || buf_ack[2]!=buf_cmd[2])
	   goto ack_rec_failed;
	printf("get_version ack len_ack.(%d)\n",len_ack);

	len_info = RecvData(buf_info,sizeof(buf_info)/sizeof(buf_info[0]),timeout_info);
	//if(len_info<20 || buf_info[1]!=0xbb)
	   //goto info_rec_failed;
	printf("get_version	info len_info.(%d)\n",len_info);
	write_uart(buf_ack,len_ack);

	for(i=0;i<len_ack;i++)
		ALOGE("get_version ->buf_ack[%d]->(0x%2x)", i, buf_ack[i]);
	for(i=0;i<len_info;i++)
		ALOGE("get_version  ->buf_info[%d]->(0x%2x)",  i, buf_info[i]);

#if 1
	printf("get_version read more version info\n");
	len_more_info = RecvData(buf_more_info,sizeof(buf_more_info)/sizeof(buf_more_info[0]),timeout_more_info);
	printf("get_version more info len_more_info.(%d)\n",len_more_info);
	if(len_more_info){
		for(i=0;i<len_more_info;i++)
		   ALOGE("get_version  ->buf_more_info[%d]->(0x%2x)",  i, buf_more_info[i]);
	}else
		printf("get_version no more data\n");
#endif
	return 0;
ack_rec_failed:
	printf("	ERROR!	ack_rec_failed\n");
	printf("get_version ack len_ack.(%d)\n",len_ack);
	return -1;
info_rec_failed:
	printf("	ERROR!	info_rec_failed\n");
	printf("get_version  info len_info.(%d)\n",len_info);
	return -2;
}


int main(int argc, char *argv[]) {

	int res = 0;
	int arg = atoi(argv[1]);
	printf("get arg.(%d)\n",arg);
	printf("arg tran to hex.(0x%2x)\n",arg);

	if (serial_init() < 0)
	{
		printf(" uart error \n");
		return -1;
	}

	//sleep(1);

#if 1
	sleep(1);
	printf("get_version\n");
	res = get_version(arg);
	if(res)
	{
		printf(" get_version  failed.(%d) \n",res);
		return -1;
	}
	printf("  succeed	get_version \n");
#endif


	printf(" main	test	quit	->>> [%d]	\n",res);
	return res;

}

