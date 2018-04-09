/**
 * Copyright(c)2006,ZHIJIU All Right Reserved
 * File Name:AgvSocketClient.h
 * File description:the class of client
 * Current Version:1.0.0.2
 * Author:Figu Lin
 * Date:3/26/2016
 */


#ifndef AGVSOCKETCLIENT_H
#define AGVSOCKETCLIENT_H

#include <iostream>
using namespace std;

class CAgvSocketClient
{
    public:
        CAgvSocketClient(string ip, unsigned int port);
        ~CAgvSocketClient(void);

    public:
        int SocketInit(bool isKeepAliveAuto);
        int SocketWrite(const void *buf, size_t len);
        int SocketRead(void *buf, size_t count, ssize_t *countRead);
        int SocketClose(void);
        inline int GetFd() {return m_fd;}
    private:
        int setKeepAlive(int fd);
        int setSendTimeout(long seconds, long microseconds);
        int setRecvTimeout(long seconds, long microseconds);

        string m_ip;
        int m_port;
        int m_fd;
};

#endif // AGVSOCKETCLIENT_H
