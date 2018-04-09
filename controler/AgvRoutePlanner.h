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
    int     id;             //路径ID
    int     cardId;         //站点卡号
    int     xPos;           //站点X坐标
    int     yPos;           //站点Y坐标
    double  angle;          //行走路径角度
    int     radius;         //转弯半径
    int     runMode;        //运行模式，0：前进 1：后退 2：左转 3：右转
    int     action;         //到站后的动作
    int     liftHeight;     //叉臂高度
    bool    actionFinish;   //动作是否做完
} WayPoint;

enum RunMode {
    StopMode = 0,           //0x00 停止
    GoForwardMode,          //0x01 前进
    GoBackwardMode,         //0x02 后退
    TurnLeftMode,           //0x03 左转
    TurnRightMode,          //0x04 右转
    RotateLeftMode,         //0x05 原地左转
    RotateRightMode,        //0x06 原地右转
    ForwardToArcMode,       //0x07 前进到曲线
    BackwardToArcMode,      //0x08 后退到曲线
    GuideWithVision,        //0x09 用视觉引导
    CarryGoods,             //0x10 叉货
};

enum ArrivedAcation {
    ActionRun = 0,          //0x00 保持运行
    ActionStop,             //0x01 停站
    ActionSpeedDown,        //0x02 减速到最小速度
    ActionResumeSpeed,      //0x03 加速到正常速度
    ActionLiftUpDown,       //0x04 叉臂升降
    ActionCharge,           //0x05 充电
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
