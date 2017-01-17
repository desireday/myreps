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

