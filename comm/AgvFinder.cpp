/*
 * AgvFinder.cpp
 *
 *  Created on: 2017-3-17
 *      Author: zxq
 */

#include "AgvFinder.h"

#include "../AgvPublic.h"
#include "AgvSchProtocol.h"

#define LOGTAG  "AgvFinder"

#define CMD_FIND    0x03FF
#define CMD_REPLY   0X01FF

CAgvFinder::CAgvFinder(int mode):CAgvThread("FinderService")
{
    workMode = mode;
    agvId = 0;
    sendTimes = 0;
    maxSendTimes = 30;
    replyTimes = 0;
    maxReplyTimes = 10;
    loopEnable = false;
    sendLength = 0;
    recvLength = 0;
    broadcastPort = 8200;
}

CAgvFinder::~CAgvFinder()
{
    StopService();
    clearFindedList();
}

int CAgvFinder::StartService(int port)
{
    if (udpSocket.SocketInit(true) != 0)
    {
        LogError(LOGTAG, "Start Finder service failed!");
        return -1;
    }

    broadcastPort = port;

    if (workMode == Finder)
    {
        agvList.clear();
        sendTimes = 0;
        maxSendTimes = 30;
    }
    else    //Replier
    {
        if (udpSocket.SocketBind(port) != 0)
        {
            LogError(LOGTAG, "Start finder service failed!");
            udpSocket.SocketClose();
            return -2;
        }
        replyTimes = 0;
        maxReplyTimes = 10;
    }

    loopEnable = true;
    LogInfo(LOGTAG, "Preparing start Finder service mode:%d", workMode);
    return Start(true);
}

int CAgvFinder::StopService()
{
    if (loopEnable)
    {
        loopEnable = false;
    }
    Stop();

    udpSocket.SocketClose();
    LogInfo(LOGTAG, "Stop finder service...");
    return 0;
}

int CAgvFinder::SendFindSignal(int times)
{
    unsigned char data[8] = {0};
    int i = 0;

    data[i++] = agvId >> 8;
    data[i++] = agvId & 0xFF;


    clearFindedList();

    CAgvSchProtocol protocol(CMD_FIND, data, i);

    protocol.type = 2;

    sendLength = protocol.PackData(sendBuf);
    sendTimes = times * 10;
    maxSendTimes = times * 10;

    return 0;
}

int CAgvFinder::ReplyFindSignal(const string &ip, int times)
{
    unsigned char data[64] = {0};
    int i = 0;

    data[i++] = agvId >> 8;
    data[i++] = agvId & 0xFF;

    strncpy((char*)data + i, agvPid.c_str(), 15);
    i += 16;

    strncpy((char*)data + i, localIp.c_str(), 15);
    i += 16;

    CAgvSchProtocol protocol(CMD_REPLY, data, i);
    protocol.type = 2;

    sendLength = protocol.PackData(sendBuf);
    replyTimes = times * 10;
    maxReplyTimes = times * 10;
    replyIp = ip;

    return 0;
}

void CAgvFinder::Run()
{
    int socketFd = udpSocket.GetSocketFd();

    if (socketFd <= 0)
    {
        LogError(LOGTAG, "Finder socket is not valid!");
        return;
    }
    LogInfo(LOGTAG, "Enter %s's runing loop...", (workMode == Finder)? "Finer":"Replier");

    fd_set readSet;
    struct timeval timeout;
    int ret = -1;

    while(loopEnable)
    {
        if (socketFd <= 0)
        {
            break;
        }

        FD_ZERO(&readSet);
        FD_SET(socketFd, &readSet);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100 * 1000;
        ret = select(socketFd + 1, &readSet, NULL, NULL, &timeout);

        if (ret < 0)
        {
            LogError(LOGTAG, "Select socket error:%s\n", strerror(errno));
            if (errno == EINTR)
            {
                continue;
            }

            break;
        }
        else if (ret == 0)  //未接收到数据时，可以发送数据
        {
            if (sendTimes > 0  && sendLength > 0)
            {
                sendTimes--;
                if (sendTimes % 10 == 0)    //1秒发送一次广播
                {
                    if (udpSocket.SocketWrite(sendBuf, sendLength, "255.255.255.255", broadcastPort) < 0)
                    {
                        LogError(LOGTAG, "Send Finder signal failed!");
                    }
                    else
                    {
                        DEBUG_INFO(LOGTAG, "Send a finder signal....");
                    }
                }
            }

            if (replyTimes > 0 && sendLength > 0)
            {
                replyTimes--;
                if (replyTimes % 10 == 0)   //1秒发送一次回应
                {
                    if (udpSocket.SocketWrite(sendBuf, sendLength, replyIp, broadcastPort) < 0)
                    {
                        LogError(LOGTAG, "Send reply to %s failed!", replyIp.c_str());
                    }
                    else
                    {
                        DEBUG_INFO(LOGTAG, "Send reply to %s ", replyIp.c_str());
                    }
                }
            }
        }
        else    //有回应的数据
        {
            string clientIp = "";
            recvLength = udpSocket.SocketRead(recvBuf, sizeof(recvBuf), clientIp, broadcastPort);
            if (recvLength > 0)
            {
                onReceiveData(clientIp, recvBuf, recvLength);
            }
            else if (recvLength < 0)
            {
                LogError(LOGTAG, "Read udp socket data failed!");
            }
        }


        if (workMode == Finder)
        {

        }
        else
        {

        }
    }

    LogInfo(LOGTAG, "Exit Finder Service loop...");
}

int CAgvFinder::GetReplyedList(vector<FinderReply>& list)
{
    list.clear();
    for (unsigned int i = 0; i < agvList.size(); i++)
    {
        list.push_back(*agvList[i]);
    }

    return agvList.size();
}

int CAgvFinder::onReceiveData(const string &ip, unsigned char *data, int len)
{
    CAgvSchProtocol protocol;
    int left = len, cnt = 0;

    do {
        int plen = protocol.UnPackData(data + len - left, left, &left);
        if (plen < 0)
        {
            LogError(LOGTAG, "Parse data from %s failed!\n", ip.c_str());
            char buf[2048] = {0};
            int i = 0;
            i += sprintf(buf + i, "Error Data[%d]:",len);
            for (int j = 0; j < len; j++)
            {
                i += sprintf(buf + i, "%02X ",data[j]);
            }
            LogError(LOGTAG, "%s",buf);
            return -1;
        }
        cnt++;
#ifdef  COMDEBUG
        DEBUG_INFO(LOGTAG, "Parse %d protocol Successfully! cmd:%04X\n", cnt, protocol.cmd);
#endif


        if (workMode == Finder) //查询者，接收回应
        {
            if (protocol.cmd == CMD_REPLY)  //收到回应
            {
                if (protocol.dataLength < 18)
                {
                    LogError(LOGTAG, "Get wrong reply data length!");
                }
                else
                {
                    unsigned short id = 0;
                    string pid = "";
                    char str[32] = {0};
                    int i = 0;

                    id = CAgvUtils::Buffer2Bin(protocol.data + i, 2);
                    i += 2;

                    strncpy(str, (char*)protocol.data + i, 15);
                    i += 16;
                    if (protocol.length == 18)  //旧协议，只有IP地址
                    {
//                        ip = str;
                    }
                    else    //新协议，返回PID和IP
                    {
                        pid = str;

                        strncpy(str, (char*)protocol.data + i, 15);
                        i += 16;
//                        ip = str;
                    }
                    LogInfo(LOGTAG, "Get a Agv's reply: Ip:%s, Id:%d Pid:%s", ip.c_str(), id, pid.c_str());
                    addFindedList(ip, id, pid);
                }
            }
            else
            {
                LogWarn(LOGTAG, "Get unknown data! Cmd:%04X Ip:%s", protocol.cmd, ip.c_str());
            }
        }
        else    //回应者，接收广播
        {
            if (protocol.cmd == CMD_FIND)   //收到查询广播
            {
                if (protocol.dataLength >= 2)
                {
                    unsigned short callerId = CAgvUtils::Buffer2Bin(protocol.data, 2);
                    LogInfo(LOGTAG, "Get %d's broadcast...", callerId);
                }
                ReplyFindSignal(ip, 3);
            }
        }

    }while(left > 0);

    return cnt;
}

bool CAgvFinder::isInFindedList(const string& ip)
{
    for (unsigned int i = 0; i < agvList.size(); i++)
    {
        if (agvList[i]->IpAddress.compare(ip) == 0)
        {
            return true;
        }
    }
    return false;
}

void CAgvFinder::addFindedList(const string& ip, unsigned short id, const string& pid)
{
    if (isInFindedList(ip))
    {
        return;
    }

    FinderReply *data = new FinderReply();
    data->Id = id;
    data->IpAddress = ip;
    data->AgvPid = pid;

    agvList.push_back(data);
}

void CAgvFinder::clearFindedList()
{
    FinderReply *data = NULL;
    for (vector<FinderReply*>::iterator it = agvList.begin(); it != agvList.end();)
    {
        data = *it;
        delete data;
        it = agvList.erase(it);
    }
}
