/*
 * AgvScheduler.h
 *
 *  Created on: 2016-9-22
 *      Author: zxq
 */

#ifndef AGVSCHEDULER_H_
#define AGVSCHEDULER_H_

#include <string>
#include <semaphore.h>

#include "../comm/AgvSocketClient.h"
#include "../comm/AgvSchProtocol.h"
#include "../utils/AgvThread.h"

using namespace std;

class CAgvScheduler : public CAgvThread
{

    public:

        enum Status{
            Running     = 0x00,    //0x00 运行；
            Stopped     = 0x01,    //0x01 停止；
            BreakDown   = 0x02,    //0x02 故障；

            Obstacle    = 0x03,    //0x03 前方障碍；
            Derailed    = 0x04,    //0x04 出轨；

            MainTainMode = 0x06,
            ManualMode  = 0x08,    //0x08 手动模式
            LowPower    = 0x09,    //0x09 电量不足；
            Starting    = 0x0A,    //0x0A 启动；
            Idle        = 0x0B,    //0x11 空闲
            Await       = 0x0C,
        };

        CAgvScheduler();
        virtual ~CAgvScheduler();

        inline void SetServerIp(string ip)  {m_ip = ip;}
        inline void SetServerPort(int port) {m_port = port;}


        void Run();

        int StartSchedule();

        int StopSchedule();

        int ConnectToServer();

        int Disconnect();


        int SendCmdAndReceive(int cmd, unsigned char *data, int len, ServerAck *ack);

        int ParseReceiveData(const unsigned char *data, int len);

        inline void SetAgvId(int id)        {curStatus.id = agvId = id;}
        inline void SetStatus(int status)   {curStatus.status = status;}
        inline void SetPowerLevel(int level) {curStatus.powerLevel = level;}
        inline void SetCarSpeed(int speed)  {curStatus.speed = speed;}
        inline void SetXPos(int xpos)       {curStatus.xPos = xpos;}
        inline void SetYPos(int ypos)       {curStatus.yPos = ypos;}
        inline void SetCarAngle(int angle)  {curStatus.carAngle = angle;}
        inline void SetLastPoint(int point) {curStatus.lastPoint = point;}
        inline void SetHeartBeatEnable(bool able) {heartBeatEnable = able;}
        inline bool IsOnLine()              {return m_isConnected;}

        int ReportStatusToServer();

        int ReportStatusToServer(ReportStatus &status);

        int ReportStatusToServer(int status, int power, int speed, int xpos, int ypos, int angle, int last);

        int ReportTaskStatus(int id, int status);

        int RequestNewPath(int xpos, int ypos, int lastStation);

        int SendHeartBeat();

        int SendAckToServer(int cmd, int result, const unsigned char *data, int dataLen);

        void cagvSchedulerSystemTimeSet(const unsigned char *data, int len);

        int  RequestSystemTimeSynchronization();
    public:
        CAgvSocketClient *client;
        pthread_mutex_t  socketLocker;
    private:
        string          m_ip;
        int             m_port;
        bool            m_loopEnable;
        bool            m_isConnected;
        int             lastCmd;
        int             cmdTimeOut;
        int             cmdTimer;
        ServerAck       lastCmdAck;
        sem_t           cmdSem;
        ReportStatus    curStatus;
        SetPath         pathList;
        int             heartBeatTimeout;
        bool            heartBeatEnable;
        unsigned short  agvId;
        unsigned char   receiveBuf[2048];
        unsigned char   sendBuf[2048];
};

#endif /* AGVSCHEDULER_H_ */
