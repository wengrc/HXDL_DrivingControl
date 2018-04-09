//
// Created by zxq on 16-11-17.
//

#ifndef MAGNAILFORKLIFT_AGVCONTROLER_H
#define MAGNAILFORKLIFT_AGVCONTROLER_H

#include "../AgvPublic.h"
#include "../devices/canbus/AgvCanOperator.h"
#include "../devices/canbus/AgvSensorManager.h"

#ifdef CryControler
#include "AgvMoveHelper.h"
#include "AgvBaseControler.h"
#endif

#define CONTROL_DEBUG                0

//public parameter
#define SUBLINE_LENGTH               5000.0     //辅助线的长度，单位 :mm*
#ifdef ZZS
#define LASER_WHEEL_LENGTH           2200.0     //旋转点到轮子的距离,单位 :mm
#define ANGULAR_RETUCTION_RATIO      1.0        //转弯减速系数
#endif
#ifdef HX
#define LASER_WHEEL_LENGTH           970.0     //旋转点到轮子的距离,单位 :mm
#define ANGULAR_RETUCTION_RATIO      4.0        //转弯减速系数
#endif
#define SPEED_SLID_VALUE             20         //平滑速度系数
#define UNIT_VALUE                   1          //单位值*
#define NEED_STOP_DECANGLE           89.0       //停止时目标点与当前点所成向量角度

//forward parameter
#define START_ADJUST_DISTANCE        1000.0     //校正距离，越大弧度越大，越平滑,单位 :mm
#define FORWARD_SLID_VALUE           2.0        //平滑校正系数
#define FORWARD_MIN_SPEED            500        //最小运行速度
#define FORWARD_RETUCTION_RATIO      1.0        //减速系数

//back parameter
#define BACK_SLID_VALUE              3.0        //平滑校正系数

#ifdef ZZS
#define BACK_ADJUST_DISTANCE         1500.0     //后退校正距离，越大弧度越大，越平滑,单位 :mm
#define BACK_MIN_SPEED               300        //最小运行速度
#define BACK_MID_SPEED               900        //细调运行速度
#define BACK_WHEEL_CORRECTION        0.4        //WHEEL的最小校正系数*
#define BACK_BANK_SCAN               2.1        //倾斜比例系数，越大校正越快，但是停止时的角度越大
#endif

#ifdef HX
#define BACK_ADJUST_DISTANCE         600.0     //后退校正距离，越大弧度越大，越平滑,单位 :mm
#define BACK_MIN_SPEED               50        //最小运行速度
#define BACK_MID_SPEED               250       //细调运行速度
#define BACK_WHEEL_CORRECTION        0.7        //WHEEL的最小校正系数*
#define BACK_BANK_SCAN               2.5        //倾斜比例系数，越大校正越快，但是停止时的角度越大
#endif

#define BACK_TAIL_CORRECTION         2.0        //后退旋转中心点的校正系数*
#define BACK_RETUCTION_RATIO         0.6        //减速系数


typedef struct stRunContents
{
    int     mode;
    int     xPos;
    int     yPos;
    float   angle;
    int     radius;
    int     speed;
    bool    needStop;
    int     liftHeight;
}RunContents;

typedef struct stPathMagStruct
{
    int     station;
    int     mode;
    int     xPos;
    int     yPos;
    float   angle;
    int     radius;
    int     speed;
    int     action;
    int     liftHeight;
}PathMagStruct;

class CAgvCarControler : CAgvThread
{
    public:
        CAgvCarControler();
        virtual ~CAgvCarControler();

        int Init();
        int InitSensors();
        int Close();

        int  LiftUp(int height);
        int  LiftDown();
        int  GetLiftStatus();
        bool LiftIsArrived(int hight);
        bool LiftIsAtTop();
        bool LiftStop();//2018-03-26
        bool LiftInit();//2018-03-26
        bool LiftIsAtBottom();
        bool SetTargetHeight(int hight);
        int SaveSourcePathMag(int id, int action, int high);
        bool CompareSourcePathMag(int id, int action, int high);


        int GetLaserStuta();
        bool LiftHaveCargo();
        bool SetBarrierDistance(int type, int id, int status,int distance);
        int  AutoSetBarrierDistance();

        int SetVoiceVolume(int volume);
        int PlayVoice(int id, int count);
        int SetRGBLight(int side, int red, int green, int blue);
        int ShowCurrentBatteryCapacity(int Cap);
        int ShowCurrentNetState(int State);
        int ShowCurrentCarState(int State);
        int SetChargeSwitch(bool stuta);
        int SetAgvSleepSwitch(bool stuta);
        int GetBatteryLevel();
        int GetBatteryCapatcity();


        int SetEnable(bool enable);
        int StopCar(bool enbreak);


        int GetRunningMode();
        int GetTargetSpeed();
        int  SetOperateMode(int mode);
        bool CarIsRunnging();



        int   IsLidarOK();

        int SaveLastPathMag(int station, int mode, int speed, int xpos,\
                            int ypos, float angle, int radius, int action, \
                            int liftheight);

        int CleanTargetStation();
        int SetTargetStation(int station, int mode, int speed, int xpos,\
                             int ypos, float angle, int radius, int action,\
                             int liftheight);

        int CleanLastPathMag();
        int ResetLastTargetStation();

    private:
        POSITION GetCurrentPosition();
        int   IsLostPosition();

        void Run();


    public:
        int                     lastLiftHeight;
        PathMagStruct           sourcePathMag;
    private:
        CAgvMoveHelper          *moveHelper;
        CAgvBaseControler       *baseController;
        PathMagStruct            lastPathMag;

        int                     liftSpeed;


        int                     operateMode;
        int                     currentX;
        int                     currentY;
        float                   currentAngle;
        int                     currentSpeed;
        RunContents             startPoint;
        RunContents             targetPoint;
        int                     distance;
        bool                    isFinished;
        bool                    isStopAfterFinish;
        bool                    runnable;
        bool                    carIsRunning;
        int                     runningMode;
        bool                    needMoveLift;

        CAgvLidarSensor         *lidar;
        CAgvControler           *controler;
        CAgvLifter              *lifter;
        CAgvBarrierSensor       *barriers;
        CAgvCargoSensor         *cargo;
        CAgvRgbLight            *rgbLight;
        CAgvButtonSensor        *buttons;
        CAgvVoicePlayer         *voicePlayer;
        CAgvPowerSensor         *power;
        CAgvHMISensor           *hmi;

        POSITION                destination;
        POSITION                nowPosition;
        POSITION                sublineStartPos;
        POSITION                carTailPoint;

        //controler
        int                     speed;
        int                     lastSpeed;
        int                     runDistance;
        int                     downSpeedValue;
        float                   angularDiff;
        float                   stopAngle;
        float                   decX;
        float                   decY;
        float                   disWheelToLine;
        float                   disTailToLine;
        float                   backWheelCorrection;
        float                   forwardDisKVlue;
        float                   forwardStartAdjustDis;
        float                   backDisKVlue;
        float                   checkValue;
        float                   lastCheckValue;

        CAgvLogs                *agvLog;
        CAgvSensorManager       sensorManager;
        CAgvCanOperator         canOperator;
        CAgvSetting             *setting;
};


#endif //MAGNAILFORKLIFT_AGVCONTROLER_H
