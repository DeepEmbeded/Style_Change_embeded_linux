---
Uart 串口通信编程
---

关于 Uart 的引脚以及重要参数就不赘述了，去八股文看看就行。



# 一、串口参数设置

## 1. termios 结构体

POSIX 规范中定义了标准接口-termios 结构体，在 linux 中用 termios 来设置串口参数。

```c
struct termios
{
    tcflag_t c_iflag; /* input mode flags */
    tcflag_t c_oflag; /* output mode flags */
    tcflag_t c_cflag; /* control mode flags */
    tcflag_t c_lflag; /* local mode flags */
    cc_t c_line; /* line discipline */
    cc_t c_cc[NCCS]; /* control characters */
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
#define _HAVE_STRUCT_TERMIOS_C_ISPEED 1
#define _HAVE_STRUCT_TERMIOS_C_OSPEED 1
};

/*
* c_iflag: 输入模式标志，决定如何解释和处理接收的字符
* c_oflag：输出模式标志，决定如何解释和处理发送到 tty 设备的字符；
* c_cflag：控制模式标志，决定设备的一系列协议特征，这一标志只对物理设备有效;
* c_lflag：本地模式标志，决定字符在输出前如何收集和处理
*/
```



## 2. 结构体成员



## 3. 相关函数

**tcgetattr()**

```c
int tcgetattr(int fd,struct termois & termios_p);
//成功返回 0，错误返回-1，并且为 errno 置值来指示错误；
```

取得文件描述符（fd）初始值，并把其值赋给 temios_p;函数可以从后台进程中调用；但是，终端属性可能被后来的前台进程所改变。



**tcsetattr()**

```c
int tcsetattr(int fd,int actions,const struct termios *termios_p);
//成功返回 0，错误返回-1，并且为 errno 置值来指示错误；
```

设置与终端相关的参数 (除非需要底层支持却无法满足)，使用 termios_p 引用的 termios 结构。optional_actions（tcsetattr 函数的第二个参数）指定了什么时候改变会起作用：

- TCSANOW：改变立即发生；
- TCSADRAIN：改变在所有写入 fd 的输出都被传输后生效。
- TCSAFLUSH ：改变在所有写入 fd 引用的对象的输出都被传输后生效,所有已接受但未读入的输入都在改变发生前丢弃(同 TCSADRAIN，但会舍弃当前所有值)；



**tcsendbreak()**





**tcdrain()**



**tcflush()**



**tcflow()**



**波特率函数**



# 二、编写程序

## 1. 打开串口设备

```c
fd = open("/dev/ttyS9", O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0){
        perror("/dev/ttyS9");
        printf("Can't Open Serial Port %s \n", "/dev/ttyS9");
        exit(0);
    } else {
        .......
    }
```



O_RDWR | O_NOCTTY | O_NDELAY  分别代表以读写方式打开、不让设备成为控制终端且设置非阻塞模式。



## 2. 获取和写配置参数

tcgetattr()函数 获取 open()函数返回的文件描述符 fd 作为 tcgetattr()的参数，然后创建一个 termios 名为 oldtio 来储存串口旧的参数。



对 newtio 结构体中的 c_cflag 加上 CLOCAL | CREAD，意为使驱动程序启动接收字符装置，同时忽略串口信号线的状态。

另外对 c_cflag 与上了~CSIZE，即设置数据位。

```c
struct termios newtio, oldtio;
if (tcgetattr(fd, &oldtio) != 0) {
perror("tcgetattr");
return -1;
}
bzero(&newtio, sizeof(newtio));
newtio.c_cflag |= CLOCAL | CREAD;
newtio.c_cflag &= ~CSIZE;
```



## 3. 清空缓冲区

```c
tcflush(fd, TCIFLUSH);
```

使用 tcflush()函数清空缓冲区，避免本次将之前残存的数据发送出去引起混乱。



## 4. 配置生效

完成了对 struct termios 结构体各个成员进行配置，还需要将配置参数写入终端设备。通过 tcsetattr()函数将配置参数写入到硬件设备。

```c
((tcsetattr(fd, TCSANOW, &newtio)) != 0){
perror("com set error");
return -1;
}
```



## 5. 串口接收函数

使用 read()函数读缓冲区的数据，即串口接收：

```c
read(fd, p_receive_buff, count); //读串口
```



## 6. 串口发送函数

使用 write()函数发送缓冲区的数据，即串口发送：

```c
write(fd, p_send_buff, count);//发送
```



## 7. 关闭串口设备

```c
close(fd);
```



## 8. 串口通信示例

```c++
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int fd;
unsigned char receive_buff[100];
unsigned int receive_num = 0;

unsigned char send_buff[100] = "test serial port";
unsigned int send_num = 0;

/*
* @description : 串口发送函数
* @param - fd : 文件描述符
* @param - *p_send_buff: 要发送数据缓冲区首地址
* @param - count: 要发送数据长度
* @return : 执行结果
*/

int func_send_frame(int fd, const unsigned char *p_send_buff, const int count)
{
    int Result = 0;

    Result = write(fd, p_send_buff, count);
    if(Result == -1){
        perror("write");
        return 0;
    }
    return Result;
}
/*
* @description : 串口接收函数
* @param - fd : 文件描述符
* @param - *p_receive_buff: 接收数据缓冲区首地址
* @param - count: 最大接收数据长度
* @return : 执行结果
*/
int func_receive_frame(int fd, unsigned char *p_receive_buff, const int count)
{
    // 阻塞用法
    int nread = 0;
    fd_set rd;
    int retval = 0;
    struct timeval timeout = {0, 500};
    
    FD_ZERO(&rd);
    FD_SET(fd, &rd);
    memset(p_receive_buff, 0x0, count);
    retval = select(fd + 1, &rd, NULL, NULL, &timeout);
    switch (retval)
    {
    case 0:
        nread = 0;
        break;

    case 1:
        printf("select%s\n", strerror(errno));
        nread = -1;
        break;
    
    default:
        nread = read(fd, p_receive_buff, count); //读串口
        break;
    }
    return nread;
}


/*
* @description : 主函数
* @return : 执行结果
*/
int main(){
    int result = 0;
    struct termios newtio, oldtio;

    //打开串口 uart9 设置可读写，不被输入影响，不等待外设响应
    fd = open("/dev/ttyS9", O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0){
        perror("/dev/ttyS9");
        printf("Can't Open Serial Port %s \n", "/dev/ttyS9");
        exit(0);
    } else {
        //设置串口参数
        if(tcgetattr(fd, &oldtio) != 0){
            perror("tcgetattr");
            return -1;
        }
        bzero(&newtio, sizeof(newtio));     //清空串口设置
        newtio.c_cflag |= CLOCAL | CREAD;   //打开接收标志和忽略控制线
        newtio.c_cflag &= ~CSIZE;           //清除数据位设置
        newtio.c_cflag |= CS8;              //设置数据位为 8 位
        newtio.c_cflag &= ~PARENB;          //无校验位
        cfsetispeed(&newtio, B9600);        //设置输入波特率为 9600
        cfsetospeed(&newtio, B9600);        //设置输出波特率为 9600
        newtio.c_cflag &= ~CSTOPB;          //设置停止位 1
        newtio.c_cc[VTIME] = 0; //不使用超时控制
        newtio.c_cc[VMIN] = 0; //不等待字符
        tcflush(fd, TCIFLUSH); //串口数据刷新
        if((tcsetattr(fd, TCSANOW, &newtio)) != 0){
            perror("com set error");
            return -1;
        }
        printf("set done!\n");
    }
    while(1){
        //数据接收
        receive_num = func_send_frame(fd, receive_buff, siezof(receive_buff));
        //读取串口收到的数据
        if(receive_num > 0){
            printf("[nread=%d] ", receive_num);
            printf("%s\n", receive_buff);   //打印接收的数据
        }
        sleep(1);       //数据接收后等待一秒延时
        send_num = func_send_frame(fd, send_buff, 16);
        if(send_num > 0){
            printf("[nwrite=%d] ", send_num);
            printf("%s\n", send_buff);      //打印发送的数据
        }
        sleep(1);
    }
    close(fd);
    exit(0);
}
```



# 三、C++封装

linux_uart.h

```cpp
#ifndef _LINUX_UART_HEAD_H
#define _LINUX_UART_HEAD_H

#include <iostream>
#include <stdint.h>
using namespace std;

class LinuxUart
{
    public:
        LinuxUart(const string &deviceName,int baudRate = 115200);
        ~LinuxUart();
        bool defaultInit(int baudRate);
        int readData(uint8_t * buf,uint32_t size);
        int writeData(const uint8_t * buf,uint32_t size);
        int readFixLenData(uint8_t * buf,uint32_t fixLen);
    private:
        int fd;
};


#endif
```



linux_uart.cpp

```c++
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "linux_uart.h"


LinuxUart::LinuxUart(const string &deviceName, int baudRate)
{
    fd = open(deviceName.c_str(),O_RDWR | O_NOCTTY);
    if(fd < 0){
        fprintf(stderr,"Fail to open %s,err:%s\n",deviceName.c_str(),strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("open %s success \n",deviceName.c_str());

    bool ok = defaultInit(baudRate);
    if(!ok){
        fprintf(stderr,"Fail to init baudRate:%d\n",baudRate);
        exit(EXIT_FAILURE);
    }
}

LinuxUart::~LinuxUart()
{
    close(fd);
}

bool LinuxUart::defaultInit(int baudRate)
{
    int ret;
    int err;
    struct termios tio;
    bzero(&tio, sizeof(tio));
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSIZE;
    // 设置数据位为8bit  
    tio.c_cflag |= CS8;
    // 设置校验位:无奇偶校验位 
    tio.c_cflag &= ~PARENB;
    
    // 设置波特率
    switch (baudRate){
    case 4800:
        cfsetispeed(&tio, B4800);
        cfsetospeed(&tio, B4800);
        break;
    case 9600:
        cfsetispeed(&tio, B9600);
        cfsetospeed(&tio, B9600);
        break;
    case 57600:
        cfsetispeed(&tio, B57600);
        cfsetospeed(&tio, B57600);
        break;
    case 115200:
        cfsetispeed(&tio, B115200);
        cfsetospeed(&tio, B115200);
        break;
    default:
        fprintf(stderr, "The baudrate:%d is not support\n", baudRate);
        return false;
    }
    // 设置停止位为1bit
    tio.c_cflag &= ~CSTOPB;

    // 设置等待时间和最小接收字符 
    tio.c_cc[VTIME] = 50;
    tio.c_cc[VMIN] = 0;

    // 刷新串口:处理未接收字符 
    tcflush(fd, TCIOFLUSH);

    // 设置参数
    err = tcsetattr(fd, TCSANOW, &tio);//TCSANOW的意思就是配置立即生效
    if (err){
        fprintf(stderr, "Fail to tcsetattr,err:%s\n", strerror(errno));
        return false;
    }
    return true;
}

/**
 * @brief 串口读取数据,返回实际读取到的长度
 * 
 * @param buf 
 * @param size 
 * @return int 
 */
int LinuxUart::readData(uint8_t *buf, uint32_t size)
{
    int len;
    len = read(fd,buf,size);
    if(len < 0){
        fprintf(stderr, "Fail to readData,err:%s\n", strerror(errno));
        return -1;
    }

    return len;
}

/**
 * @brief 串口写入数据,返回实际写入的长度
 * 
 * @param buf 
 * @param size 
 * @return int 
 */
int LinuxUart::writeData(const uint8_t *buf, uint32_t size)
{
    int len;
    len = write(fd,buf,size);
    if(len < 0){
        fprintf(stderr, "Fail to writeData,err:%s\n", strerror(errno));
        return -1;
    }

    return len;
}

int LinuxUart::readFixLenData(uint8_t *buf, uint32_t fixLen)
{

    int n;
    int count = 0;
    while(count < fixLen){
        n = read(fd,buf + count,fixLen - count);
        if(n <= 0){
            break;
        }
        count += n;
    }

    return count != fixLen?-1:fixLen;
}

```

该代码没有写 I/O 多路复用，可以重写下接收数据部分。





