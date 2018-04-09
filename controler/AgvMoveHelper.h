/*
 * AgvCarRunControl.h
 *
 *  Created on: 2017-5-4
 *      Author: zxq
 */

#ifndef AGVCARRUNCONTROL_H_
#define AGVCARRUNCONTROL_H_


#include "../AgvPublic.h"
#include "AgvBaseControler.h"
#include <string>

using namespace std;
typedef struct StRunContext
{
    int     station;
    int     mode;
    int     xPos;
    int     yPos;
    float   angle;
    int     radius;
    int     speed;
    int     action;
    bool    needStop;
    int     liftHeight;
}RunContext;

/**
 * AGV 移动控制类
 */
class CAgvMoveHelper
{
    public:
        /**
         * 构造函数
         * @param controler  底盘控制器类的指针
         */
        CAgvMoveHelper(const char *name, CAgvBaseControler *controler);

        /**
         * 析构
         */
        virtual ~CAgvMoveHelper();

        /**
         * 开始运行
         * @return
         */
        int Start();

        /**
         * 停止运行
         * @return
         */
        int Stop();

        /**
         * 在控制器中循环调用的函数，约10ms调用一次
         * @return
         */
        virtual int LoopRunning() = 0;

        /**
         * 设置移动的目的地状态和动作信息
         * @param station       站点号
         * @param mode          行进的模式
         * @param speed         行进的速度
         * @param xpos          到站之后的X坐标
         * @param ypos          到站之后的Y坐标
         * @param angle         到站的车身角度
         * @param radius        转弯的半径
         * @param action        到站之后的动作
         * @param liftheight    到之后的叉臂的高度
         * @return
         */
        virtual int SetTarget(int station, int mode, int speed, int xpos, int ypos, float angle, int radius, int action, int liftheight) = 0;

        /**
         *清除目的地信息
         * @return
         */
        virtual int ClearTarget() = 0;

        virtual int GetRunningMode() = 0;

        virtual int SetBarrierStatus(int status) = 0;

        /**
         *设置运行的状态
         * @param run 是否在运行
         */
        inline void SetRunning(bool run) {isRunning = run;}

        /**
         *检查是否在运行中
         * @return
         */
        bool IsRunning()    {return isRunning;}

        inline const string &Name() {return name;}



    protected:
        string              name;
        CAgvBaseControler   *controler;
        CAgvControler       *s_Controler;
        CAgvLidarSensor     *s_Lidar;
        RunContext          startPoint;
        RunContext          targetPoint;

        bool                isRunning;
        int                 distance;
        bool                isFinished;
        bool                isStopAfterFinish;
        int                 runningMode;
        bool                needMoveLift;
        int                 liftHeight;
        int                 CarRunningMode;
};


#endif /* AGVCARRUNCONTROL_H_ */
