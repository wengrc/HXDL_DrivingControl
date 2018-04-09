/*
 * AgvCanOperator.h
 *
 *  Created on: 2016-10-18
 *      Author: zxq
 */

#ifndef AGVCANOPERATOR_H_
#define AGVCANOPERATOR_H_

#include <pthread.h>

#include "../../AgvPublic.h"

#define MAXSENSORS  32



#define     ES0_CANID               0x0301

//Car Controler
#define     CARCONTROLER_RCANID     0x0202
#define     CARCONTROLER_WANID      0x0212

//Navigation Mode
#define     LIDAR_RCANID            0x0102
#define     LIDAR_WCANID            0x0000

#define     LIFTER_RCANID           0x0804
#define     LIFTER_WCANID           0x0814

#define     POWER_RCANID            0x0407
#define     POWER_WCANID            0x0417

//Front Barrier Mode
#define     BARRIER_RCANID          0x0101
#define     BARRIER_WCANID          0x0111


//Media Mode
#define     RGBLIGHT_RCANID         0x0401
#define     RGBLIGHT_WCANID         0x0412

#define     KEY_RCANID              0x0401
#define     KEY_WCANID              0x0411

#define     HMI_RCANID              0x0501
#define     HMI_WCANID              0x0511

//Voice Mode
#define     VOICE_RCANID            0x0502
#define     VOICE_WCANID            0x0512
/*************************************************/


#define     GROSCOPE_RCANID         0x0103
#define     GROSCOPE_WCANID         0x0000


#define     CARGO_RCANID            0x0302
#define     CARGO_WCANID            0x0312

#define     MAG1_RCANID             0x0200
#define     MAG1_WCANID             0x0210

#define     MAG2_RCANID             0x0201
#define     MAG2_WCANID             0x0211

#define     RFID_RCANID             0x0206
#define     RFID_WCANID             0x0000


typedef struct _CanBusMsg
{
    uint canId;
    uint len;
    unsigned char data[8];
    int  ret;
}CanBusMsg;


enum KeyType
{
    KeyUrgent,
    KeyLiftTopBottom,
    KeyStartStop,
    KeyAutoManual
};


enum OperateMode
{
    AutoMode,
    ManualMode,
    NoKownMode,
};

class IAgvCanListener
{
    public:
        IAgvCanListener() {}
        virtual ~IAgvCanListener() {}

        virtual int SendHeartBeat() = 0;
        virtual int HandlerData(unsigned int canId, const unsigned char *data, int len) = 0;
};

class CAgvCanOperator : public CAgvThread
{
    public:
        CAgvCanOperator();

        virtual ~CAgvCanOperator();

        int Init();

        int AddCanListener(IAgvCanListener *listener);

        int RemoveCanListener(IAgvCanListener *listener);

        int AddCanFilter(unsigned int canId, int mask = CAN_EFF_MASK);

        int ResetCanFilter();

        int StartMonitor();

        int StopMonitor();

        void Run();

        CanBusMsg GetCanBusMessage(int timeout = 10);

        int SendData(int canId, const unsigned char *data, unsigned int len);

    private:
        bool                        m_loopEnable;
        vector<IAgvCanListener *>   m_listenerList;
        pthread_mutex_t             m_socketLocker;
        CAgvSocketCan               m_socket;
        struct can_filter           m_filter[MAXSENSORS];
        unsigned int                m_canFilterCount;
        CAgvLogs                    *agvLogs;
        int                         m_heartBeatTimer;
};

#endif /* AGVCANOPERATOR_H_ */
