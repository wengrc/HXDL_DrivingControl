/*
 * CAgvEvent.h
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#ifndef CAGVEVENT_H_
#define CAGVEVENT_H_

#include <string>
#include <pthread.h>

using namespace std;

enum Priority
{
    Normal = 0,     //����
    Important,      //��Ҫ
    Urgent,         //����

    PriorityMax
};

class CAgvEvent
{

    public:
        int     event;      //�¼�ֵ
        int     param;      //����
        int     priority;   //���ȼ�
        string  name;       //����
        void    *data;      //��������ָ��

};

enum EventList
{
    evNone = 0,             //
    ev10msTimer,            //10ms��ʱ��
    ev100msTimer,           //100ms��ʱ��
    ev500msTimer,           //500ms��ʱ��
    ev1sTimer,              //1s��ʱ��
    evUpDown,
    evUser,                 //�û���Ϣ
    evSysReady,             //ϵͳ׼�����
    evSysClose,             //�ر�ϵͳ
    evPowerDown,            //����
    evSysReboot,            //����
    evNetworkOK,            //��������
    evNetworkError,         //�����쳣
    evModemOnLine,          //Modem���ųɹ�
    evModemOffLine,         //Modem���ŶϿ�
    evNearBarrier,          //�������ϰ�
    evMidBarrier,           //�о����ϰ�
    evFarBarrier,           //Զ�����ϰ�
    evBarrierTouched,       //�������� 18
    evBarrierOk,            //�ϰ���� 19
    evDerailed,             //����
    evUrgentStop,           //����ֹͣ
    evLiftHeight,           //��۸߶ȱ仯
    evLiftArrived,          //��۵���ָ���߶�
    evLiftAtTop,            //��۵���
    evLiftAtBottom,         //��۵���
    evKeyStart,             //������������
    evKeyStop,              //ֹͣ��������
    evKeyAutoMode,          //�Զ�ģʽ��������
    evKeyManualMode,        //�ֶ�ģʽ��������
    evKeyLiftUp,            //������������������
    evKeyLiftDown,          //�������½���������
    evBattery,              //�������
    evBatteryLow,           //��ص�����
    evBatteryCharge,        //��س��״̬
    evStartCharge,          //��ʼ���
    evStopCharge,           //ֹͣ���
    evSetSpeed,             //���ó���
    evGetNewPath,           //�������·��µ�·��
    evRequestPath,          //�����������·��
    evApplyPath,            //ʹ���µ�·��
    evNextPathPoint,        //����һ��·��
    evPathFinished,         //·�����
    evSchedulerOnline,      //����ϵͳ����
    evSchedulerOffline,     //����ϵͳ����
    evSensorOffline,           //������ʧ��
    evSensorOnline,
    evPosition,             //����仯
    evLostPosition,         //���궪ʧ
    evPathMagError,
    evTurnOnCharge,
    evTurnOffCharge,
    evSetBarrierDistance,
    evAgvSleep,
    evAgvWakeUp,
    evLifterInit,
};


class IAgvEventListener
{
    public:
        virtual bool HandleEvent(CAgvEvent *evt) {if (evt){;} return false;}
};

#endif /* CAGVEVENT_H_ */
