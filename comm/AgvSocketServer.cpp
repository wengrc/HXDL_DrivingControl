/**
 * Copyright(c)2006,ZHIJIU All Right Reserved
 * File Name:AgvSocketServer.cpp
 * File description:the class of server
 * Current Version:1.0.0.6
 * Author:Figu Lin
 * Date:4/26/2016
 */


#include "AgvSocketServer.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

CAgvSocketServer::CAgvSocketServer(int port)
{
    serverFd = 0;
    m_port = port;
    enableFlag = false;
}

CAgvSocketServer::~CAgvSocketServer()
{

}

void CAgvSocketServer::ServerRunning(ISocketListener *listener)
{
    if(initSocket() != 0)
    {
        printf("Invoke initSocket failed\n");
        return;
    }

    //fd_set active_fd_set, read_fd_set;
    struct sockaddr_in clientName;

    //Figu:Initialize the set of active sockets.
    FD_ZERO (&active_fd_set);
    FD_SET(serverFd, &active_fd_set);

    unsigned char buffer[1024] = {0};
    int len;
    int i;

    enableFlag = true;

    while (enableFlag)
    {
        read_fd_set = active_fd_set;

        //Figu:block model
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            perror ("select");
            return;
        }


        for (i = 0; i < FD_SETSIZE; ++i)//Figu:++i ???
        {
            //printf("current i = %d\n",i);

            if (FD_ISSET(i,&read_fd_set))//Figu:server socket handle
            {
                if (i == serverFd)
                {

                    int new_client_conn;
                    len = sizeof (clientName);

                    new_client_conn = accept (serverFd,(struct sockaddr *)&clientName,(socklen_t*)&len);

                    if (new_client_conn < 0)
                    {
                        perror ("accept");
                        return;
                    }

                    fprintf (stderr,"%s [%d] is online\n",inet_ntoa(clientName.sin_addr),ntohs(clientName.sin_port));

                    //Figu:
                    if(setKeepAlive(new_client_conn) != 0)
                    {
                        printf("Invoke set_keep_alive failed\n");
                        return;
                    }

                    //Figu:can remove???
                    if(setSendTimeout(new_client_conn,1,500) != 0)
                    {
                        printf("Invoke set_send_timeout failed\n");
                        return;
                    }

                    //Figu:can remove???
                    if(setRecvTimeout(new_client_conn,1,500) != 0)
                    {
                        printf("Invoke set_send_timeout failed\n");
                        return;
                    }

                    FD_SET (new_client_conn, &active_fd_set);

                }
                else //Figu:client socket handle
                {

                    //Figu:Data arriving on an already-connected socket,i eq FD
                    int lengthOfRead = read(i,buffer,sizeof buffer);

                    if( lengthOfRead > 0)
                    {
                        if(2 == lengthOfRead)
                        {

                            if(processClentsKeepAliveData(i,buffer,lengthOfRead) != 0)
                            {
                                printf("Invoke ackAndDataProcess failed\n");
                                //return;
                            }

                        }
                        else if (listener != NULL)
                        {
                            if(listener->OnRecvData(i,buffer,lengthOfRead) != 0)
                            {
                                printf("Invoke callbackfun failed\n");
                                //return;
                            }
                        }
                    }
                    else
                    {
                        if(errno == EAGAIN)
                        {
                            printf("EAGAIN Happened\n");
                        }

                        printf("FD %d is offline\n",i);
                        //Figu:
                        if(-1 == close(i))
                        {
                            perror("close");
                            return;
                        }

                        FD_CLR(i, &active_fd_set);

                        //Figu:remove element from list
                        list<ConnInfo>::iterator pList;

                        for(pList = allConnInfo.begin(); pList != allConnInfo.end(); pList++)
                        {

                            if( pList->fd == i)
                            {
                                printf("remove an element from list\n");
                                allConnInfo.erase(pList);
                                break;
                            }
                        }

                    }


                }

            }

        }



    }


}


int CAgvSocketServer::processClentsKeepAliveData(int fd,unsigned char *dataIn,int len)
{
    unsigned char data[24] = {0};
    memcpy(data,dataIn,len);//Figu:the best way is operating data buffer

    if(write(fd,data,len) !=  len)
    {
        printf("Invoke write failed\n");
    }
    else
    {
    }

    bool isIn = false;

    tempConnInfo.fd = fd;
    tempConnInfo.id = data[0] << 8;
    tempConnInfo.id += data[1];

    list<ConnInfo>::iterator pList;

    for(pList = allConnInfo.begin(); pList != allConnInfo.end(); pList++)
    {

        if( pList->id == tempConnInfo.id)
        {
            if(pList->fd != tempConnInfo.fd)//Figure:offline
            {
                printf("The caller %d is already in the list,remove an element from list\n",tempConnInfo.id);

                //Figure:close fd
                if(-1 == close(pList->fd))
                {
                    perror("close");
                }

                //Figure:
                FD_CLR(pList->fd,&active_fd_set);
                allConnInfo.erase(pList);


                break;
            }
            else
            {
                isIn = true;
            }
        }
    }

    if(!isIn)
    {
        allConnInfo.push_back(tempConnInfo);
    }

    usleep(5000);

    return 0;
}

int CAgvSocketServer::ServerClose(void)
{
    enableFlag = false;
    if(serverFd > 0)
    {
        if(-1 == close(serverFd))
        {
            return -1;
        }
    }

    return 0;
}


int CAgvSocketServer::GetCallerIdBasedOnFd(int fd,int *callerId)
{
    *callerId = -1;

    list<ConnInfo>::iterator pList;

    for(pList = allConnInfo.begin(); pList != allConnInfo.end(); pList++)
    {

        if( pList->fd == fd)
        {
            *callerId = pList->id;
            break;
        }
    }

    if(-1 == *callerId)
    {
        return -1;
    }

    return 0;
}

/**
 * ServerWriteToOneConnection - Send data to a specified client
 * @callerId(in):Caller ID
 * @buffer(in):data
 * @len(in):the length of buffer
 * return:On success,0 is returned
 */
int CAgvSocketServer::ServerWriteToOneConnection(int callerId, const void* buffer, int len)//-1
{

    int fd = -1;

    //Figu:
    list<ConnInfo>::iterator pList;

    for(pList = allConnInfo.begin(); pList != allConnInfo.end(); pList++)
    {

        if( pList->id == callerId)
        {
            fd = pList->fd;
            break;
        }
    }

    if(fd != -1)
    {
        //Figu:loop serval times???
        if(write(fd,buffer,len) != len)
        {
            perror("CAgvSocketServer::ServerWriteToOneConnection: write");
            //Figu:consider remove element and close FD???
            //...

            return -1;
        }
    }
    else
    {
        return -2;
    }



    return 0;
}

int CAgvSocketServer::ServerWriteToAllConnections(const void* buffer, int len)
{
    list<ConnInfo>::iterator pList;

    for(pList = allConnInfo.begin(); pList != allConnInfo.end(); pList++)
    {

        if(write(pList->fd, buffer, len) != len)
        {
            perror("CAgvSocketServer::ServerWriteToAllConnections: write");
            //Figu:consider remove element and close FD???
            //...

            return -1;
        }

    }

    return 0;

}

int CAgvSocketServer::initSocket(void)
{
    //Figu:Create the socket
    serverFd = socket (PF_INET, SOCK_STREAM, 0);

    if (serverFd < 0)
    {
        perror ("init socket error:");
        return -1;
    }

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons (m_port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);

    if (bind (serverFd, (struct sockaddr *) &name, sizeof (name)) < 0)
    {
        perror ("bind");
        return -2;
    }

    if (listen (serverFd, 10) < 0)
    {
        perror ("listen");
        return -3;
    }

    return 0;

}

int CAgvSocketServer::setSendTimeout(int fd, long seconds, long microseconds)
{

    struct timeval timeo = {1, 0};
    timeo.tv_sec = seconds;
    timeo.tv_usec = microseconds;

    socklen_t len = sizeof(timeo);

    if (setsockopt(fd, SOL_SOCKET,SO_SNDTIMEO, (const void*)&timeo, len) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    return 0;

}

int CAgvSocketServer::setRecvTimeout(int fd, long seconds, long microseconds)
{

    struct timeval timeo;
    timeo.tv_sec = seconds;
    timeo.tv_usec = microseconds;

    socklen_t len = sizeof(timeo);

    if (setsockopt(fd, SOL_SOCKET,SO_RCVTIMEO, (const void*)&timeo, len) == -1)
    {
        perror("setsockopt");
        return -2;
    }

    return 0;

}

int CAgvSocketServer::setKeepAlive(int fd)
{

    int keepAlive = 1;

    int keepIdle = 10;
    int keepInterval = 3;
    int keepCout = 2;

    int res; //
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
