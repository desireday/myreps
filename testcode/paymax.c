/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The  Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/paymax.h>

/******************************************************************************/

#define UART_DEV_NAME "/dev/ttyHSL1"

static pthread_once_t paymax_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t paymax_mutex = PTHREAD_MUTEX_INITIALIZER;
static int uart_fd;

/**
 * device methods
 */

void init_global_var(void)
{
    // init the mutex
    pthread_mutex_init(&paymax_mutex, NULL);
}


/** Close the paymax device */
static int close_paymax(struct paymax_device_t *dev){
	close(uart_fd);
    if (dev) {
        free(dev);
    }
    return 0;
}

int open_uart(void){
	int fd = -1;
	struct termios options;

	fd = open(UART_DEV_NAME,O_RDWR | O_NOCTTY);
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

static void writeCmdToMax(struct paymax_device_t *dev, const char* cmd_buf,unsigned int cmd_len){
/*	int value = 0;
	int timeout = 300;
	int i;*/

	write(uart_fd,cmd_buf,cmd_len);
/*	while(!value && timeout--){
		ioctl(uart_fd,TIOCSERGETLSR,&value);
	}*/
}

static unsigned int readDataFromMax(struct paymax_device_t *dev, char* read_buf, unsigned int read_len, unsigned int timeout){
	unsigned int size = 0;
	char buf[2048] = { 0 };
	unsigned int cycle=0;

	pthread_mutex_lock(&paymax_mutex);
	//usleep(1000);
	size = read(uart_fd,buf,read_len);
	while(size<=3){
		usleep(1000);
		size = read(uart_fd,buf,read_len);
		if (cycle++ > timeout)
			goto read_failed;
	}
	ALOGE("read_data  cycle .used.(%d) 	timeout.(%d)",cycle,timeout);
	memcpy((void*)read_buf,(void*)buf,size);
	pthread_mutex_unlock(&paymax_mutex);
	return size;

read_failed:
	ALOGE("read_failed  cycle .used.(%d) 	timeout.(%d)",cycle,timeout);
	pthread_mutex_unlock(&paymax_mutex);
	return size;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a paymax device using name */
static int open_paymax(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{

    pthread_once(&paymax_init, init_global_var);

    struct paymax_device_t *dev = malloc(sizeof(struct paymax_device_t));

    if(!dev)
        return -ENOMEM;

    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_paymax;
    dev->writeCmdToMax = writeCmdToMax;
	dev->readDataFromMax = readDataFromMax;

	if((uart_fd=open_uart())<0)
		return -1;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t paymax_module_methods = {
    .open =  open_paymax,
};

/*
 * The paymax Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = PAYMAX_HARDWARE_MODULE_ID,
    .name = "paymax Module",
    .author = "SDTM, Inc.",
    .methods = &paymax_module_methods,
};
