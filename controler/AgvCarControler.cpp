//
// Created by zxq on 16-11-17.
//

#include "AgvCarControler.h"
#include "AgvRoutePlanner.h"
#include "AgvSteeringBaseControler.h"

#ifdef CryControler
#include "AgvMoveHelperForklift.h"
#endif

#define LOGTAG      "CarControler"

#define UpAndRunFlag 0

CAgvCarControler::CAgvCarControler() : CAgvThread("CarControler")
{
    agvLog    = &CAgvLogs::Instance();

    liftSpeed       = 30;
    operateMode     = NoKownMode;
    currentX     = 0;
    currentY     = 0;
    currentSpeed = 0;
    currentAngle = 0;
    runningMode  = 0;
    isFinished        = false;
    isStopAfterFinish = false;
    runnable     = false;
    carIsRunning = false;
    needMoveLift = false;
    lastSpeed    = 0;

    moveHelper     = NULL;
    baseController = NULL;
    memset(&sourcePathMag,0,sizeof(sourcePathMag));
}

CAgvCarControler::~CAgvCarControler()
{
    Close();

    if (moveHelper != NULL)
    {
        delete moveHelper;
        moveHelper = NULL;
    }
    if (baseController != NULL)
    {
        delete baseController;
        baseController = NULL;
    }
}

int CAgvCarControler::Close()
{
    if (runnable)
    {
        baseController->StopCar(true);
        moveHelper->Stop();
        runnable = false;
        Stop();
        sensorManager.CloseAllSensors();
        canOperator.StopMonitor();
    }
    return 0;
}

int CAgvCarControler::Init()
{
    setting = &CAgvSetting::Instance();

    DEBUG_INFO(LOGTAG, "Start init CAN bus...");
    if (0 != canOperator.Init())
    {
        LogError(LOGTAG, "Init Can operator failed!");
        return -1;
    }
    InitSensors();
    canOperator.StartMonitor();

    if (NULL == baseController)
    {
        baseController = new CAgvSteeringBaseControler();
    }
    if (baseController->Init())
    {
        LogWarn(LOGTAG, "Warning!!! BaseControler init failed!");
    }

    if(NULL == moveHelper)
    {
        moveHelper = new CAgvMoveHelperForklift(baseController);
    }

    runnable = true;
    Start(true);
    return 0;
}

int CAgvCarControler::InitSensors()
{
    sensorManager.SetCanOperator(&canOperator);

    if (lidar == NULL)
    {
        lidar = CAgvLidarSensor::Instance();
        sensorManager.AddSensor(lidar);
    }

    if (controler == NULL)
    {
        controler = CAgvControler::Instance();
        sensorManager.AddSensor(controler);
    }

    if (lifter == NULL)
    {
        lifter = new CAgvLifter();
        sensorManager.AddSensor(lifter);
    }

    if (barriers == NULL)
    {
        barriers = new CAgvBarrierSensor();
        sensorManager.AddSensor(barriers);
    }

    if (cargo == NULL)
    {
        cargo = new CAgvCargoSensor();
        sensorManager.AddSensor(cargo);
    }

    if (rgbLight == NULL)
    {
        rgbLight = new CAgvRgbLight();
        sensorManager.AddSensor(rgbLight);
    }

    if (buttons == NULL)
    {
        buttons = new CAgvButtonSensor();
        sensorManager.AddSensor(buttons);
    }

    if (voicePlayer == NULL)
    {
        voicePlayer = new CAgvVoicePlayer();
        sensorManager.AddSensor(voicePlayer);
    }

    if (power == NULL)
    {
        power = new CAgvPowerSensor(CAgvSetting::Instance().BatteryType);
        sensorManager.AddSensor(power);
    }

    if(hmi == NULL)
    {
        hmi = new CAgvHMISensor();
        sensorManager.AddSensor(hmi);
    }

    return sensorManager.InitAllSensors();
}

/**********************Lifter operation*******************************/
int CAgvCarControler::LiftUp(int height)
{
    if (lifter == NULL)
    {
        return -1;
    }

    return lifter->SetLiftUp(30, height);
}

int CAgvCarControler::LiftDown()
{
    if (lifter == NULL)
    {
        return -1;
    }
    return lifter->SetLiftDown(30, 0);
}

int CAgvCarControler::GetLiftStatus()
{
    if (lifter == NULL)
    {
        return -1;
    }
    return lifter->GetLiftStatus();
}

bool CAgvCarControler::LiftIsArrived(int hight)
{
    if (lifter == NULL)
    {
        return -1;
    }
    return lifter->LiftIsArrived(hight);
}

bool CAgvCarControler::SetTargetHeight(int hight)
{
    if (lifter == NULL)
    {
        return -1;
    }
    return lifter->SetTargetHeight(hight);
}
int CAgvCarControler::SaveSourcePathMag(int id,int action,int high)
{
#if UpAndRunFlag
    if(sourcePathMag.station != id)
    {
        memset(&sourcePathMag,0,sizeof(sourcePathMag));
        sourcePathMag.station = id;
        sourcePathMag.action = action;
        sourcePathMag.liftHeight = high;
        printf("~~~~~~~sourcePathMag.station = %d,sourcePathMag.action = %d~\n",sourcePathMag.station,sourcePathMag.action);
    }
#endif
    return 0;
}

bool CAgvCarControler::CompareSourcePathMag(int id,int action,int high)
{
#if UpAndRunFlag
    if(sourcePathMag.station == id)
    {
        if((sourcePathMag.action != 4 && sourcePathMag.liftHeight >= 0)
            && (sourcePathMag.action != action && sourcePathMag.liftHeight == high))
        {
                return true;
        }
    }
#endif
    return false;
}

bool CAgvCarControler::LiftIsAtTop()
{
    if (lifter == NULL)
    {
        return false;
    }
    return lifter->LiftIsAtTop();
}

bool CAgvCarControler::LiftIsAtBottom()
{
    if (lifter == NULL)
    {
        return false;
    }
    return lifter->LiftIsAtBottom();
}

//2018-03-26
bool CAgvCarControler::LiftStop()
{
#if  UpAndRunFlag
    if(lifter == NULL)
    {
        return false;
    }
    return lifter->StopLift();
#endif
    return false;
}

//2018-03-26
bool CAgvCarControler::LiftInit()
{
    if(lifter == NULL)
    {
        return false;
    }
    return lifter->InitLift();
}

bool CAgvCarControler::LiftHaveCargo()
{
    if (cargo == NULL)
    {
        return false;
    }
    return cargo->IsHaveCargo();
}

bool CAgvCarControler::SetBarrierDistance(int type, int id, int status,int distance)
{
    if (barriers == NULL)
    {
        return false;
    }
    return barriers->SetBarrierDistance(type,id,status,distance);
}

int CAgvCarControler::AutoSetBarrierDistance()
{
    if(controler->speed > 700 && controler->direction == 1)
    {
        SetBarrierDistance(2,1,0,13);
    }
//    else if(controler->speed > 400 && controler->direction == 1)
//    {
//        SetBarrierDistance(2,1,0,12);
//    }
//    else if(controler->speed > 300 && controler->direction == 1)
//    {
//        SetBarrierDistance(2,1,0,11);
//    }

    else
    {
        SetBarrierDistance(2,1,0,10);
    }

    return 0;
}
/*******************************Other device**********************************************/
int CAgvCarControler::SetVoiceVolume(int volume)
{
    if (voicePlayer == NULL)
    {
        return -1;
    }
    return voicePlayer->SetVolume(volume);
}

int CAgvCarControler::PlayVoice(int id, int count)
{
    if (voicePlayer == NULL)
    {
        return -1;
    }

    return voicePlayer->PlayVoice(id, count);
}

int CAgvCarControler::SetRGBLight(int side, int red, int green, int blue)
{
    if (rgbLight == NULL)
    {

        return -1;
    }

    return rgbLight->SetLight(side, red, green, blue);
}

int CAgvCarControler::ShowCurrentBatteryCapacity(int Cap)
{
    if(hmi == NULL)
    {
        return -1;
    }
    return hmi->ShowBatteryCapacity(Cap);
}

int CAgvCarControler::ShowCurrentNetState(int State)
{
    if(hmi == NULL)
    {
        return -1;
    }
    return hmi->ShowNetState(State);
}

int CAgvCarControler::ShowCurrentCarState(int State)
{
    if(hmi == NULL)
    {
        return -1;
    }
    return hmi->ShowCarState(State);
}

int CAgvCarControler::SetChargeSwitch(bool stuta)
{
    if (power == NULL)
    {
        return -1;
    }
    return power->SetChargeSwitch(stuta);
}

int CAgvCarControler::SetAgvSleepSwitch(bool stuta)
{
    if (power == NULL)
    {
        return -1;
    }
    return power->SetAgvSleepSwitch(stuta);
}

int CAgvCarControler::GetBatteryLevel()
{
    return power->level;
}

int CAgvCarControler::GetBatteryCapatcity()
{
    return power->capacity;
}

int CAgvCarControler::SetEnable(bool enable)
{
    if (controler == NULL)
    {
        return -1;
    }
    return controler->SetEnable(enable);
}

int CAgvCarControler::StopCar(bool enbreak)
{
    lastLiftHeight = lifter->currentHeight;
    CleanTargetStation();
    return baseController->StopCar(enbreak);
}

int CAgvCarControler::GetRunningMode()
{
    return moveHelper->GetRunningMode();
}

int CAgvCarControler::GetTargetSpeed()
{
    return controler->speed;
}

int CAgvCarControler::SetOperateMode(int mode)
{
    operateMode = mode;
    return 0;
}

bool CAgvCarControler::CarIsRunnging()
{
    return (controler->speed > 0);
}


POSITION CAgvCarControler::GetCurrentPosition()
{
    POSITION currentPosition = { lidar->xPos, lidar->yPos, lidar->angle };
    return currentPosition;
}

int CAgvCarControler::IsLostPosition()
{
    static int losePositionCount = 0;
    static POSITION lastPosition;
    POSITION nowPosition = GetCurrentPosition();

    if(nowPosition.xPos == lastPosition.xPos && \
            nowPosition.yPos == lastPosition.yPos && \
            nowPosition.angle == lastPosition.angle)
    {
        losePositionCount++;

        if(controler->speed > 0)
        {
            if(losePositionCount > setting->maxTimeOfLostPos)
            {
                losePositionCount = 0;
                return -1;
            }
        }
        else
        {
            if(losePositionCount > (setting->maxTimeOfLostPos + 1000))
            {
                losePositionCount = 0;
                return -1;
            }
        }

    }
    else
    {
        losePositionCount = 0;
    }

    lastPosition.xPos = nowPosition.xPos;
    lastPosition.yPos = nowPosition.yPos;
    lastPosition.angle = nowPosition.angle;
    return 0;
}

int CAgvCarControler::IsLidarOK()
{
    if(IsLostPosition() < 0)
    {
        printf("No change position values:---->x=%d,y=%d,angle=%f\n",\
               lidar->xPos,lidar->yPos,lidar->angle);
        StopCar(true);
        SendUrgentEvent(evLostPosition,0,NULL);
    }
    if (true == lidar->IsHaveError())
    {
        //printf("Lidar mode error or have error code:---->mode=%d,error code=%d\n",\
        //       lidar->mode,lidar->errorCode);
        //StopCar(true);
        //SendUrgentEvent(evLostPosition, lidar->errorCode, NULL);
    }

    return 0;
}


int CAgvCarControler::SaveLastPathMag(int station, int mode, int speed, int xpos,\
                    int ypos, float angle, int radius, int action, \
                    int liftheight)
{
    printf("***** Save last path message******\n");
    lastPathMag.station = station;
    lastPathMag.mode    = mode;
    lastPathMag.speed   = speed;
    lastPathMag.xPos    = xpos;
    lastPathMag.yPos    = ypos;
    lastPathMag.angle   = angle;
    lastPathMag.radius  = radius;
    lastPathMag.action  = action;
    lastPathMag.liftHeight = liftheight;
    return 0;
}

int CAgvCarControler::CleanTargetStation()
{
    if (moveHelper == NULL)
    {
        return -1;
    }
    return moveHelper->ClearTarget();
}

int CAgvCarControler::SetTargetStation(int station, int mode, int speed, int xpos,\
                                       int ypos, float angle, int radius, int action, \
                                       int liftheight)
{
    LogInfo(LOGTAG,"Start execute WayPoint[%d] mode:%d speed:%d (%d,%d) angle:%f radius:%d action:%d",\
            station, mode, speed, xpos, ypos, angle,\
            radius, action, liftheight);

    if (moveHelper == NULL)
    {
        return -1;
    }

    SaveLastPathMag(station,mode,speed,xpos,ypos,\
                   angle,radius,action,liftheight);


    return moveHelper->SetTarget(station, mode, speed, xpos, ypos,\
                                 angle, radius, action, liftheight);
}

int CAgvCarControler::CleanLastPathMag()
{
    SaveLastPathMag(0,0,0,0,0,\
                   0,0,0,0);
    return 0;
}

int CAgvCarControler::ResetLastTargetStation()
{
    printf("----> lastPathMag.station = %d\n",lastPathMag.station);
    if (moveHelper == NULL || lastPathMag.station == 0)
    {
        return -1;
    }

#if UpAndRunFlag
    //2018-03-26
    if(sourcePathMag.action != 4 && sourcePathMag.liftHeight>=0)
    {
        if( 0 < lastPathMag.liftHeight && lastPathMag.liftHeight < 0xBB8)//0-3000mm
        {
          LiftUp(lastPathMag.liftHeight);
        }
        else if(0 == lastPathMag.liftHeight)
        {
           LiftDown();
        }
    }

#endif


    return moveHelper->SetTarget(lastPathMag.station, lastPathMag.mode, lastPathMag.speed,\
                                 lastPathMag.xPos,lastPathMag.yPos, lastPathMag.angle, \
                                 lastPathMag.radius, lastPathMag.action, lastPathMag.action);
}

void CAgvCarControler::Run()
{
    while(runnable)
    {
        if (operateMode == ManualMode || operateMode == NoKownMode)
        {
            usleep(10 * 1000);
            continue;
        }

        if(GoForwardMode == moveHelper->GetRunningMode())
        {
            moveHelper->SetBarrierStatus(barriers->GetBarrierError(CAgvBarrierSensor::Front));
        }
        else if(GoBackwardMode == moveHelper->GetRunningMode())
        {
            moveHelper->SetBarrierStatus(CAgvBarrierSensor::None);
        }


        moveHelper->LoopRunning();

        if(moveHelper->GetRunningMode() == StopMode && CarIsRunnging())
        {
            StopCar(false);
        }
    }
}


int CAgvCarControler::GetLaserStuta()
{
    if (barriers == NULL)
    {
        return false;
    }
    return barriers->GetBarrierError(CAgvBarrierSensor::Front);
}



