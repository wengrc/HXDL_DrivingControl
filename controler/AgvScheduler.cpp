/*
 * AgvScheduler.cpp
 *
 *  Created on: 2016-9-22
 *      Author: zxq
 */

#include "AgvScheduler.h"
#include "../AgvPublic.h"
#include <time.h>
#include <stdio.h>
#define LOGTAG      "AgvScheduler"

//#define COMDEBUG

CAgvScheduler::CAgvScheduler() : CAgvThread("SchedulerThread")
{
    m_ip = "192.168.1.119";
    m_port = 8622;

    m_loopEnable = false;
    m_isConnected = false;

    agvId = 1001;
    cmdTimeOut = 0;
    cmdTimer  = 0;
    lastCmd = 0;

    heartBeatTimeout = 0;
    heartBeatEnable = false;
    client = NULL;
    pthread_mutex_init(&socketLocker, NULL);

    sem_init(&cmdSem, 0, 0);

}

CAgvScheduler::~CAgvScheduler()
{
    StopSchedule();
    sem_destroy(&cmdSem);
    pthread_mutex_destroy(&socketLocker);
}


int CAgvScheduler::StartSchedule()
{
    m_isConnected = false;
    m_loopEnable = true;
    return Start(true);
}

int CAgvScheduler::ConnectToServer()
{
    if (m_isConnected)
        return 0;

    if (client == NULL)
    {
        client = new CAgvSocketClient(m_ip, m_port);
    }

    if (0 != client->SocketInit(false))
    {
        return -1;
    }
    return 0;

}


int CAgvScheduler::Disconnect()
{
    if (m_isConnected)
    {
        heartBeatTimeout = 0;
        m_isConnected = false;
        if (client != NULL)
        {
            client->SocketClose();
            delete client;
            client = NULL;
            LogInfo(LOGTAG, "Disconnected from Scheduler Server\n");
        }

        SendImportantEvent(evSchedulerOffline, 0, NULL);
    }
    return 0;
}

int CAgvScheduler::StopSchedule()
{
    if (m_loopEnable)
    {
        m_loopEnable = false;
        sem_post(&cmdSem);
        Disconnect();
        LogInfo(LOGTAG, "Scheduler stoped!\n");
        Stop();
    }

    return 0;
}

void CAgvScheduler::Run()
{
    LogInfo(LOGTAG, "Start enter AgvScheduler communication loop...\n");
    int  readCnt = 0, ret = -1, socket = 0;
    fd_set readSet;
    struct timeval timeout;
    curStatus.id = agvId;
    heartBeatEnable = true;

    while(m_loopEnable)
    {
        if (!m_isConnected)
        {
            LogInfo(LOGTAG, "Start to connect Scheduler Server %s:%d...\n", m_ip.c_str(), m_port);
            if (ConnectToServer() == 0)
            {
                LogInfo(LOGTAG, "Connect to Scheduler Server ok!\n");
                m_isConnected = true;
                socket = client->GetFd();
                SendEvent(evSchedulerOnline, 0, NULL);
            }
            else
            {
                LogInfo(LOGTAG, "Connect Scheduler Server failed! Retry again after 5 secs...\n");
                int waittime = 50;
                while(m_loopEnable && waittime--)
                {
                    usleep(1000*100);
                }
                SendImportantEvent(evSchedulerOffline, 0, NULL);
                continue;
            }
        }


        FD_ZERO(&readSet);
        FD_SET(socket, &readSet);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200 * 1000;
        ret = select(socket + 1, &readSet, NULL, NULL, &timeout);
        if (ret < 0)
        {
            LogError(LOGTAG, "Select socket error:%s\n", strerror(errno));
            if (errno == EINTR)
            {
                continue;
            }
            Disconnect();
            continue;
        }
        else if (ret == 0)
        {
            //SendHeartBeat();
            if (heartBeatEnable && heartBeatTimeout++ > 50)
            {
                heartBeatTimeout = 0;
                LogError(LOGTAG, "Waiting for Scheduler Server HeartBeat response time out! ReConnect to server now...\n");
                Disconnect();
            }
            else
            {
                ReportStatusToServer();
            }
            continue;
        }


        if (!FD_ISSET(socket, &readSet))
        {
            LogError(LOGTAG, "No this socket data!\n");
            continue;
        }

        readCnt = 0;
        pthread_mutex_lock(&socketLocker);
        ret = client->SocketRead(receiveBuf, sizeof(receiveBuf), &readCnt);
        pthread_mutex_unlock(&socketLocker);

        if ( 0 == ret || -3 == ret)
        {
            /*printf(">>>>Read Data[%d]:", readCnt);
            for (int i = 0; i < readCnt; i++)
            {
                printf("%02X ", receiveBuf[i]);
            }
            printf("\n");
            */
            if (readCnt < 10)
            {
                continue;
            }

            ParseReceiveData(receiveBuf, readCnt);
        }
        else
        {
            Disconnect();
            continue;
        }
    }
    LogInfo(LOGTAG, "AgvScheduler Loop finished!\n");
}

int CAgvScheduler::SendCmdAndReceive(int cmd, unsigned char* data, int len, ServerAck *ack)
{
    if (!m_isConnected)
    {
        return -1;
    }
    int ret = -1;
    unsigned char buf[2048] = {0};

    CAgvSchProtocol protocol(cmd, data, len);
    int length = protocol.PackData(buf);
    pthread_mutex_lock(&socketLocker);
#ifdef COMDEBUG
    {
        printf("Send Cmd:%04X data len=%d to Scheduler Server...\n", cmd, len);
        printf("<<<<<Send Data[%d]:", length);
        for (int i = 0; i < length; i++)
            printf("%02X ", buf[i]);
        printf("\n");
    }
#endif
    if (0 != client->SocketWrite(buf, length))
    {
        LogError(LOGTAG, "Send cmd %04X failed!:%s\n", cmd, strerror(errno));
        pthread_mutex_unlock(&socketLocker);
        return -2;
    }
    pthread_mutex_unlock(&socketLocker);
    if (ack == NULL)
    {
        return 0;
    }
#ifdef  COMDEBUG
    {
        printf("Waitting for Cmd:%04X Scheduler Server response...\n", cmd);
    }
#endif

    lastCmd = cmd;

    timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 5;

    ret = sem_timedwait(&cmdSem, &timeout);
    if (ret == 0)
    {
        *ack = lastCmdAck;
        ret = 0;
    }
    else
    {
        printf("Waitting for server response failed!:%s\n", strerror(errno));
        ret = 1;
    }


    return ret;
}


int CAgvScheduler::ParseReceiveData(const unsigned char* data, int len)
{
    CAgvSchProtocol protocol;
    int left = len, cnt = 0;

    do {
        int plen = protocol.UnPackData(data + len - left, left, &left);
        if (plen < 0)
        {
            LogError(LOGTAG, "Parse data failed!\n");
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
        switch (protocol.cmd)
        {
            case CAgvSchProtocol::AgvHeartBeat:
                printf("Get Agv Heart Beat response\n");
                break;

            case CAgvSchProtocol::ServerSendPath:
            {
                pathList.unPackData(protocol.data, protocol.dataLength);
                LogInfo(LOGTAG, "Get path count:%d - %d\n", pathList.stationCount, pathList.stationList.size());
//                if (pathList.stationCount && pathList.stationList.size())
                {
                    SendImportantEvent(evGetNewPath, pathList.stationCount, &pathList.stationList);
                }
                SendAckToServer(protocol.cmd, 0, NULL, 0);
                break;
            }
            case CAgvSchProtocol::ServerSendAction:
            {
                DoAction action;
                action.unPackData(protocol.data, protocol.dataLength);
                printf("CAgvSchProtocol::ServerSendAction:%d\n",action.action);
                switch (action.action)
                {
                    case 0x00:  //ÔËÐÐ
                        SendEvent(evKeyStart, 0 , NULL);
                        SendAckToServer(protocol.cmd, 0,&action.action, 2);
                        break;
                    case 0x01:  //Í£Ö¹
                        SendUrgentEvent(evKeyStop, 0, NULL);
                        SendAckToServer(protocol.cmd, 0,&action.action, 2);
                        break;
                    default:
                        break;
                }
                break;
            }
            case CAgvSchProtocol::ServerSetSpeed:
            {
                printf("CAgvSchProtocol::ServerSetSpeed\n");
                SetSpeed ss;
                ss.unPackData(protocol.data, protocol.dataLength);
                SendEvent(evSetSpeed, ss.speed, NULL);

                SendAckToServer(protocol.cmd, 0, NULL, 0);
                break;
            }
            case CAgvSchProtocol::ServerResponseAck:
            {
                //printf("CAgvSchProtocol::ServerResponseAck\n");
                heartBeatTimeout = 0;  
                ServerAck ack;
                ack.unPackData(protocol.data, protocol.dataLength);
                if (ack.cmd == lastCmd)
                {
                    lastCmdAck = ack;
                    sem_post(&cmdSem);
                }
                break;
            }
            case CAgvSchProtocol::ServerChargerTurnOn:
            {
                SendEvent(evTurnOnCharge, 1, NULL);
                SendAckToServer(protocol.cmd, 0, NULL, 0);
                break;
            }

            case CAgvSchProtocol::ServerChargerTurnOff:
            {
                SendUrgentEvent(evTurnOffCharge, 1, NULL);
                SendAckToServer(protocol.cmd, 0, NULL, 0);
                break;
            }
            case CAgvSchProtocol::ServerRunTest:
            {
                SendAckToServer(protocol.cmd, 0, NULL, 0);

                int i = 2 + 19;
                TestStationInfo *info = new TestStationInfo();
                SetTestPath::GetStationInfo(protocol.data + i, protocol.dataLength - i, info);
                LogInfo(LOGTAG, "X:%d Y:%d Direct:%d radius:%d angle:%d mode:%d action:%d liftHeight:%d",
                                info->xPos, info->yPos, info->direction,
                                info->turnRadius, info->turnAngle, info->turnMode,
                                info->action, info->liftHeight);
                break;
            }
            case CAgvSchProtocol::ServerSynchronizeTime:
            {
                SendAckToServer(protocol.cmd, 0, NULL, 0);
                cagvSchedulerSystemTimeSet(protocol.data,protocol.dataLength);
            }
            break;

            case CAgvSchProtocol::ServerSetBarrierDistance:
            {
                int distance = protocol.data[0];
                SendAckToServer(protocol.cmd, 0, NULL, 0);
                printf("----> distance = %d <-----\n",distance);
                SendUrgentEvent(evSetBarrierDistance, distance, NULL);
            }
            break;

            case CAgvSchProtocol::ServerSetAgvSleep:
            {
                SendUrgentEvent(evAgvSleep, 1, NULL);
                printf("=====evAgvSleep====\n");
                SendAckToServer(protocol.cmd, 0, NULL, 0);
            }
            break;

            case CAgvSchProtocol::ServerSetAgvWakeUp:
            {
                SendUrgentEvent(evAgvWakeUp, 1, NULL);
                printf("=====evAgvWakeUp====\n");
                SendAckToServer(protocol.cmd, 0, NULL, 0);
            }
            break;

            default:
                LogWarn(LOGTAG, "Get Unknown cmd:%04X datalen:%d", protocol.cmd, protocol.dataLength);
                break;
        }
    }while(left > 0);

    return cnt;
}

//#include <time.h>

void CAgvScheduler::cagvSchedulerSystemTimeSet(const unsigned char *data, int len)
{
    SystemTime systemTime;
    systemTime.unPackData(data, len);
    struct tm tmSystem;
    tmSystem.tm_year = systemTime.m_year - 1900;
    tmSystem.tm_mon  = systemTime.m_mon - 1;
    tmSystem.tm_mday = systemTime.m_mday;
    tmSystem.tm_hour = systemTime.m_hour;
    tmSystem.tm_min  = systemTime.m_min;
    tmSystem.tm_sec  = systemTime.m_sec;
    tmSystem.tm_wday = 1;//systemTime.m_wday;
    tmSystem.tm_yday = 2;//systemTime.m_yday;
    tmSystem.tm_isdst = -1;//systemTime.m_isDst;
    LogInfo("CAgvScheduler::cagvSchedulerSystemTimeSet",
            "%d-%d-%d %d:%d:%d wd:%d yd:%d isdst:%d\n", tmSystem.tm_year,
            tmSystem.tm_mon, tmSystem.tm_mday, tmSystem.tm_hour,
            tmSystem.tm_min, tmSystem.tm_sec, tmSystem.tm_wday,
            tmSystem.tm_yday, tmSystem.tm_isdst);

    time_t timeOfDay = mktime(&tmSystem);
    if (timeOfDay == -1)
    {
        LogError("[MAKE SYSTEM TIME]",
                 "***>Make system time fault with error %s\n",
        strerror(errno));
        return;
    }
    else
    {
        struct timeval timeVal;
        timeVal.tv_sec = timeOfDay;
        timeVal.tv_usec = 0;
        if (-1 == settimeofday(&timeVal, NULL))
        {
            LogError("[SET SYSTEM TIME]",
                     "***>Set system time fault with error %s\n",
                     strerror(errno));
            return;
        }
    }
}

int CAgvScheduler::ReportStatusToServer(ReportStatus &status)
{

    int len = status.packData(sendBuf, sizeof(sendBuf));
    int ret = SendCmdAndReceive(CAgvSchProtocol::AgvReportStatus, sendBuf, len, NULL);

    return ret;
}

int CAgvScheduler::ReportStatusToServer(int status, int power, int speed,  int xpos, int ypos, int angle, int last)
{
    ReportStatus rs;

    rs.id = agvId;
    rs.status = status;
    rs.powerLevel = power;
    rs.speed = speed;
    rs.xPos = xpos;
    rs.yPos = ypos;
    rs.carAngle = angle;
    rs.lastPoint = last;

    return ReportStatusToServer(rs);

}

int CAgvScheduler::ReportStatusToServer()
{
    return ReportStatusToServer(curStatus);
}

int CAgvScheduler::ReportTaskStatus(int id, int status)
{
    ReportActionStatus as;

    as.id = agvId;
    as.taskId = id;
    as.status = status;

    int len = as.packData(sendBuf, sizeof(sendBuf));
    int ret = SendCmdAndReceive(CAgvSchProtocol::AgvReportTaskStatus, sendBuf, len, NULL);
    LogInfo(LOGTAG, "Report station[%d] action status:%d to server:%d", id, status, ret);
    return ret;

}

int CAgvScheduler::RequestNewPath(int xpos, int ypos, int lastStation)
{
    RequestPath rt;
    rt.id = agvId;
    rt.xPos = xpos;
    rt.yPos = ypos;
    rt.lastPoint = lastStation;

    int len = rt.packData(sendBuf, sizeof(sendBuf));
//    ServerAck ack;
    int ret = SendCmdAndReceive(CAgvSchProtocol::AgvRequestStation, sendBuf, len, /*&ack*/NULL);
    if (ret == 0)
    {
        return /*ack.result*/0;
    }

    return ret;

}

int CAgvScheduler::SendHeartBeat()
{
    HeartBeat hb;
    hb.id = agvId;

    int len = hb.packData(sendBuf, sizeof(sendBuf));
    int ret = SendCmdAndReceive(CAgvSchProtocol::AgvHeartBeat, sendBuf, len, NULL);

    return ret;

}

int CAgvScheduler::SendAckToServer(int cmd, int result, const unsigned char* data, int dataLen)
{
    AckToServer ack;
    ack.id = agvId;
    ack.cmd = cmd;
    ack.result = result;
    ack.dataLength = dataLen;
    ack.data = (unsigned char*)data;

    int len = ack.packData(sendBuf, sizeof(sendBuf));
    int ret = SendCmdAndReceive(CAgvSchProtocol::AgvCmdAck, sendBuf, len, NULL);
    return ret;
}

int CAgvScheduler::RequestSystemTimeSynchronization()
{
    printf("-------> RequestSystemTimeSynchronization\n");
    SystemTime systemTime;

    systemTime.m_agvID = agvId;
    int len = systemTime.packData(sendBuf, sizeof(sendBuf));
    int ret = SendCmdAndReceive(CAgvSchProtocol::AgvRequestSystemTimeSyn, sendBuf, len, NULL);
    return ret;
}
