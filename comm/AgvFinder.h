/*
 * AgvFinder.h
 *
 *  Created on: 2017-3-17
 *      Author: zxq
 */

#ifndef AGVFINDER_H_
#define AGVFINDER_H_

#include <stdlib.h>
#include <vector>

#include "AgvUdpSocket.h"
#include "../utils/AgvThread.h"

using namespace std;

class FinderReply
{
    public:
        unsigned short  Id;
        string          AgvPid;
        string          IpAddress;

};

class CAgvFinder : public CAgvThread
{
    public:

        enum FinderMode{
            Finder, //发送查询广播，等待接收来子AGV的回应
            Replier //接收到广播后回应AGV的信息
        };

        CAgvFinder(int mode);
        virtual ~CAgvFinder();

        inline void SetAgvPid(const string &pid)    {agvPid  = pid;}
        inline void SetAgvId(const string &id)      {agvId   = atoi(id.c_str());}
        inline void SetLocalIp(const string &ip)    {localIp = ip;}

        int StartService(int port);

        int StopService();

        //For finder
        int SendFindSignal(int times);
        int GetReplyedList(vector<FinderReply> &list);

        //For replier
        int ReplyFindSignal(const string &ip, int times);

    private:

        void Run();

        int onReceiveData(const string &ip, unsigned char *data, int len);

        bool isInFindedList(const string &ip);

        void addFindedList(const string &ip, unsigned short id, const string &pid);

        void clearFindedList();

    private:
        int                 workMode;
        int                 broadcastPort;
        string              agvPid;
        unsigned short      agvId;
        string              localIp;

        CAgvUdpSocket       udpSocket;
        vector<FinderReply*>  agvList;

        unsigned char       sendBuf[64];
        unsigned char       recvBuf[256];
        int                 sendLength;
        int                 recvLength;
        string              replyIp;

        int                 sendTimes;
        int                 maxSendTimes;
        int                 replyTimes;
        int                 maxReplyTimes;
        bool                loopEnable;
};

#endif /* AGVFINDER_H_ */
