/*
 * AgvCar.h
 *
 *  Created on: 2016-10-19
 *      Author: zxq
 */

#ifndef AGVCAR_H_
#define AGVCAR_H_

#include "../AgvPublic.h"

#include "AgvCarControler.h"

#include "AgvRoutePlanner.h"


enum CarStatus
{
    StatusInit = -1,
    StatusNormal = 0,
    StatusRunning,
    StatusStopped,
    StatusTurningLeft,
    StatusTurningRight,
    StatusError,
    StatusWarning,
    StatusUpdateFirmware,
    StatusLowPower,
};


enum CarError
{
    NoError = 0,
    ErrorSystem,
    ErrorNetWork,
    ErrorModem,
    ErrorModule,
    ErrorCommunication,
    ErrorUrgentStop,
    ErrorDerail,
    ErrorIrNear,
    ErrorIrMid,
    ErrorIrFar,
    ErrorUltrasonicNear,
    ErrorUltrasonicMid,
    ErrorUltrasonicFar,
    ErrorTouch1,
    ErrorTouch2,
    ErrorTouch3,
    ErrorTouch4,
    ErrorSensorLost,
    ErrorBatteryLow,
    ErrorConnectServer,
    ErrorPosition,
};

enum HmiShowCarState
{
    CarState_ConnectionNetError,
    CarState_ConnectionNetNormal,
    CarState_ConnectionServerNormal,
    CarState_ConnectionServerError,
    CarState_ManualMode,
    CarState_AutoMode,
    CarState_NavigatorNormal,
    CarState_NavigatorFault,
    CarState_SensorNormal,
    CarState_SensorLoss,
    CarState_AheadFarObstacle,
    CarState_AheadMiddleObstacle,
    CarState_AheadNearObstacle,
    CarState_AheadTentaclesObstacle,
    CarState_AheadCrashBarObstacle,
    CarState_BackTentaclesObstacle,
    CarState_BackLaserObstacle,
    CarState_NoObstacle,
    CarState_LowPower,
    CarState_ReceiveNewPathMessage,
    CarState_StartExecutionNextPoint,
    CarState_PathExecutionComplete,
    CarState_PathMessageError,
    CarState_ForkArmToTheTop,
    CarState_ForkArmToTheBottom,
};


typedef struct _CarInfo{
        int             XPos;       //当前X坐标，单位mm
        int             YPos;       //当前Y坐标，单位mm
        int             Angle;      //当前车身的角度
        int             Speed;      //当前车的速度，单位mm/sec
        int             Status;     //当前车的状态
        unsigned int    Error;      //当前的错误标记
        int				voice;
}CarInfo;



class CAgvCar : public IAgvEventListener
{
    public:
        CAgvCar();
        virtual ~CAgvCar();

        int Init();

        int Destroy();

        int StartRun();

        int StopCar(bool all);



        int SetLiftHeight(int height);


        inline CarInfo &CurrentInfo() {return carInfo;}

        int SetCarStatus(int status, int para);


        int  SetErrorFlag(int flag);
        int  ClearErrorFlag(int flag);
        bool IsHaveError(int flag);





        void OnLiftAtTop();

        void OnLiftAtBottom();

        void OnLiftArrivedHeight(int height);

        int GetRunAction(WayPoint *curPoint);

        int WhetherJumpCurPoint(WayPoint *curPoint);

        int GoToNextPoint(bool isfirst);

        int DoPathPointAction();

        void OnArrivedStation(int id);

        void OnStationActionFinished();

        inline bool IsCarStoped() {return !controler.CarIsRunnging();}

    private:

        bool HandleEvent(CAgvEvent *evt);

        int OnBarrierMessage(int event, int dev);



        int OnBatteryMessage(int vol,int cap,int ele);


        int OnStationMessage(int x, int y, int angle, bool derail);




    private:
        CAgvSetting         *setting;
        CAgvLogs            *agvLogs;
        CarInfo             carInfo;
        CarInfo             lastCarInfo;

        CAgvCarControler    controler;
        CAgvRoutePlanner    router;

        int                 liftHeight;
        int                 batteryVoltage;
        int                 batteryCapacity;
        int                 carElectricity;

        int                 speedFormServer;

        int                 operateMode;
        int                 reportStatus;
        int                 currentPathAction;
        int                 currentPathId;
        int                 carRunMode;

        bool                StopButton;


};

#endif /* AGVCAR_H_ */
