/**
 * Copyright(c)2006,ZHIJIU All Right Reserved
 * File Name:AgvSocketClient.cpp
 * File description:the class of client
 * Current Version:1.0.0.2
 * Author:Figu Lin
 * Date:3/26/2016
 */


#include "AgvSocketClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <error.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <linux/if.h>



CAgvSocketClient::CAgvSocketClient(string ip,unsigned int port)
{
    m_ip = ip;
    m_port = port;
    m_fd = -1;
}

CAgvSocketClient::~CAgvSocketClient(void)
{

}

int CAgvSocketClient::SocketInit(bool isKeepAliveAuto)
{
    struct hostent *host;
    struct in_addr inp;
    struct ifreq ifr;

    m_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd < 0)
    {
        perror("socket");
        return -1;
    }

    //wgzh
    memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, "ppp0", 6);
    setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));

    //note
    host = gethostbyname(m_ip.c_str());

    if(NULL == host)
    {
        perror("gethostbyname");
        return -2;
    }

    if(inet_aton(inet_ntoa(*((struct in_addr *)host->h_addr)), &inp) != 1)
    {
        perror("inet_ntoa");
        return -3;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof servaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_port);
    servaddr.sin_addr = inp;

    /*m_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof servaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_port);
    servaddr.sin_addr.s_addr = inet_addr(m_ip.c_str());*/

    if (connect(m_fd,(struct sockaddr*) &servaddr, sizeof(struct sockaddr)) < 0)
    {
        perror("connect");
        return -2;
    }

    if(isKeepAliveAuto)
    {
        if(setKeepAlive(m_fd) !=0 )
        {
            return -3;
        }
    }
/*
    if(setRecvTimeout(2,0) != 0)
    {
        return -4;
    }

    if(setSendTimeout(2,0) != 0)
    {
        return -5;
    }
*/

    return 0;
}


int CAgvSocketClient::SocketWrite(const void *buf, size_t len)
{
    if (m_fd <= 0)
    {
        return -1;
    }
    if(write(m_fd,buf,len) != (int)len)
    {
        return -1;
    }

    return 0;
}


int CAgvSocketClient::SocketRead(void *buf, size_t count, ssize_t *countRead)
{
    if (m_fd <= 0)
    {
        return -1;
    }
    *countRead = 0;
    ssize_t res = read(m_fd,buf,count);

    if (res <= 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return -3;
        }
        else
        {
            perror("read");
            return -1;
        }
    }
    else
    {
        *countRead = res;
    }

    return 0;
}

int CAgvSocketClient::SocketClose(void)
{
    if(m_fd > 0)
    {
        if(close(m_fd) != 0)
        {
            perror("close");
            return -1;
        }
        m_fd = 0;
    }

    return 0;
}


int CAgvSocketClient::setSendTimeout(long seconds,long microseconds)
{
    if (m_fd <= 0)
    {
        return -1;
    }
    struct timeval timeo = {1, 0};
    timeo.tv_sec = seconds;
    timeo.tv_usec = microseconds;

    socklen_t len = sizeof(timeo);

    if (-1 == setsockopt(m_fd, SOL_SOCKET,SO_SNDTIMEO, (const void*)&timeo, len))
    {
        perror("setsockopt");
        return -1;
    }

    return 0;

}

int CAgvSocketClient::setRecvTimeout(long seconds,long microseconds)
{
    if (m_fd <= 0)
    {
        return -1;
    }
    struct timeval timeo;
    timeo.tv_sec = seconds;
    timeo.tv_usec = microseconds;

    socklen_t len = sizeof(timeo);

    if (setsockopt(m_fd, SOL_SOCKET,SO_RCVTIMEO, (const void*)&timeo, len) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    return 0;

}


int CAgvSocketClient::setKeepAlive(int fd)
{

    int keepAlive = 1;
    int keepIdle = 5;
    int keepInterval = 3;
    int keepCout = 2;

    int res;
    res=setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&keepAlive,sizeof(int));
    if(-1 == res)
    {
        perror ("setsockopt");
        return -1;
    }

    res=setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&keepIdle,sizeof(int));
    if(res<0)
    {
        perror ("setsockopt");
        return -2;
    }

    res=setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&keepInterval,sizeof(int));
    if(res<0)
    {
        perror ("setsockopt");
        return -3;
    }

    res=setsockopt(fd,IPPROTO_TCP,TCP_KEEPCNT,&keepCout,sizeof(int));
    if(res<0)
    {
        perror ("setsockopt");
        return -4;
    }


    return 0;
}
