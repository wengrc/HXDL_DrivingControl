#include "AgvDownloadBin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <limits.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <errno.h>


CAgvDownLoadBin::CAgvDownLoadBin()
{

}


CAgvDownLoadBin::~CAgvDownLoadBin()
{

}


int CAgvDownLoadBin::readDataToAckCheck(CAgvSocketCan &socket_can,unsigned char *pSendBuf,unsigned char *pReadBuf,int len)
{
    int res = 0;
    unsigned int loop_count = 5000,can_id = 0,len_out = 0,i =0;

    struct can_filter filter[2];
    filter[0].can_id = 0x20;
    filter[0].can_mask = CAN_EFF_MASK;

    filter[1].can_id = 0x21;
    filter[1].can_mask = CAN_EFF_MASK;

    for(unsigned int j=0;j<loop_count;j++)//Figu:non-block model
    {
        memset(pReadBuf,0,sizeof(pReadBuf));

        usleep(1000);//1ms
        i++;

        res = socket_can.Read(filter,2,&can_id,&len_out,pReadBuf);

        printf("res = %d\n",res);
        if( res != 0)
        {
            if(res != -3)
            {
                return -1;
            }
            else
            {
                if(i >= 100)
                {
                    i = 0;
                    if(socket_can.Write(0x11,pSendBuf,len)!= 0)
                    {
                        printf("Invoke SocketWrite failed\n");
                    }
                    else
                    {
                        printf("send 0xFF 0xFF 0x01 ok\n");
                    }
                }
            }
        }
        else
        {
            if(0 == memcmp(pSendBuf,pReadBuf,len_out))
            {
                break;
            }
            else
            {
                return -2;
            }


        }

        if((loop_count-1) == j)
        {
            printf("ACK check failed\n");
            return -3;
        }

    }
    return 0;
}

int CAgvDownLoadBin::SendFileViaCanBus(const char *pFileName,CAgvSocketCan &socket_can, int boardNo)
{
    int iReturn = 0;

    int bin_fd;
    struct stat state;


    bin_fd = open(pFileName,O_RDONLY);

    if(bin_fd < 0)
    {
        perror("CAgvDownLoadBin open");

        iReturn--;
        return iReturn;
    }

    long filelen = (fstat(bin_fd,&state) < 0) ? -1 : state.st_size;

    if( -1 == filelen || filelen < 80)
    {
        printf("Get the lenth of file failed\n");

        if(close(bin_fd) != 0)
        {
            perror("close");
        }

        iReturn--;
        return iReturn;
    }

//##  0x10 0x20
    int totalFrames = filelen/8 + ((filelen%8 > 0) ? 1:0);

    int sendlen = 0;
    unsigned char sendBuf[8] = {0x00};
    unsigned char readBuf[8] = {0X00};

    sendBuf[0] = 0xff;
    sendBuf[1] = 0xff;
    sendBuf[2] = boardNo; //ver

    if(socket_can.Write(0x10,sendBuf,3) != 0)
    {
        printf("Invoke CAgvSocketCan::Write failed\n");

        if(close(bin_fd) != 0)
        {
            perror("close");
        }

        iReturn--;
        return iReturn;
    }

    int res = 0;
    unsigned int loop_count = 20000,can_id = 0,len_out = 0,i = 0;

    struct can_filter filter[2];
    filter[0].can_id   = 0x20;
    filter[0].can_mask = CAN_EFF_MASK;

    filter[1].can_id   = 0x21;
    filter[1].can_mask = CAN_EFF_MASK;


    for(unsigned int j=0;j<loop_count;j++)//Figu:non-block model
    {
        usleep(1000);
        i++;
        res = socket_can.Read(filter,2,&can_id,&len_out,readBuf);//id = 0x20

        if( res != 0)
        {
            if(res != -3)
            {
                return -1;
            }
            else
            {
                if(i == 100)
                {
                    i = 0;
                    if(socket_can.Write(0x10,sendBuf,3)!= 0)
                    {
                        printf("Invoke SocketWrite failed\n");
                    }
                    else
                    {
                        printf("send 0xFF 0xFF 0x01 ok\n");
                    }
                }
            }
        }
        else
        {
            if(0 == memcmp(sendBuf,readBuf,len_out))
            {
                printf("memcmp ok \n");
                break;
            }
            else
            {
                if(i == 100)
                {
                  i = 0;
                  if(socket_can.Write(0x10,sendBuf,3)!= 0)
                  {
                      printf("Invoke SocketWrite failed\n");
                  }
                  else
                  {
                      printf("x send 0xFF 0xFF 0x01 ok\n");
                  }
               }
            }
        }

        if(j == (loop_count-1))
        {
            printf("memcmp fail \n");
            if(close(bin_fd) != 0)
            {
                perror("close");
            }
            return -2;
       }
    }


    //BUG
    memset(readBuf,0,sizeof(readBuf));
    int E_flag = 1;
    while(E_flag != 0)
    {
        usleep(1000);
        socket_can.Read(filter,2,&can_id,&len_out,readBuf);//id = 0x20
        if((readBuf[0] == 0xAA)&&(readBuf[1] == 0xAA)&&(readBuf[2] == boardNo))
        {
            E_flag = 0;
            printf("OK\n");
        }
        else
        {
            printf("Wait.....\n");
        }
    }


//## 0x11 0x21


    for(int i=0;i < totalFrames; i++)
    {
        memset(sendBuf,0,sizeof(sendBuf));
        memset(readBuf,0,sizeof(readBuf));



        if(i == totalFrames-1)
        {
            if( read(bin_fd,sendBuf,filelen-i*8) != filelen-i*8 )
            {
                perror("read");

                if(close(bin_fd) != 0)
                {
                    perror("close");
                }

                iReturn--;
                return iReturn;
            }

            sendlen = filelen-i*8;
        }
        else
        {
            if( read(bin_fd,sendBuf,8) != 8 )
            {
                perror("read");

                if(close(bin_fd) != 0)
                {
                    perror("close");
                }

                iReturn--;
                return iReturn;
            }

            sendlen = 8;
        }


        for(int k=0;k<1;k++)
        {

            if(socket_can.Write(0x11,sendBuf,sendlen) != 0)
            {
                if(0 == k)
                {
                    printf("Invoke SocketWrite failed\n");

                    if(close(bin_fd) != 0)
                    {
                        perror("close");
                    }

                    iReturn--;
                    return iReturn;
                }
            }
            else
            {
                break;
            }
        }


        if(CAgvDownLoadBin::readDataToAckCheck(socket_can,sendBuf,readBuf,sendlen) != 0)//Debug
        {
            if(close(bin_fd) != 0)
            {
                perror("close");
            }

            iReturn--;
            return iReturn;
        }
    }

    if(close(bin_fd) != 0)
    {
        perror("close");
    }


    sendBuf[0] = 0xee;
    sendBuf[1] = 0xee;
    sendBuf[2] = boardNo;
    sendBuf[3] = 0x00;
    sendBuf[4] = 0x00;
    sendBuf[5] = 0x01;
    if(socket_can.Write(0x10,sendBuf,6) != 0)
    {
        printf("Invoke CAgvSocketCan::Write failed\n");

        iReturn--;
        return iReturn;
    }

    printf("Process:100 percent\n");

    return iReturn;
}
