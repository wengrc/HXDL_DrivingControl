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
#define SUBLINE_LENGTH               5000.0     //�����ߵĳ��ȣ���λ :mm*
#ifdef ZZS
#define LASER_WHEEL_LENGTH           2200.0     //��ת�㵽���ӵľ���,��λ :mm
#define ANGULAR_RETUCTION_RATIO      1.0        //ת�����ϵ��
#endif
#ifdef HX
#define LASER_WHEEL_LENGTH           970.0     //��ת�㵽���ӵľ���,��λ :mm
#define ANGULAR_RETUCTION_RATIO      4.0        //ת�����ϵ��
#endif
#define SPEED_SLID_VALUE             20         //ƽ���ٶ�ϵ��
#define UNIT_VALUE                   1          //��λֵ*
#define NEED_STOP_DECANGLE           89.0       //ֹͣʱĿ����뵱ǰ�����������Ƕ�

//forward parameter
#define START_ADJUST_DISTANCE        1000.0     //У�����룬Խ�󻡶�Խ��Խƽ��,��λ :mm
#define FORWARD_SLID_VALUE           2.0        //ƽ��У��ϵ��
#define FORWARD_MIN_SPEED            500        //��С�����ٶ�
#define FORWARD_RETUCTION_RATIO      1.0        //����ϵ��

//back parameter
#define BACK_SLID_VALUE              3.0        //ƽ��У��ϵ��

#ifdef ZZS
#define BACK_ADJUST_DISTANCE         1500.0     //����У�����룬Խ�󻡶�Խ��Խƽ��,��λ :mm
#define BACK_MIN_SPEED               300        //��С�����ٶ�
#define BACK_MID_SPEED               900        //ϸ�������ٶ�
#define BACK_WHEEL_CORRECTION        0.4        //WHEEL����СУ��ϵ��*
#define BACK_BANK_SCAN               2.1        //��б����ϵ����Խ��У��Խ�죬����ֹͣʱ�ĽǶ�Խ��
#endif

#ifdef HX
#define BACK_ADJUST_DISTANCE         600.0     //����У�����룬Խ�󻡶�Խ��Խƽ��,��λ :mm
#define BACK_MIN_SPEED               50        //��С�����ٶ�
#define BACK_MID_SPEED               250       //ϸ�������ٶ�
#define BACK_WHEEL_CORRECTION        0.7        //WHEEL����СУ��ϵ��*
#define BACK_BANK_SCAN               2.5        //��б����ϵ����Խ��У��Խ�죬����ֹͣʱ�ĽǶ�Խ��
#endif

#define BACK_TAIL_CORRECTION         2.0        //������ת���ĵ��У��ϵ��*
#define BACK_RETUCTION_RATIO         0.6        //����ϵ��


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
