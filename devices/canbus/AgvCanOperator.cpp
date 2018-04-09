/*
 * AgvCanOperator.cpp
 *
 *  Created on: 2016-10-18
 *      Author: zxq
 */

#include "AgvCanOperator.h"

#define LOGTAG  "AgvCanOperator"

CAgvCanOperator::CAgvCanOperator() : CAgvThread("CanOperatorThread")
{
    agvLogs = &CAgvLogs::Instance();
    m_listenerList.reserve(16);
    m_loopEnable = false;
    m_heartBeatTimer = 0;
    m_canFilterCount = 0;
    memset(m_filter, 0, sizeof(m_filter));
    pthread_mutex_init(&m_socketLocker, NULL);
}

CAgvCanOperator::~CAgvCanOperator()
{
    StopMonitor();
    m_socket.Close();
    pthread_mutex_destroy(&m_socketLocker);
}

int CAgvCanOperator::Init()
{
    int ret = m_socket.Initial(0, 500000, true);
    if (ret != 0)
    {
        LogError(LOGTAG, "Init CanBus socket error");
        return ret;
    }

    ResetCanFilter();

    LogInfo(LOGTAG, "CanBus socket operator init finished...\n");
    return ret;
}

int CAgvCanOperator::AddCanListener(IAgvCanListener *listener)
{
    if (listener == NULL)
    {
        return -1;
    }
    for (unsigned int i = 0; i < m_listenerList.size(); i++)
    {
        if (m_listenerList[i] == listener)
        {
            return 0;
        }
    }
    m_listenerList.push_back(listener);
    return 0;
}

int CAgvCanOperator::RemoveCanListener(IAgvCanListener *listener)
{
    if (listener == NULL)
    {
        return -1;
    }

    for (vector<IAgvCanListener *>::iterator it = m_listenerList.begin();
         it != m_listenerList.end(); ++it)
    {
        if (*it == listener)
        {
            m_listenerList.erase(it);
            return 0;
        }
    }

    return 1;
}

int CAgvCanOperator::AddCanFilter(unsigned int canId, int mask)
{
    unsigned int i = 0;
    for (i = 0; i < m_canFilterCount; i++)
    {
        if (m_filter[i].can_id == canId)
        {
            m_filter[i].can_mask = mask;
            return 0;
        }
    }

    if (m_canFilterCount < (sizeof(m_filter) / sizeof(struct can_filter) - 1))
    {
        m_filter[m_canFilterCount].can_id = canId;
        m_filter[m_canFilterCount].can_mask = mask;
        m_canFilterCount++;
        return 0;
    }

    return -1;
}

int CAgvCanOperator::ResetCanFilter()
{
    memset(m_filter, 0, sizeof(m_filter));
    m_canFilterCount = 0;

    AddCanFilter(ES0_CANID, CAN_EFF_MASK);
    return 0;
}

int CAgvCanOperator::StartMonitor()
{
    if (IsRunning())
    {
        return 0;
    }

    m_loopEnable = true;

    Start(true);
    LogInfo("CanOperator", "CanBus monitor started");
    return 0;
}

int CAgvCanOperator::StopMonitor()
{
    m_loopEnable = false;
    Stop();

    LogInfo("CanOperator", "Can monitor stoped");
    return 0;
}

void CAgvCanOperator::Run()
{
    CanBusMsg  canMsg;
    unsigned char recvBuf[2048] = {0};
    unsigned int recvLength = 0, totalLength = 0;
    unsigned short recvCrc = 0;
    int multiMode = 0;

    LogInfo(LOGTAG, "CanOperator thread started...\n");

    while(m_loopEnable)
    {
        canMsg = GetCanBusMessage(10);

        if (canMsg.ret < 0 && canMsg.ret != -3)
        {
            DEBUG_ERROR("CanOperator", "Read CAN data failed!:%d", canMsg.ret);
            LogError(LOGTAG, "CanBus read data failed!%d", canMsg.ret);
            m_loopEnable = false;
            break;
        }

        if (canMsg.len == 0)//CanBus data is free
        {
            usleep(10*1000);

            if (m_heartBeatTimer++ > 10)
            {
                m_heartBeatTimer = 0;


                for (unsigned int i = 0; i < m_listenerList.size(); i++)
                {

                    m_listenerList[i]->SendHeartBeat();
                }
            }
            continue;
        }


//        DEBUG_INFO(LOGTAG, "Get CanId[%X] data[%d]:", canMsg.canId, canMsg.len);
//        for (unsigned int i = 0; i < canMsg.len; i++) {
//            printf("%02X ", canMsg.data[i]);
//        }
//        printf("\n");


        if (canMsg.data[0] == 0xFE && canMsg.data[1] == 0xFE)
        {
            if (canMsg.len == 2 && multiMode == 1)    //分包结尾
            {
                if (recvLength == totalLength)  //接收完毕
                {
                    unsigned short crc = CAgvCheckSum::GetCrc16(recvBuf, totalLength, 0);
                    if (crc == recvCrc)
                    {
                        //校验和OK
//                        DEBUG_INFO(LOGTAG, "Finish multi package");
                        multiMode = 2;
                    }
                    else
                    {
                        LogError(LOGTAG, "Package data CRC error! Calc:%08X, Recv:%08X", crc, recvCrc);
                        for (unsigned int i = 0; i < recvLength; i++)
                        {
                            printf("%02X ", recvBuf[i]);
                        }
                        printf("\n");

                        multiMode = 0;
                        recvLength = 0;
                        totalLength = 0;
                        continue;
                    }
                }
                else if (recvLength > totalLength)
                {
//                    LogError(LOGTAG, "=================Package data length error! Total:%d, Recv:%d=================", totalLength, recvLength);
//                    for (unsigned int i = 0; i < recvLength; i++)
//                    {
//                        printf("%02X ", recvBuf[i]);
//                    }
//                    printf("\n");
                    multiMode = 0;
                    recvLength = 0;
                    totalLength = 0;
                    continue;
                }
            }

            else if (canMsg.len == 6/* && multiMode == 0*/)   //分包开头
            {
                recvLength = 0;
                totalLength = CAgvUtils::Buffer2Bin(canMsg.data + 2, 2);
                recvCrc = CAgvUtils::Buffer2Bin(canMsg.data + 4, 2);
                multiMode = 1;
                continue;
            }
        }


        if (multiMode == 2)
        {
            multiMode = 0;
        }
        else
        {
            if (recvLength + canMsg.len > sizeof(recvBuf))
            {
                LogWarn(LOGTAG, "Read buffer is overflow!");
                memcpy(recvBuf + recvLength, canMsg.data, sizeof(recvBuf) - recvLength);
                recvLength += sizeof(recvBuf) - recvLength;
                multiMode = 0;
            }
            else
            {
                memcpy(recvBuf + recvLength, canMsg.data, canMsg.len);
                recvLength += canMsg.len;
                if (multiMode == 1)
                {
                    continue;
                }
            }
        }


        for (unsigned int i = 0; i < m_listenerList.size(); i++)
        {
            m_listenerList[i]->HandlerData(canMsg.canId, recvBuf, recvLength);
        }

        recvLength = 0;
        totalLength = 0;
    }

    LogInfo(LOGTAG, "CanOperator thread finished...\n");
}

CanBusMsg CAgvCanOperator::GetCanBusMessage(int timeout)
{
    CanBusMsg  msg;
    memset ( &msg, 0, sizeof(msg) );
    pthread_mutex_lock(&m_socketLocker);
    m_socket.SetReadTimeout(timeout);
    msg.ret = m_socket.Read (m_filter, m_canFilterCount, &msg.canId, &msg.len, msg.data);

    pthread_mutex_unlock(&m_socketLocker);
    return msg;
}


int CAgvCanOperator::SendData(int canId, const unsigned char* data, unsigned int len)
{
    if (canId <= 0)
    {
        return -1;
    }

    int ret = -1;
    pthread_mutex_lock(&m_socketLocker);
    ret = m_socket.Write(canId, data, len);
    usleep(2*1000);//10*1000->500--0308--YJW
    pthread_mutex_unlock(&m_socketLocker);
    return ret;
}

