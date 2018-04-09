/*
 * AgvRoutePlanner.h
 *
 *  Created on: 2016-10-19
 *      Author: zxq
 */

#ifndef AGVROUTEPLANNER_H_
#define AGVROUTEPLANNER_H_

#include <list>
#include <vector>

#include "AgvScheduler.h"
#include "../main/AgvSetting.h"

using namespace std;

typedef struct _WayPoint
{
    int     id;             //·��ID
    int     cardId;         //վ�㿨��
    int     xPos;           //վ��X����
    int     yPos;           //վ��Y����
    double  angle;          //����·���Ƕ�
    int     radius;         //ת��뾶
    int     runMode;        //����ģʽ��0��ǰ�� 1������ 2����ת 3����ת
    int     action;         //��վ��Ķ���
    int     liftHeight;     //��۸߶�
    bool    actionFinish;   //�����Ƿ�����
} WayPoint;

enum RunMode {
    StopMode = 0,           //0x00 ֹͣ
    GoForwardMode,          //0x01 ǰ��
    GoBackwardMode,         //0x02 ����
    TurnLeftMode,           //0x03 ��ת
    TurnRightMode,          //0x04 ��ת
    RotateLeftMode,         //0x05 ԭ����ת
    RotateRightMode,        //0x06 ԭ����ת
    ForwardToArcMode,       //0x07 ǰ��������
    BackwardToArcMode,      //0x08 ���˵�����
    GuideWithVision,        //0x09 ���Ӿ�����
    CarryGoods,             //0x10 ���
};

enum ArrivedAcation {
    ActionRun = 0,          //0x00 ��������
    ActionStop,             //0x01 ͣվ
    ActionSpeedDown,        //0x02 ���ٵ���С�ٶ�
    ActionResumeSpeed,      //0x03 ���ٵ������ٶ�
    ActionLiftUpDown,       //0x04 �������
    ActionCharge,           //0x05 ���
};


class CAgvRoutePlanner
{
    public:
        CAgvRoutePlanner();
        virtual ~CAgvRoutePlanner();

        int Start();

        int Stop();

        int OnGetNewPath(const vector<StationInfo*> &list);

        int RequestNewPath();

        void ResetPathList();

        int ReloadPathList();

        int InsertPoint(WayPoint *first, WayPoint *second);

        bool MoveToNextStation();

        void CleanLastStationPoint();

        bool MoveToLastStation();

        int OnActionFinished(int lastLiftHight);

        WayPoint *GetTargetStation();

        WayPoint *GetLastStation();

        void UpdateCarInfo(int x, int y, int angle, int speed, int power, int status);

        void RequestSystemTimeSynchronization();
        inline int PathCount()      {return pathList.size();}
        inline int TargetPathId()   {return targetPathId;}
        inline int LastPathId()     {return lastPathId;}
        inline vector<WayPoint *> PathList() {return pathList;}

        inline void SetWheelBase(int val) {wheelBase = val;}
    private:
        void clearPathList();
        int savePathInfo(bool all);
        int loadPathInfo();
    private:
        CAgvScheduler           scheduler;
        vector<WayPoint *>      pathList;
        CAgvSetting             *setting;
        int                     targetPathId;
        int                     lastPathId;
        int                     lastStationPoint;
        int                     xPos;
        int                     yPos;
        int                     carAngle;
        int                     wheelBase;
        int                     carDirection;
        bool                    isWayPointFinished;
};

#endif /* AGVROUTEPLANNER_H_ */
