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
    Normal = 0,     //常规
    Important,      //重要
    Urgent,         //紧急

    PriorityMax
};

class CAgvEvent
{

    public:
        int     event;      //事件值
        int     param;      //参数
        int     priority;   //优先级
        string  name;       //名称
        void    *data;      //附带数据指针

};

enum EventList
{
    evNone = 0,             //
    ev10msTimer,            //10ms定时器
    ev100msTimer,           //100ms定时器
    ev500msTimer,           //500ms定时器
    ev1sTimer,              //1s定时器
    evUpDown,
    evUser,                 //用户消息
    evSysReady,             //系统准备完毕
    evSysClose,             //关闭系统
    evPowerDown,            //掉电
    evSysReboot,            //重启
    evNetworkOK,            //网络正常
    evNetworkError,         //网络异常
    evModemOnLine,          //Modem拨号成功
    evModemOffLine,         //Modem拨号断开
    evNearBarrier,          //近距离障碍
    evMidBarrier,           //中距离障碍
    evFarBarrier,           //远距离障碍
    evBarrierTouched,       //碰到触须 18
    evBarrierOk,            //障碍清除 19
    evDerailed,             //出轨
    evUrgentStop,           //紧急停止
    evLiftHeight,           //叉臂高度变化
    evLiftArrived,          //叉臂到达指定高度
    evLiftAtTop,            //叉臂到顶
    evLiftAtBottom,         //叉臂到底
    evKeyStart,             //启动按键按下
    evKeyStop,              //停止按键按下
    evKeyAutoMode,          //自动模式按键按下
    evKeyManualMode,        //手动模式按键按下
    evKeyLiftUp,            //升降器上升按键按下
    evKeyLiftDown,          //升降器下降按键按下
    evBattery,              //电池容量
    evBatteryLow,           //电池电量低
    evBatteryCharge,        //电池充电状态
    evStartCharge,          //开始充电
    evStopCharge,           //停止充电
    evSetSpeed,             //设置车速
    evGetNewPath,           //服务器下发新的路径
    evRequestPath,          //向服务器请求路径
    evApplyPath,            //使用新的路径
    evNextPathPoint,        //到下一个路点
    evPathFinished,         //路径完成
    evSchedulerOnline,      //调度系统在线
    evSchedulerOffline,     //调度系统离线
    evSensorOffline,           //传感器失联
    evSensorOnline,
    evPosition,             //坐标变化
    evLostPosition,         //坐标丢失
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
