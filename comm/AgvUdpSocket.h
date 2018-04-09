/*
 * AgvUdpSocket.h
 *
 *  Created on: 2017-3-16
 *      Author: zxq
 */

#ifndef AGVUDPCLIENT_H_
#define AGVUDPCLIENT_H_

#include <string>
#include <netinet/in.h>

using namespace std;

class CAgvUdpSocket
{
    public:
        CAgvUdpSocket();
        virtual ~CAgvUdpSocket();

        int SocketInit(bool broadcast);
        int SocketBind(int port);
        int SocketWrite(const void *buf, size_t len, string ip, int port);
        int SocketRead(void *buf, size_t count, string &ip, int port);
        int SocketClose(void);

        inline int GetSocketFd() {return socketFd;}

    private:
        int     socketFd;
        bool    isBroadcast;
        bool    isServer;
};

#endif /* AGVUDPCLIENT_H_ */
