#ifndef AGVSOCKETSERVER_H
#define AGVSOCKETSERVER_H

#include <netinet/tcp.h>
#include <list>
using namespace std;

typedef struct conninfo
{
    //Figu:id of caller
    int id;
    int fd;
}ConnInfo;

class ISocketListener
{
    public:
        ISocketListener() {}
        virtual ~ISocketListener() {}

        virtual int OnRecvData(int fd, unsigned char *data, int len) = 0;
};

class CAgvSocketServer
{
    public:
        CAgvSocketServer(int port);
        ~CAgvSocketServer();

    public:
        void ServerRunning(ISocketListener *listener);//Figure:callback function
        int ServerWriteToOneConnection(int callerId, const void* buffer, int len);
        int ServerWriteToAllConnections(const void* buffer, int len);//
        int ServerClose(void);
        int GetCallerIdBasedOnFd(int fd,int *callerId);

    private:
        int initSocket(void);
        int setSendTimeout(int fd, long seconds, long microseconds);
        int setRecvTimeout(int  fd,long seconds, long microseconds);
        int setKeepAlive(int fd);
        int processClentsKeepAliveData(int fd,unsigned char *dataIn,int len);

        int m_port;
        int serverFd;

        bool   enableFlag;
        fd_set active_fd_set;
        fd_set read_fd_set;

        ConnInfo tempConnInfo;
        list<ConnInfo> allConnInfo;

};

#endif // AGVSOCKETSERVER_H
