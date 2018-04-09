/*
 * AgvUdpSocket.cpp
 *
 *  Created on: 2017-3-16
 *      Author: zxq
 */

#include "AgvUdpSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

CAgvUdpSocket::CAgvUdpSocket()
{
    socketFd = 0;
    isBroadcast = false;
    isServer = false;
}

CAgvUdpSocket::~CAgvUdpSocket()
{
    if (socketFd > 0)
    {
        SocketClose();
    }
}

int CAgvUdpSocket::SocketInit(bool broadcast)
{
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0)
    {
        perror("Init udp socket error:");
        return -1;
    }
    isBroadcast = broadcast;
    isServer = false;
    if (broadcast)
    {
        const int opt = 1;
        //设置该套接字为广播类型，
        int nb = 0;
        nb = setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
        if(nb == -1)
        {
            perror("Set udp socket boradcast mode failed:");
            return -2;
        }
    }

    return 0;
}

int CAgvUdpSocket::SocketBind(int port)
{
    if (socketFd <= 0)
    {
        return -1;
    }

    struct sockaddr_in serverAddr;

    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if(bind(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind udp socket failed:");
        SocketClose();
        return -2;
    }

    isServer = true;
    return 0;
}

int CAgvUdpSocket::SocketWrite(const void* buf, size_t len, string ip, int port)
{
    if (socketFd <= 0)
    {
        return -1;
    }

    struct sockaddr_in toAddr;

    bzero(&toAddr, sizeof(toAddr));
    toAddr.sin_family = AF_INET;
    toAddr.sin_port = htons(port);
//    if (isBroadcast)
//    {
//        toAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
//    }
//    else
    {
        inet_aton(ip.c_str(), (struct in_addr *)&toAddr.sin_addr);
    }

    int wlen = sendto(socketFd, buf, len, 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
    if (wlen < 0)
    {
        perror("Send udp data failed:");
        return -1;
    }
    return wlen;
}

int CAgvUdpSocket::SocketRead(void* buf, size_t count, string &ip, int port)
{
    if (socketFd <= 0)
    {
        return -1;
    }

    struct sockaddr_in fromAddr;
    unsigned int addlen = sizeof(struct sockaddr_in);

    bzero(&fromAddr, sizeof(fromAddr));
    fromAddr.sin_family = AF_INET;
    fromAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    fromAddr.sin_port = htons(port);

    int rlen = recvfrom(socketFd, buf, count, 0, (struct sockaddr *)&fromAddr, &addlen);
    if (rlen < 0)
    {
        perror("Recv udp data failed:");
        return -1;
    }

    ip = inet_ntoa(fromAddr.sin_addr);

    return rlen;
}

int CAgvUdpSocket::SocketClose(void)
{
    if (socketFd > 0)
    {
        close(socketFd);
        socketFd = 0;
    }
    return 0;
}
