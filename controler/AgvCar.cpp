#include "AgvCar.h"

#define  LOGTAG  "AgvCar"

CAgvCar::CAgvCar() : IAgvEventListener()
{
    agvLogs = &CAgvLogs::Instance();
    setting = &CAgvSetting::Instance();
    memset(&carInfo, 0, sizeof(CarInfo));

    liftHeight = 0;
    batteryVoltage = 0;
    carElectricity = 0;
    operateMode = NoKownMode;
    reportStatus = CAgvScheduler::Starting;
    currentPathId = 0;
    carRunMode = 0;
    currentPathAction = 0;
    batteryCapacity = 0;
    StopButton = true;
    printf("---->CAgvCar:StopButton = true\n");
}

CAgvCar::~CAgvCar()
{
    Destroy();
}

int CAgvCar::Init()
{
    carInfo.XPos   = 0;
    carInfo.YPos   = 0;
    carInfo.Angle  = 0;
    carInfo.Speed  = 0;
    speedFormServer = 0;
    carInfo.Status = StatusInit;
    carInfo.Error  = NoError;
    SetErrorFlag(ErrorNetWork);
    SetErrorFlag(ErrorPosition);

    DEBUG_INFO("AgvCar", "Start init controler...");
    if (0 != controler.Init())
    {
        LogError(LOGTAG, "Init Can operator failed!");
        return -1;
    }
    controler.SetAgvSleepSwitch(false);
    controler.SetVoiceVolume(setting->Volume);//2018-03-26
    CAgvEventHelper::Instance().RegisterListener(this);

    return 0;
}

int CAgvCar::Destroy()
{
    StopCar(true);
    router.Stop();
    controler.Close();
    CAgvEventHelper::Instance().DeRegisterListener(this);

    return 0;
}

static const char *StatusString[] =
{
    "StatusInit",
    "StatusNormal",
    "StatusRunning",
    "StatusStopped",
    "StatusTurningLeft",
    "StatusTurningRight",
    "StatusError",
    "StatusWarning",
    "StatusUpdateFirmware"
};

static const char *ReportStatusString[] =
{
    "Running",
    "Stopped",
    "BreakDown",
    "Obstacle",
    "Derailed",
    "",
    "MainTainMode",
    "",
    "ManualMode",
    "LowPower",
    "Starting",
    "Idle",
    "Await",
    "",
    "",
    "",
    "",
    ""
};

#define BLOCK_LOG_FILE " BLCOK_LOG.txt"



int CAgvCar::StartRun()
{
    int ret = controler.SetEnable(true);
    if(AutoMode == operateMode) //自动模式下按路线走
    {
        if (router.PathCount())
        {
            if (0 == router.TargetPathId()) //开始执行路线
            {
                if (GoToNextPoint(true))
                {
                    LogError(LOGTAG, "Start first Way Point failed!");
                    SendImportantEvent(evPathMagError, 0, NULL);
                    return 1;
                }
                LogInfo(LOGTAG, "Start to execute first way point...");
            }
            else //恢复执行路线
            {
                LogInfo(LOGTAG, "Resume execute last way point...");
                controler.ResetLastTargetStation();
            }
            StopButton = false;
            LogInfo(LOGTAG, "---->StartRun:StopButton = false\n");
            SetCarStatus(StatusRunning,carRunMode);
        }
        else //无路线时先向服务器请求
        {
            LogInfo(LOGTAG, "Not path in local , send a request to scheduler...");
            ret = router.RequestNewPath();
            return ret;
        }
    }

    int barrier = controler.GetLaserStuta();

        if(barrier == CAgvBarrierSensor::None)
        {
            ClearErrorFlag(ErrorIrMid);
            ClearErrorFlag(ErrorIrFar);
            ClearErrorFlag(ErrorIrNear);
            //ClearErrorFlag(ErrorTouch1);
        }
        else
        {
            if(IsHaveError(ErrorIrNear))
            {
                SendUrgentEvent(evNearBarrier, 0, NULL);
            }
            if(IsHaveError(ErrorIrMid))
            {
                SendUrgentEvent(evMidBarrier, 0, NULL);
            }
            if(IsHaveError(ErrorIrFar))
            {
                SendUrgentEvent(evFarBarrier, 0, NULL);
            }
            if(IsHaveError(ErrorTouch1))
            {
               // SendUrgentEvent(evBarrierTouched, 0, NULL);
            }
        }
    return ret;
}

int CAgvCar::StopCar(bool all)
{
    StopButton = true;
    LogInfo(LOGTAG, "---->StopCar:StopButton = true\n");

    SetCarStatus(StatusStopped, CAgvVoicePlayer::MP3_MUTE);

    controler.LiftStop();//2018-03-26

    if (all)
    {
        controler.SetEnable(false);
    }
    return 0;
}

int CAgvCar::SetLiftHeight(int height)
{
    int ret;
    if( 0 < height && height < 0xBB8)//0-3000mm
    {
      ret = controler.LiftUp(height);
    }
    else if(0 == height)
    {
        ret = controler.LiftDown();
    }
    return ret;
}

void CAgvCar::OnLiftAtTop()
{
    LogInfo(LOGTAG, "Now Lift is at top side! action:%d\n", currentPathAction);
    controler.PlayVoice(CAgvVoicePlayer::MP3_TOP, 1);

    if (currentPathAction == ActionLiftUpDown)
    {
        OnStationActionFinished();
    }
}

void CAgvCar::OnLiftAtBottom()
{
    LogInfo(LOGTAG, "Now Lift is at bottom side! action:%d\n", currentPathAction);
    controler.PlayVoice(CAgvVoicePlayer::MP3_BOTTOM, 1);

    if (currentPathAction == ActionLiftUpDown)
    {
        OnStationActionFinished();
    }
}

int  CAgvCar::OnBarrierMessage(int event, int dev)
{
    int type = (dev & 0xF0) >> 4;
    int id   = dev & 0x0F;
    int pos  = (dev & 0xFF00) >> 8;
    static const char *posStr[] =
    {
         "UNKOWN",
         "FRONT",
         "BACK",
         "LEFT",
         "RIGHT"
    };

    if (pos > 0)
    {
        if (controler.GetRunningMode() == GoForwardMode && pos != CAgvBarrierSensor::Front)
        {
            return 1;
        }
        if (controler.GetRunningMode() == GoBackwardMode && pos != CAgvBarrierSensor::Back)
        {
            return 1;
        }
        if (controler.GetRunningMode() == RotateLeftMode || controler.GetRunningMode() == RotateRightMode)
        {
            return 1;
        }
    }

    if (event == evBarrierOk)
    {
        ClearErrorFlag(ErrorIrFar);
        ClearErrorFlag(ErrorIrMid);
        ClearErrorFlag(ErrorIrNear);
        for (int i = 0; i < 4; i++)
        {
            ClearErrorFlag(ErrorTouch1 + i);
        }
    }

    if(event == evBarrierTouched)
    {
        SetErrorFlag(ErrorTouch1);
    }

    if(event == evNearBarrier)
    {
        SetErrorFlag(ErrorIrNear);
        ClearErrorFlag(ErrorIrMid);
        ClearErrorFlag(ErrorIrFar);
    }

    if(event == evMidBarrier)
    {
        if(IsHaveError(ErrorIrNear))
        {
            return 1;
        }
        SetErrorFlag(ErrorIrMid);
        ClearErrorFlag(ErrorIrFar);
        ClearErrorFlag(ErrorIrNear);
    }

    if(event == evFarBarrier)
    {
        if(IsHaveError(ErrorIrNear) || IsHaveError(ErrorIrMid))
        {
            return 1;
        }
        SetErrorFlag(ErrorIrFar);
        ClearErrorFlag(ErrorIrMid);
        ClearErrorFlag(ErrorIrNear);
    }


    static int lastEvent = evBarrierOk;

    if(StopButton == true)
    {
        lastEvent =  event;
        return 0;
    }

    printf("lastEvent = %d,event = %d\n",lastEvent,event);

    if((lastEvent == evNearBarrier || lastEvent == evBarrierTouched) && \
            (event != evNearBarrier && event != evBarrierTouched))
    {
        printf("*************Re_Set Path Massage*******************\n");
        controler.SetEnable(true);
        controler.ResetLastTargetStation();
    }

    lastEvent =  event;
    switch(event)
    {
        case evBarrierOk:
            LogInfo(LOGTAG, "All barrier is clear!");
            SetCarStatus(lastCarInfo.Status, lastCarInfo.voice);
            controler.ShowCurrentCarState(CarState_NoObstacle);
            break;

        case evNearBarrier:
            LogWarn(LOGTAG, "At %s side, Found Near barrier, source:%d-%d",posStr[pos], type, id);
            SetCarStatus(StatusError, CAgvVoicePlayer::MP3_NEAR);
            controler.ShowCurrentCarState(CarState_AheadNearObstacle);
            break;

        case evMidBarrier:
            LogWarn(LOGTAG, "At %s side, Found Middle barrier, source:%d-%d",posStr[pos], type, id);
            SetCarStatus(StatusWarning, CAgvVoicePlayer::MP3_MID);
            controler.ShowCurrentCarState(CarState_AheadMiddleObstacle);
            break;

        case evFarBarrier:
            LogWarn(LOGTAG, "At %s side, Found Far barrier, source:%d-%d",posStr[pos], type, id);
            SetCarStatus(StatusWarning, CAgvVoicePlayer::MP3_FAR);
            controler.ShowCurrentCarState(CarState_AheadFarObstacle);
            break;

        case evBarrierTouched:
            LogError(LOGTAG, "At %s side, Barrier [%d-%d] was triged!",posStr[pos], type, id);

            SetCarStatus(StatusError, CAgvVoicePlayer::MP3_TOUCH);
            controler.ShowCurrentCarState(CarState_AheadTentaclesObstacle);
            break;
        default:
            break;
    }

    return 0;
}

int  CAgvCar::OnBatteryMessage(int vol,int cap,int ele)
{
    batteryVoltage  = vol;
    batteryCapacity = cap;
    carElectricity  = ele;
    /*
    printf("batteryVoltage = %d,batteryCapacity = %d,carElectricity = %d\n",\
           batteryVoltage,batteryCapacity,carElectricity);
    */
    return 0;
}

int CAgvCar::OnStationMessage(int x, int y, int angle, bool derail)
{
    carInfo.XPos = x;
    carInfo.YPos = y;
    carInfo.Angle = angle;


    carInfo.Speed = (( controler.GetTargetSpeed() * 0.075 * 0.722 )/ 60)*100;
    router.UpdateCarInfo(carInfo.XPos, carInfo.YPos, carInfo.Angle,
                         carInfo.Speed,
                         batteryCapacity, reportStatus);

    return 0;
}


int CAgvCar::SetErrorFlag(int flag)
{
    if(flag < 0)
    {
        return -1;
    }
    if (!IsHaveError(flag))
    {
        carInfo.Error |= (1 << flag);
    }
    return 0;
}

int CAgvCar::ClearErrorFlag(int flag)
{
    if(flag < 0)
    {
        return -1;
    }
    if (IsHaveError(flag))
    {
        carInfo.Error &= ~(1 << flag);
    }
    return 0;
}

bool CAgvCar::IsHaveError(int flag)
{
    if (0 == (carInfo.Error & (1 << flag)))
    {
        return false;
    }
    return true;
}

int CAgvCar::SetCarStatus(int status, int para)
{
    static bool fristSetStatus = true;
    if (status == StatusInit || status == StatusError || status == StatusLowPower
        || status != carInfo.Status || status == StatusRunning)
    {
        static int last_status = -100;
        if (last_status != status)
        {
            last_status = status;
            LogInfo(LOGTAG, "Change status to %s ...\n",StatusString[status + 1]);
        }

        switch (status)
        {
            case StatusInit:
                //controler.SetVoiceVolume(setting->Volume);//2018-03-26
                //usleep(50*1000);//2018-03-26
                controler.PlayVoice(CAgvVoicePlayer::MP3_WELCOM, 1);
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::FlashNormal,
                                      CAgvRgbLight::Off);
                fristSetStatus = false;
                reportStatus = CAgvScheduler::Idle;
                break;

            case StatusNormal:
                if (para == 0)
                {
                    controler.PlayVoice(CAgvVoicePlayer::MP3_MUTE, 1);
                }
                else
                {
                    controler.PlayVoice(CAgvVoicePlayer::MP3_NORMAL, 3);
                }
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::On);
                reportStatus = CAgvScheduler::Idle;
                break;

            case StatusRunning:
                if(GoBackwardMode == para)
                {
                    controler.PlayVoice(CAgvVoicePlayer::MP3_BACKING_AGV, 0xff);//2018-03-26
                }
                else
                {
                    controler.PlayVoice(CAgvVoicePlayer::MP3_RUN, 0xff);
                }
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::On,
                                      CAgvRgbLight::Off);

                reportStatus = CAgvScheduler::Running;
                break;

            case StatusStopped:
                controler.PlayVoice(CAgvVoicePlayer::MP3_MUTE, 1);
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::On);

                controler.StopCar(false);

                reportStatus = CAgvScheduler::Stopped;
                break;

            case StatusTurningLeft:
                controler.PlayVoice(CAgvVoicePlayer::MP3_TURN, 1);
                controler.SetRGBLight(CAgvRgbLight::Left, CAgvRgbLight::On,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::Off);
                controler.SetRGBLight(CAgvRgbLight::Right, CAgvRgbLight::Off,
                                      CAgvRgbLight::On,
                                      CAgvRgbLight::Off);
                break;

            case StatusTurningRight:
                controler.PlayVoice(CAgvVoicePlayer::MP3_TURN, 1);
                controler.SetRGBLight(CAgvRgbLight::Left, CAgvRgbLight::Off,
                                      CAgvRgbLight::On,
                                      CAgvRgbLight::Off);
                controler.SetRGBLight(CAgvRgbLight::Right, CAgvRgbLight::On,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::Off);

                break;

            case StatusUpdateFirmware:

                //            controler.StopCar (true);
                controler.PlayVoice(CAgvVoicePlayer::MP3_MUTE, 1);
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::FlashBusy,
                                      CAgvRgbLight::FlashBusy,
                                      CAgvRgbLight::FlashBusy);

                reportStatus = CAgvScheduler::Stopped;
                break;

            case StatusError:
                controler.PlayVoice(para, 0xff);
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::On,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::Off);
                controler.StopCar(true);

                if(IsHaveError(ErrorPosition))
                {
                    reportStatus = CAgvScheduler::BreakDown;
                }
                else if (para == CAgvVoicePlayer::MP3_PATHMAGFAULT)
                {
                    reportStatus = CAgvScheduler::Derailed;
                }
                else if (IsHaveError(ErrorIrNear)
                         || IsHaveError(ErrorTouch1)
                         || IsHaveError(ErrorTouch2)
                         || IsHaveError(ErrorTouch3)
                         || IsHaveError(ErrorTouch4))
                {
                    reportStatus = CAgvScheduler::Obstacle;
                }
                else if (para > 0)
                {
                    reportStatus = CAgvScheduler::BreakDown;
                }
                break;

            case StatusWarning:
                if (para > 0)
                {
                    controler.PlayVoice(para, 3);
                }
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::FlashNormal,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::Off);
                break;

            case StatusLowPower:
                {
                    if(reportStatus == CAgvScheduler::Obstacle || \
                            reportStatus == CAgvScheduler::BreakDown)
                    {
                        break;
                    }
                    controler.PlayVoice (para, 0xff);
                    controler.SetRGBLight (CAgvRgbLight::BothSide,
                                           CAgvRgbLight::FlashNormal,
                                           CAgvRgbLight::Off,
                                           CAgvRgbLight::Off);
                    reportStatus = CAgvScheduler::LowPower;
                }
                break;

            default:
                break;
        }

    }

    if (status != StatusError && status != StatusWarning
        && status != StatusInit)
    {
        lastCarInfo.Status = status;
        lastCarInfo.voice = para;
    }

    carInfo.Status = status;

    if(fristSetStatus == true)
    {
        reportStatus = CAgvScheduler::Starting;
    }

    if (ManualMode == operateMode)
    {
        reportStatus = CAgvScheduler::ManualMode;
    }

    //LogInfo(LOGTAG, "Update RoutePlanner reportStatus:%s",ReportStatusString[reportStatus]);
    router.UpdateCarInfo(carInfo.XPos, carInfo.YPos, carInfo.Angle,
                         carInfo.Speed,
                         batteryCapacity, reportStatus);
//    {
//        static int cnt;
//        if ((cnt % 10) == 0)
//        {
//            cnt++;
//            LogInfo(LOGTAG, "Update RoutePlanner reportStatus:%s",
//                    ReportStatusString[reportStatus]);
//        }
//    }
    return 0;
}

void CAgvCar::OnArrivedStation(int id)
{
    LogInfo(LOGTAG, "Arrived WayPoint[%d]", id);

    if (DoPathPointAction() == 0)
    {
        OnStationActionFinished();
    }
}

int CAgvCar::DoPathPointAction()
{
    int ret = -1;
    WayPoint *path = router.GetTargetStation();
    currentPathAction = path->action;
    LogInfo(LOGTAG, "Execute WayPoint[%d]'s action:%d", path->id, path->action);

    switch (path->action)
    {
        case ActionRun:
            ret = 0;
            break;

        case ActionStop:
            StopCar(false);
            ret = 0;
            break;

        case ActionSpeedDown:
            ret = 0;
            break;

        case ActionResumeSpeed:
            ret = 0;
            break;

        case ActionLiftUpDown:
            LogInfo(LOGTAG, "Set lift height to %d", path->liftHeight);
            StopCar(false);
            if (controler.LiftIsArrived(path->liftHeight))
            {
                ret = 0;
            }
            else
            {
                SetLiftHeight(path->liftHeight);
            }
            break;

        case ActionCharge:
            LogInfo(LOGTAG, "Enable charge switch!");
            StopCar(false);
            controler.SetChargeSwitch(true);
            ret = 0;
            break;

        default:
            break;
    }

    return ret;
}

void CAgvCar::OnLiftArrivedHeight(int height)
{
    LogInfo(LOGTAG, "Now Lift arrived height %d! action:%d\n", height, currentPathAction);

    if (currentPathAction == ActionLiftUpDown)
    {
        OnStationActionFinished();
    }
}

void CAgvCar::OnStationActionFinished()
{
    if (router.OnActionFinished(controler.lastLiftHeight))
    {
        return;
    }
    currentPathAction = 0;
    int ret = GoToNextPoint(false);
    if (ret != 0)
    {
        if(1 == ret)
        {
            //到站
            LogInfo(LOGTAG, "Finish all WayPoint!");
            SendImportantEvent(evPathFinished, 0, NULL);
        }
        if(-1 == ret)
        {
            SendImportantEvent(evPathMagError, 0, NULL);
        }
    }
    else
    {
        LogInfo(LOGTAG, "Station[%d] action %d is finished! Prepare going to next point..",
                currentPathId, currentPathAction);
        StopButton = false;
        LogInfo(LOGTAG, "---->OnStationActionFinished:StopButton = false\n");
        SetCarStatus(StatusRunning, carRunMode);

        if(IsHaveError(ErrorIrNear))
        {
            SendUrgentEvent(evNearBarrier, 0, NULL);
        }
        if(IsHaveError(ErrorIrMid))
        {
            SendUrgentEvent(evMidBarrier, 0, NULL);
        }
        if(IsHaveError(ErrorIrFar))
        {
            SendUrgentEvent(evFarBarrier, 0, NULL);
        }
        if(IsHaveError(ErrorTouch1))
        {
           // SendUrgentEvent(evBarrierTouched, 0, NULL);
        }
    }
}

int CAgvCar::GetRunAction(WayPoint *curPoint)
{
//    WayPoint *lastPoint= router.GetLastStation();

//    if(lastPoint == NULL)
//    {
//        return curPoint->action;
//    }

    int speedDownFlag = 1;
    if(router.MoveToNextStation() == false)
    {
        return curPoint->action;//nextPoint = NULL
    }
    WayPoint *nextPoint = router.GetTargetStation();
    router.MoveToLastStation();

    POSE currentpose = CAgvMathUtils::getPoseFrom_x_y_alpha(curPoint->xPos,curPoint->yPos,curPoint->angle);
    POSE nextpose = CAgvMathUtils::getPoseFrom_x_y_alpha(nextPoint->xPos,nextPoint->yPos,nextPoint->angle);

    currentpose = CAgvMathUtils::transformPose(currentpose, -setting->LidarToTailLength);
    nextpose = CAgvMathUtils::transformPose(nextpose, -setting->LidarToTailLength);

    currentpose = CAgvMathUtils::transformCoordinate(nextpose, currentpose);
    currentpose.angle = CAgvMathUtils::transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

    if(fabs(currentpose.angle)<=8 && abs(currentpose.yPos)<= 80 && abs(currentpose.xPos)>= 3000
            && curPoint->runMode == nextPoint->runMode)//mm
    {
        speedDownFlag = 0;
        printf("Next point is almost in the same line, current point don't need speed down.\n");
    }
    else
    {
        speedDownFlag = 1;//default-set value;
    }

    if((ActionRun == curPoint->action && 1 ==speedDownFlag) || ActionCharge == nextPoint->action)
    {
        curPoint->action = ActionSpeedDown;
    }

    if(ActionLiftUpDown == nextPoint->action)
    {
        curPoint->action = ActionStop;
    }

    return curPoint->action;
}

int CAgvCar::WhetherJumpCurPoint(WayPoint *curPoint)
{
    int JumpCurPointFlag = 0;
    WayPoint *CurPoint = router.GetTargetStation();
    if(router.MoveToNextStation() == false)
    {
        JumpCurPointFlag = 0;
        return JumpCurPointFlag;//nextPoint = NULL
    }
    WayPoint *nextPoint = router.GetTargetStation();
    router.MoveToLastStation();

    if( ActionCharge == nextPoint->action || CurPoint->runMode != nextPoint->runMode
            || (ActionLiftUpDown == nextPoint->action && GoBackwardMode == nextPoint->runMode))
    {   //at this situation, mustn't jump the point;
        JumpCurPointFlag = 0;
        return JumpCurPointFlag;
    }

    POSE currentpose = CAgvMathUtils::getPoseFrom_x_y_alpha(curPoint->xPos,curPoint->yPos,curPoint->angle);
    POSE nextpose = CAgvMathUtils::getPoseFrom_x_y_alpha(nextPoint->xPos,nextPoint->yPos,nextPoint->angle);

    currentpose = CAgvMathUtils::transformPose(currentpose, -setting->LidarToTailLength);
    nextpose = CAgvMathUtils::transformPose(nextpose, -setting->LidarToTailLength);

    currentpose = CAgvMathUtils::transformCoordinate(nextpose, currentpose);
    currentpose.angle = CAgvMathUtils::transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

    if(fabs(currentpose.angle)< 3 && abs(currentpose.yPos)< 50 && ActionLiftUpDown != curPoint->action)//mm
    {
        JumpCurPointFlag = 1;
        printf("Next point is in the same straight line, jump to next point.\n");
    }
    else
    {
        JumpCurPointFlag = 0;//default-set value;
    }       

    return JumpCurPointFlag;
}




int CAgvCar::GoToNextPoint(bool isfirst)
{
    if(!isfirst && !router.MoveToNextStation())
    {
        return 1;
    }    
    else//add--0321--wrc
    {
        WayPoint *CurTargetPoint = router.GetTargetStation();
        int JumpCurPointFlag;
        int CurPointliftHeight;
        JumpCurPointFlag = WhetherJumpCurPoint(CurTargetPoint);
        CurPointliftHeight = CurTargetPoint->liftHeight;
        if(1 == JumpCurPointFlag && !isfirst) //the first point can't be jumped
        {
            router.MoveToNextStation();
            WayPoint *NextTargetPoint = router.GetTargetStation();
            int NextPointliftHeight = NextTargetPoint->liftHeight;

            if( CurPointliftHeight >= 0 && NextPointliftHeight >= 0 && CurPointliftHeight != NextPointliftHeight)
            {   //at this situation, don't jump the point;
                router.MoveToLastStation();
            }
            else if(CurPointliftHeight >= 0 && NextPointliftHeight < 0)
            {
                //Height transfer to next point to be done
                NextTargetPoint->liftHeight = CurPointliftHeight;
            }
        }
    }

    int speed;
    WayPoint *path = router.GetTargetStation(); 

    currentPathId = path->id;
    carRunMode = path->runMode;

    if(path->action == 0)
    {
        if(router.MoveToNextStation())
        {
            WayPoint *nextPath = router.GetTargetStation();
            if(nextPath->runMode != path->runMode)
            {
                path->action = 1;
            }
            router.MoveToLastStation();
        }
        else
        {
            path->action = 1;
        }
    }

    if (path->runMode == GoForwardMode)
    {
        if(0 == speedFormServer)
        {
            speed = setting->ForwardSpeed;
        }
        else
        {
            speed = speedFormServer;
        }
    }
    else if (path->runMode == GoBackwardMode)
    {

        speed = setting->BackwardSpeed;
    }
    else
    {
        speed = 0;
    }

    controler.SetTargetHeight(path->liftHeight);

    if ((path->action != ActionLiftUpDown && path->liftHeight >= 0 ) ||
            controler.CompareSourcePathMag(path->id,path->action,path->liftHeight))
    {
        controler.SaveSourcePathMag(path->id,path->action,path->liftHeight);

        if(!controler.LiftIsArrived(path->liftHeight))
        {
            SetLiftHeight(path->liftHeight);
        }
        path->action = ActionLiftUpDown;

    }






    controler.ShowCurrentCarState(CarState_StartExecutionNextPoint);



    path->action = GetRunAction(path);

    float pathGetNewAngle =  path->angle;

    if((1 <= path->runMode && path->runMode <= 2) || (5 <= path->runMode && path->runMode <= 6))
    {
        if((path->runMode == 5 || path->runMode == 6) && (path->angle < 0))
        {
            printf("---> Angle error! path->runMode = %d,path->angle = %f\n",path->runMode,path->angle);
            return -1;
        }

        if(path->runMode == 5)
        {
            pathGetNewAngle = path->angle * (-1);
        }

        return controler.SetTargetStation(path->id, path->runMode, speed, path->xPos, path->yPos,
                                          pathGetNewAngle,
                                          path->radius, path->action, path->liftHeight);
    }
    else
    {
        printf("--->  path->runMode = %d\n",path->runMode);
        return -1;
    }
}




bool CAgvCar::HandleEvent(CAgvEvent *evt)
{
    static bool isInitComplete = false;
    static bool isLifterInit = false;//2018-03-26

    if (evt == NULL)
    {
        return true;
    }

    if (evt->event > evUser)
    {
        //DEBUG_INFO("AgvCar", "Handle event:%s", evt->name.c_str());
    }

    bool ret = false;
    switch (evt->event)
    {
        case ev10msTimer:
            {
                if(true == isInitComplete)
                {
                   controler.IsLidarOK();
                }

                static int count = 0;
                if(count++ > 10)
                {
                    count = 0;
                    controler.AutoSetBarrierDistance();
                }
            }
            break;

        case ev1sTimer:
            {
                {   static int showBatTimer = 0;
                    showBatTimer++;
                    if(showBatTimer > 5)
                    {
                        showBatTimer = 0;
                        controler.ShowCurrentBatteryCapacity(batteryCapacity);
                    }
                    static int lowBatteryAlarmTimes = 0;
                    if ((batteryVoltage <= setting->alarmVoltage || batteryCapacity <= 15) && batteryVoltage > 0)
                    {
                        lowBatteryAlarmTimes++;
                        if (lowBatteryAlarmTimes > 20)
                        {
                            lowBatteryAlarmTimes = 0;
                            controler.ShowCurrentCarState(CarState_LowPower);
                            SetCarStatus(StatusLowPower, CAgvVoicePlayer::MP3_BATTERYLOW);
                        }
                    }
                    else
                    {
                        lowBatteryAlarmTimes = 0;
                    }
                }

                {
                    bool isNetworkOk = IsHaveError(ErrorNetWork);
                    bool isHavePosition = IsHaveError(ErrorPosition);

                    if (!isInitComplete && !isNetworkOk && !isHavePosition && isLifterInit && \
                            (operateMode != NoKownMode))
                    {
                        isInitComplete = true;
                        SendEvent(evSysReady, 0, NULL);
                    }
                }

                {
                    static int RequestSynCount = 58;
                    if(RequestSynCount++ > 60)
                    {
                        RequestSynCount = 0;
                        router.RequestSystemTimeSynchronization();
                    }
                }
            }
            break;

        case evSysReady:
            {
                LogInfo(LOGTAG, "Now system is ready!");
                SetCarStatus(StatusInit,NULL);
            }
            break;

        case evBattery:
            {
                BATTERYMAG *batteryMag = (BATTERYMAG *)(evt->data);
                OnBatteryMessage(batteryMag->vol,batteryMag->cap,batteryMag->ele);
            }
            break;

        case evNetworkOK:
            {
                LogInfo(LOGTAG, "Network is ready now!");
                if(IsHaveError(ErrorNetWork))
                {
                   ClearErrorFlag(ErrorNetWork);
                }
                controler.ShowCurrentCarState(CarState_ConnectionNetNormal);
                router.Start();
            }
            break;

        case evNetworkError:
            {
                LogError(LOGTAG, "Network disconnected!");
                SetErrorFlag(ErrorNetWork);
                controler.ShowCurrentCarState(CarState_ConnectionNetError);
                router.Stop();
            }
            break;

        case evSchedulerOnline:
            {
                controler.ShowCurrentNetState(1);
                controler.ShowCurrentCarState(CarState_ConnectionServerNormal);
                LogInfo(LOGTAG, "Scheduler is online now!");
            }
            break;

        case evSchedulerOffline:
            {
                controler.ShowCurrentNetState(0);
                controler.ShowCurrentCarState(CarState_ConnectionServerError);
                LogWarn(LOGTAG, "Scheduler is offline!");
                if(setting->IsStopCarForScheOffLine)
                {
                    LogWarn(LOGTAG,"IsStopCarForScheOffLine = 1");
                    StopCar(true);
                }
            }
            break;

        case evPosition:
            {
                if(IsHaveError(ErrorPosition))
                {
                    printf("Clear Navigator Fault\n");
                    ClearErrorFlag(ErrorPosition);
                    controler.ShowCurrentCarState(CarState_NavigatorNormal);
                    //SetCarStatus(StatusNormal, CAgvVoicePlayer::MP3_MUTE);
                }

                if(evt->data != NULL)
                {
                    POSITION *pos = (POSITION *)(evt->data);
                    OnStationMessage(pos->xPos, pos->yPos, pos->angle * 100, false);
                    delete pos;
                    evt->data = NULL;
                }
            }
            break;

        case evLostPosition:
            {
                if(true == isInitComplete)
                {
                    printf("!!! Position information loss !!!\n");
                    SetErrorFlag(ErrorPosition);
                    SetCarStatus(StatusError, CAgvVoicePlayer::MP3_NAVIGATOR_FAULT);
                    controler.ShowCurrentCarState(CarState_NavigatorFault);
                }
            }
            break;

        case evSensorOnline:
            {
                if(IsHaveError(ErrorSensorLost))
                {
                    ClearErrorFlag(ErrorSensorLost);
                }
                controler.ShowCurrentCarState(CarState_SensorNormal);
            }
            break;

        case evSensorOffline:
            {
                SetErrorFlag(ErrorSensorLost);
                controler.ShowCurrentCarState(CarState_SensorLoss);
            }
            break;

        //2018-03-26
        case evLifterInit:
            {
                isLifterInit = true;
                controler.PlayVoice(CAgvVoicePlayer::MP3_MUTE, 1);
                controler.SetRGBLight(CAgvRgbLight::BothSide,
                                      CAgvRgbLight::Off,
                                      CAgvRgbLight::On,
                                      CAgvRgbLight::Off);
            }
            break;
        case evKeyAutoMode:
            {
                if (operateMode != AutoMode)
                {
                    LogInfo(LOGTAG, "Change mode to AutoMode");
                    operateMode = AutoMode;
                    controler.SetOperateMode(operateMode);
                    controler.ShowCurrentCarState(CarState_AutoMode);
                    SetCarStatus(StatusNormal, CAgvVoicePlayer::MP3_MUTE);
                    controler.StopCar(true);
                    controler.SetChargeSwitch(false);
                    router.ReloadPathList();

                    //2018-03-26
                    if(isLifterInit == false)
                    {
                        controler.LiftInit();

                        controler.PlayVoice(CAgvVoicePlayer::MP3_BOTTOM, 0xff);
                        controler.SetRGBLight(CAgvRgbLight::BothSide,
                                              CAgvRgbLight::FlashNormal,
                                              CAgvRgbLight::Off,
                                              CAgvRgbLight::Off);
                    }
                }
            }
            break;

        case evKeyManualMode:
            {
                if (operateMode != ManualMode)
                {
                    LogInfo(LOGTAG, "Change mode to ManualMode");
                    StopButton = true;
                    operateMode = ManualMode;
                    controler.SetOperateMode(operateMode);
                    controler.ShowCurrentCarState(CarState_ManualMode);
                    controler.SetChargeSwitch(false);
                    router.CleanLastStationPoint();
                    SetCarStatus(StatusStopped, CAgvVoicePlayer::MP3_MUTE);
                }
            }
            break;

        case evKeyStart:
            {
                if (AutoMode == operateMode)
                {
                    LogInfo(LOGTAG, "Start key pressed!");
                    StartRun();
                }
            }
            break;

        case evKeyStop:
            {
                if (AutoMode == operateMode)
                {
                    LogInfo(LOGTAG, "Stop Key pressed!");
                    StopCar(false);
                }
            }
            break;

        case evTurnOnCharge:
            {
                if(operateMode == ManualMode || evt->param == 1)
                {
                   LogInfo(LOGTAG, "Turn on the charge switch!\n");
                   controler.PlayVoice(CAgvVoicePlayer::MP3_MANUALCHAGERON, 1);
                   StopCar(true);
                   controler.SetChargeSwitch(true);
                }
            }
            break;

        case evTurnOffCharge:
            {
                if(operateMode == ManualMode || evt->param == 1)
                {
                   LogInfo(LOGTAG, "Turn off the charge switch!\n");
                   controler.PlayVoice(CAgvVoicePlayer::MP3_MANUALCHAGEROFF, 1);
                   StopCar(true);
                   controler.SetChargeSwitch(false);
                }
            }
            break;

       case evAgvSleep:
            {
                controler.SetAgvSleepSwitch(true);
            }
            break;

       case evAgvWakeUp:
            {
                controler.SetAgvSleepSwitch(false);
            }
            break;
        case evGetNewPath:
            {
                if (evt->data != NULL)
                {
                    vector<StationInfo *> list = *((vector<StationInfo *> *)evt->data);
                    if (router.OnGetNewPath(list) > 0)
                    {
                        SetCarStatus(StatusStopped, CAgvVoicePlayer::MP3_MUTE);
                        controler.ShowCurrentCarState(CarState_ReceiveNewPathMessage);
                    }
                }
            }
            break;

        case evApplyPath:
            {
                if (AutoMode == operateMode)
                {
                    LogInfo(LOGTAG, "Start execute new path....");
                    StartRun();
                }
            }
            break;

        case evNextPathPoint:
            {
                OnArrivedStation(router.GetTargetStation()->id);
            }
            break;

        case evPathMagError:
            {
                StopCar(false);
                if(evt->param == -5)
                {

                }
                else
                {
                    printf("Current path point message error\n");
                    controler.ShowCurrentCarState(CarState_PathMessageError);
                    SetCarStatus(StatusError, CAgvVoicePlayer::MP3_PATHMAGFAULT);
                }
            }
            break;

        case evPathFinished:
            {
                printf("Path Finished\n");
                StopCar(false);
                SetCarStatus(StatusNormal, CAgvVoicePlayer::MP3_NORMAL);
                router.ReloadPathList();
                LogInfo(LOGTAG, "Re-init the path data!!!");
                if (setting->IsAutoLoop)
                {
                    SendEvent(evKeyStart, 0, NULL);
                }
                controler.ShowCurrentCarState(CarState_PathExecutionComplete);    
            }
            break;

        case evLiftArrived:
            {
                OnLiftArrivedHeight(evt->param);

                if(1 == controler.GetLiftStatus())
                {
                    controler.ShowCurrentCarState(CarState_ForkArmToTheBottom);
                }
                else if(2 == controler.GetLiftStatus())
                {
                    controler.ShowCurrentCarState(CarState_ForkArmToTheTop);
                }
            }
            break;

        case evSetBarrierDistance:
            {
                controler.SetBarrierDistance(2,1,0,evt->param+9);
            }
            break;

        case evNearBarrier:
        case evMidBarrier:
        case evFarBarrier:
        case evBarrierTouched:
        case evBarrierOk:
            {
                if (operateMode == AutoMode)
                {
                    OnBarrierMessage(evt->event, evt->param);
                }
                break;
            }

        case evUrgentStop:
            {
                SetCarStatus(StatusError, CAgvVoicePlayer::MP3_URGENT);
                break;
            }

        case evSetSpeed:
            {
                speedFormServer = evt->param;
                break;
            }

        default:
            break;
    }

    return ret;
}

