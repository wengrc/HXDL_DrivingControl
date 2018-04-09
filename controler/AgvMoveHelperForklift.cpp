/*
 * AgvRunningHelperHuawei.cpp
 *
 *  Created on: 2017-6-14
 *      Author: crystal
 *
 */


#include "AgvCarControler.h"
#include "AgvRoutePlanner.h"
#include "AgvMoveHelperForklift.h"
#include "../devices/canbus/AgvSensor.h"
#include <float.h>
#include "devices/canbus/AgvSensor.h"

#define LOGTAG  "MoveHelperForklift"



CAgvMoveHelperForklift::CAgvMoveHelperForklift(CAgvBaseControler *ctrler) : CAgvMoveHelper("AlgoMoveHelper",ctrler)
{
    s_Controler =  CAgvControler::Instance();
    s_Lidar = CAgvLidarSensor::Instance();
    setting = &CAgvSetting::Instance();

    DELAY_ANGLE_OFFSET = setting->DelayAngleOffset;
    printf("xml---------> DELAY_ANGLE_OFFSET = %f\n",DELAY_ANGLE_OFFSET);
    DELAY_ARC_DISTANCE_OFFSET = setting->DelayArcDistanceOffset;
    printf("xml---------> DELAY_ARC_DISTANCE_OFFSET = %d\n",DELAY_ARC_DISTANCE_OFFSET);
    DELAY_DISTANCE_OFFSET = setting->DelayDistanceOffset;
    printf("xml---------> DELAY_DISTANCE_OFFSET = %d\n",DELAY_DISTANCE_OFFSET);
    STANDARD_RADIUS = setting->StandardRadius;
    SavedXMLSTANDARD_RADIUS = STANDARD_RADIUS;
    printf("xml---------> STANDARD_RADIUS = %d\n",STANDARD_RADIUS);
    ANGLE_OFFSET = setting->AngleOffset;
    printf("xml---------> ANGLE_OFFSET = %f\n",ANGLE_OFFSET);
    LIDAR_LEFT_RIGHT_OFFSET_MM = setting->LidarLeftRightOffsetMM;
    printf("xml---------> LIDAR_LEFT_RIGHT_OFFSET_MM = %d\n",LIDAR_LEFT_RIGHT_OFFSET_MM);
    wheelBase = setting->CarWheelBase;
    printf("xml---------> wheelBase = %d\n",wheelBase);
    LIDAR_TO_TAIL_LENGTH = setting->LidarToTailLength;
    printf("xml---------> LIDAR_TO_TAIL_LENGTH = %d\n",LIDAR_TO_TAIL_LENGTH);






    initParaFlag = false;
    haveBarrier = false;
    isChangeSpeedInit = false;
    isGlobalTargetSave = false;
    rotateMode = relative;
    agvMathUtils = new CAgvMathUtils;
    angle = 0.0;
    subpathSize = 0;
    clearSubpath();

    forwardHighSpeed = CAgvSetting::Instance().ForwardSpeed * 1000 / 60;
    forwardLowSpeed = forwardHighSpeed / 5;
    arriveSpeed = forwardLowSpeed / 5;
    backwardSpeed = CAgvSetting::Instance().BackwardSpeed * 1000 / 60;
    turningSpeed = CAgvSetting::Instance().TurningSpeed * 1000 / 60;
    turningLowSpeed = turningSpeed / 4;
    turningArriveSpeed = turningSpeed / 4;

    initSpeed = INIT_SPEED;
    targetSpeed = 0;

    currentLinearSpeed = 0;
    currentAngularSpeed = 0.0;
    baseAngularSpeed = 0.0;

    lastLinearSpeed = 0;
    lastAngularSpeed = 0.0;

    isCarGetStuck = false;
    isAvoidingBarrier = false;

    BarrierStatus = 0;
    CarRunningMode = 0;
    carID = controler->GetCAN_ID();
}

CAgvMoveHelperForklift::~CAgvMoveHelperForklift()
{
    Stop();
}

int CAgvMoveHelperForklift::Start()
{
    CAgvMoveHelper::Start();
    DEBUG_INFO(LOGTAG, "Start....");
    return 0;
}

int CAgvMoveHelperForklift::Stop()
{
    CAgvMoveHelper::Stop();
    DEBUG_INFO(LOGTAG, "MoveHelper Stop!");
    return 0;
}

/***************************************************************
 *
 * This function is used to calculate the precise wheel driver
 * gear reduction ratio. We use the distance of the agv runs and
 * the time it takes to run the distance to calculate the agv speed.
 * with the set target speed and the real agv speed, we can calculate
 * the reduction.
 *also, this function can use to fix the zero point of the steering angle.
 * It's very important to fix the zero point, because it will affect the
 * performance for the agv run forward line.
 *
 * */
int CAgvMoveHelperForklift::ForwardTest(POSE currentpose)
{
    static POSE lastPose = {0, 0 , 0.0};
    static int i = 0;

    int runDistance;

    if(false == initParaFlag)
    {
        currentLinearSpeed = BASE_LINEAR_SPEED+50;
        baseAngularSpeed = 0.0;
        currentAngularSpeed = 0.0;

         s_Controler->SetDirection(0);//0:forward, 1:backward.
         s_Controler->SetBreak(false);
          //s_Controler->SetSpeed(200);
          //s_Controler->SetAngle(3.41);//
        //controler->RunForward(currentLinearSpeed);
        controler->SetCarSpeed(currentLinearSpeed, currentAngularSpeed);
        //printf("speed:%d, %f\n", currentLinearSpeed, currentAngularSpeed);
        lastPose = currentpose;
        runDistance = 0;
        i = 0;

        initParaFlag = true;
        LogInfo(LOGTAG,"start run...");
    }
    else
    {
        runDistance = (int)agvMathUtils->getDistanceBetweenPose(currentpose, lastPose);
        static int count = 0;
        if(count++ > 30000)
        {
            count = 0;
            printf("current angle: %f, current speed: %d\n",   s_Controler->angle,   s_Controler->speed);
        }

        if(runDistance > DISTANCE_1000_MM)
        {
            LogInfo(LOGTAG, "run %d mm", runDistance);
            lastPose = currentpose;
            i++;
        }

        //control the two wheels when the control period comes
        if(i > 9)
        {
            LogInfo(LOGTAG, "finish run...");
            printf("%d mm ", runDistance);
            memset(&lastPose, 0, sizeof(lastPose));
            runningMode = StopMode;
            initParaFlag = false;

            controler->StopCar(false);
            usleep(600*1000);
            controler->SetBreak(true);

            return 1;
        }
    }

    return 0;
}

/*
 * This func is used to test the time it takes for the agv rotate.
 * It is easy to find that when the angle almost reaches, it takes
 * a lot of time for the steering to reach the angle.
 * */
int CAgvMoveHelperForklift::RotateTest(float angle)
{
    if(false == initParaFlag)
    {
        initParaFlag = true;
          s_Controler->SetAngle(angle);
        LogInfo(LOGTAG, "start rotate...");
    }
    else
    {
        static int count = 0;
        //if(count++ > 20)
        {
            count = 0;
            LogInfo(LOGTAG, "rotate angle: %f set angle: %f\n",   s_Controler->angle, angle);
        }

//        if(fabs(  s_Controler->angle - angle) < 0.2)
//        {
//            initParaFlag = false;
//            LogInfo(LOGTAG, "finish rotate...");
//            return 1;
//        }
    }
    return 0;
}

/****************************************************************************/
int CAgvMoveHelperForklift::SetTarget(int station, int mode, int speed, int xpos, int ypos, float angle, int radius, int action, int liftheight)
{
    printf("-------> runMode = %d,speed = %d<-----------\n",mode,speed);
    CarRunningMode = mode;
    MAXSPEEDALLOWED  = speed;

    POSE currentpose = GetCurrentPose();
    POSE targetpose = agvMathUtils->getPoseFrom_x_y_alpha(xpos, ypos, angle);

    cout<<endl<<endl;
    cout<<"CAgvMoveHelperForklift::SetTarget() ..."<<endl<<endl;
    cout<<"currentpos: ("<<currentpose.xPos<<", "
                         <<currentpose.yPos<<", "
                         <<currentpose.angle<<")"<<endl<<endl;
    clearSubpath();
    subpathSize = 0;

    static int count = 0;
    count++;
    if(0 == count%3)
    {
        LogInfo(LOGTAG, "The agv run task for %d times.", count/3);
    }

    //forward to line or backward to line
    if(GoForwardMode == mode || GoBackwardMode == mode)
    {
        int ErrorCode = SplitToSubTarget(currentpose, targetpose, (DIRECTION)mode);
        if(0 != ErrorCode)
        {
            printf("Path decomposition error,Errorcode = %d\n",ErrorCode);
            SendImportantEvent(evPathMagError,ErrorCode, NULL);
            return -1;
        }

        if( ActionStop == action || (ActionLiftUpDown == action && GoBackwardMode == mode) || ActionCharge == action)
        {
            targetPoint.needStop = true;
        }
        else
        {
            targetPoint.needStop = false;
        }
        rotateMode = relative;
    }
    else if(CarryGoods == mode)
    {
        AttitudeAdjustToFork(currentpose, targetpose);
        targetPoint.needStop = true;
        rotateMode = absolute;
    }
    else
    {
        subpathSize = 0;
        subpath[0].runmode = mode;
        subpath[0].target = targetpose;
        subpath[0].distance = 0;
        subpath[0].speed = INIT_SPEED;
        targetPoint.needStop = false;
        rotateMode = relative;
    }


    targetPoint.station = station;
    targetPoint.mode = mode;
    targetPoint.xPos = xpos;
    targetPoint.yPos = ypos;
    targetPoint.angle = angle;
    targetPoint.radius = radius;
    targetPoint.mode = mode;
    targetPoint.action = action;
    targetPoint.speed = speed;
    targetPoint.liftHeight = liftheight;

    pathIndex = 0;
    targetPose  = subpath[0].target;
    runningMode = subpath[0].runmode;
    targetSpeed = subpath[0].speed;

    if(absolute == rotateMode)
    {
        if(RotateLeftMode == runningMode || RotateRightMode == runningMode)
        {
            targetPose.angle = currentpose.angle - targetPose.angle;
            targetPose.angle = agvMathUtils->transformAngle(targetPose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);
            printf("target angle: %f, rotate mode: %d\n", targetPose.angle, rotateMode);
        }
    }

    LogInfo(LOGTAG, "SetTargetStation: [%d] mode:%d xpos:%d ypos:%d angle:%f target speed: %d",
            station, runningMode, targetPose.xPos, targetPose.yPos, targetPose.angle, targetSpeed);

    isRunning = true;
    initParaFlag = false;
    isChangeSpeedInit = false;

    return 0;
}

int CAgvMoveHelperForklift::ClearTarget()
{
    isRunning = false;

    runningMode = StopMode;
    initParaFlag = false;
    isChangeSpeedInit = false;

    CarRunningMode = StopMode;
    return 0;
}


int CAgvMoveHelperForklift::GetRunningMode()
{
    return CarRunningMode;
}

int CAgvMoveHelperForklift::SetBarrierStatus(int status)
{
    static int lastStaus = 0;

    if(lastStaus != status)
    {
        lastStaus = status;
        BarrierStatus = status;
        printf("******BarrierStatus = %d******\n",status);
    }
    return 0;
}

int CAgvMoveHelperForklift::LoopRunning()
{
    if(false == isRunning)
    {
        usleep(10 * 1000);
        return 0;
    }

    int  ret = 0;
    POSE currentPose = GetCurrentPose();

    switch(runningMode)
    {
        case StopMode:
            break;

        case GoForwardMode: //0x01
            ret = ForwardToLine(currentPose,targetPose);
            break;

        case GoBackwardMode://0x02
            ret = BackwardToLine(currentPose, targetPose);
            break;

        case ForwardToArcMode://0x07
            ret = ForwardToArc(currentPose, targetPose);
            break;

        case BackwardToArcMode://0x08
            ret = BackwardToArc(currentPose, targetPose);
            break;

        case RotateLeftMode://0x05
        case RotateRightMode://0x06
            ret = Rotate(targetPose.angle);

            //ret = RotateTest(targetPose.angle);
            //ret = ForwardTest(currentPose);

            break;

        default:
            break;
    }

    if(1 == ret)
    {
        currentPose = GetCurrentPose();//WRC
        printf("target pose: (%d %d %f) ", targetPose.xPos, targetPose.yPos, targetPose.angle);
        printf("current pose: (%d %d %f)\n", currentPose.xPos, currentPose.yPos, currentPose.angle);

        if(++pathIndex < subpathSize)
        {
            targetPose  = subpath[pathIndex].target;
            runningMode = subpath[pathIndex].runmode;
            targetSpeed = subpath[pathIndex].speed;

            if(absolute == rotateMode)
            {
                if(RotateLeftMode == runningMode || RotateRightMode == runningMode)
                {
                    targetPose.angle = currentPose.angle - targetPose.angle;
                    targetPose.angle = agvMathUtils->transformAngle(targetPose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);
                    printf("target angle: %f, rotate mode: %d\n", targetPose.angle, rotateMode);
                }
            }

            LogInfo(LOGTAG, "SetTargetStation: [%d] mode:%d xpos:%d ypos:%d angle:%f target speed: %d",
                    targetPoint.station, runningMode, targetPose.xPos, targetPose.yPos, targetPose.angle, targetSpeed);
        }
        else
        {
            if((ActionLiftUpDown == targetPoint.action && GoBackwardMode == runningMode) || ActionCharge == targetPoint.action)
            {
                controler->StopCar(true);
                usleep(10 * 1000);
            }
            else if (ActionStop == targetPoint.action && (RotateLeftMode != targetPoint.mode && RotateRightMode != targetPoint.mode))
            {
                //printf("runningMode = %d.\n",runningMode);
                controler->StopCar(false);
                printf("Before break, usleep(600 * 1000);\n");
                usleep(900 * 1000);//600*1000
                controler->SetBreak(true);
                usleep(400 * 1000);//400*1000
            }

            runningMode = StopMode;
            pathIndex = 0;
            subpathSize = 0;

            clearSubpath();
            SendImportantEvent(evNextPathPoint, targetPoint.station, NULL);
        }
    }
    else if(ret < 0)
     {
         printf("Error in running path process,ErrorCode = %d\n",ret);
         SendImportantEvent(evPathMagError,ret, NULL);
     }

    return 0;
}

#define ForwardInitDefaultSpeed 300//mm/s
#define GoChargeSpeed_mm_s 100
#define FinalDelayDefault_DISTANCE 40 //40 is OK
int CAgvMoveHelperForklift::ForwardToLine(POSE currentpose, POSE targetpose)
{
    static runMode mode = lowSpeedMode;
    static POSE lastPose = {0, 0 , 0.0};
    static bool arrivingFlag = false;
    static int savedTargetSpeed = 0;
    int distance;
    int runDistance;
    float angleDiff;

    POSE tempPose = currentpose;
    currentpose = agvMathUtils->transformPose(currentpose, -LIDAR_TO_TAIL_LENGTH);
    targetpose = agvMathUtils->transformPose(targetpose, -LIDAR_TO_TAIL_LENGTH);

    currentpose = agvMathUtils->transformCoordinate(targetpose, currentpose);
    currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

    if(false == initParaFlag)
    {
        printf("\n~~~~~~~\n~~~~~~~\n  ForwardToLine-start__startPose: (%d, %d, %f)\n", tempPose.xPos, tempPose.yPos, tempPose.angle);

        //check if the start point ahead of the target
        if(currentpose.xPos >= 0.0)
        {
            printf("the start pose is ahead of the target pose!!!!");
            return -1;
        }
        else
        {
            forwardSpeedInit(&savedTargetSpeed, &mode);

            int ret1 = s_Controler->SetDirection(0);
            int ret2 = s_Controler->SetBreak(false);
            int ret3 = controler->SetCarSpeed(currentLinearSpeed, currentAngularSpeed);
            if(ret1 != 0 || ret2 != 0 || ret3 != 0 )
            {
                printf("InitRun set failed!\n");
                initParaFlag = false;
            }
            else
            {
                printf("Init_speed:%d, %f\n", currentLinearSpeed, currentAngularSpeed);
                lastAngularSpeed = currentAngularSpeed;
                lastLinearSpeed = currentLinearSpeed;
                targetSpeed = currentLinearSpeed;
                aspeed = 0;
                lastPose = currentpose;
                runDistance = 0;
                arrivingFlag = false;

                initParaFlag = true;
            }
        }
    }
    else
    {
        int finalDelayDistance;
        int SpeedFinalFinal;
        float Kdistance;

        finalDelayDistance = calFinalDelayDis(&SpeedFinalFinal, &Kdistance);

        //check if the agv reach the target point
        if(currentpose.xPos >= (0 - finalDelayDistance))
        {
            //LogInfo(LOGTAG, "Currentpose.xPos:%d.",currentpose.xPos);            
            memset(&lastPose, 0, sizeof(lastPose));
            mode = lowSpeedMode;
            runningMode = StopMode;
            arrivingFlag = false;
            initParaFlag = false;
            isChangeSpeedInit = false;

            return 1;
        }
        else if( (currentpose.xPos >= (0 - DISTANCE_100_MM*5)) &&
                 (ActionCharge == targetPoint.action))
        {
            targetSpeed = (float)(abs(currentpose.xPos))/DISTANCE_100_MM/5 * BASE_LINEAR_SPEED + SpeedFinalFinal;
            currentLinearSpeed = targetSpeed;
            arrivingFlag = true;
            mode = lowSpeedMode;
        }
        else if(currentpose.xPos >= (0 - DISTANCE_100_MM*8)&&
                (ActionStop == targetPoint.action || ActionLiftUpDown == targetPoint.action))
        {
            targetSpeed = ( 2*(float)(abs(currentpose.xPos))/DISTANCE_1000_MM + 1.4 )*BASE_LINEAR_SPEED;
            currentLinearSpeed = targetSpeed;
            arrivingFlag = true;
            mode = lowSpeedMode;
        }

        //run forward following the line
        angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, 0.0);
        distance = (int)currentpose.yPos;

        if(false == arrivingFlag)
        {
            int refereceDistance = abs(DISTANCE_10_MM * currentLinearSpeed / BASE_LINEAR_SPEED * 26);
            if(currentpose.xPos > (0-refereceDistance))//when near the target, slow down
            {
                if(ActionCharge == targetPoint.action || ActionLiftUpDown == targetPoint.action
                        || ActionStop == targetPoint.action || ActionSpeedDown == targetPoint.action)
                {
                    targetSpeed = BASE_LINEAR_SPEED*2;

                    //wrc: calculate deceleration: control period is about 0.05s, so deceleration unit is about m/s/0.05s
                    dspeed = (int) ((currentLinearSpeed*currentLinearSpeed-targetSpeed*targetSpeed)/abs(2*currentpose.xPos));//uniformly retarded motion
                    dspeed = (int) (dspeed * 0.09);//0.15//change from 0.12--0320//0.10--0321
                }
                else //normal--if not need to speed down, comment out below lines.(wrc)
                {
                    //do nothing
                }
                arrivingFlag = true;
                mode = lowSpeedMode;
            }
            else
            {
                //when the distance and angle is small, change to high speed mode
                if(lowSpeedMode == mode &&
                        (fabs(angleDiff) <= ANGLE_1_DEGREE * 4 && abs(distance) <= DISTANCE_10_MM * 15)) //1, 2
                {
                    targetSpeed = savedTargetSpeed;
                    aspeed = calAspeed()*2;//change from 1 --0320
                    mode = highSpeedMode;
                }//change mode to low speed mode when the distance is to big, the agv is far away from the line
                else if(highSpeedMode == mode && abs(distance) >= DISTANCE_200_MM)
                {
                    targetSpeed = BASE_LINEAR_SPEED*2;
                    dspeed = calDspeed();
                    mode = lowSpeedMode;
                }
            }
        }

        runDistance = (int)agvMathUtils->getDistanceBetweenPose(currentpose, lastPose);
        //control the two wheels when the control period comes

        if(runDistance > DISTANCE_10_MM * currentLinearSpeed / BASE_LINEAR_SPEED /2)//wrc: control time period is about 0.05s
        {
            printf("\n*****************************************\nRunDistance:%d.  ", runDistance);
            ManageWheelSpeed();
            wheelControl(distance, angleDiff);
            lastPose = currentpose;
        }

    }

    return 0;
}

int CAgvMoveHelperForklift::StopFork(void)
{
    controler->StopCar(true);
    sleep(1);
    return 1;
}

#define backwardSpeedInit 150 //mm/s//150
#define backwardSpeedFinal 200 //120//200//180
#define backwardTargetSpeedInit 450
int CAgvMoveHelperForklift::BackwardToLine(POSE currentpose, POSE targetpose)
{
    static runMode mode = lowSpeedMode;

    static POSE lastPose = {0, 0 , 0.0};
    static bool arrivingFlag = false;
    //static int savedTargetSpeed = 0;

    int distance;
    int runDistance;


    float angleDiff;
    POSE tempPose = currentpose;

    currentpose = agvMathUtils->transformPose(currentpose, -LIDAR_TO_TAIL_LENGTH);
    targetpose = agvMathUtils->transformPose(targetpose, -LIDAR_TO_TAIL_LENGTH);

    currentpose = agvMathUtils->transformCoordinate(targetpose, currentpose);
    currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);


    if(false == initParaFlag)
    {
        printf("\n~~~~~~~\n~~~~~~~\n  BackwardToLine-start__startPose: (%d, %d, %f)\n", tempPose.xPos, tempPose.yPos, tempPose.angle);

        //check if the start point ahead of the target
        if(currentpose.xPos <= 0)
        {
            printf("the start pose is ahead of the target pose!!!!");
            return -1;
        }
        else
        {
            currentLinearSpeed = -backwardSpeedInit;
            //savedTargetSpeed = targetSpeed;
            targetSpeed = -backwardTargetSpeedInit;//20171225
            aspeed = 50;
            dspeed = 50;
            mode = lowSpeedMode;
            baseAngularSpeed = 0.0;
            currentAngularSpeed = 0.0;

            int ret1 = s_Controler->SetDirection(1);
            int ret2 = controler->SetBreak(false);
            int ret3 = controler->SetCarSpeed(currentLinearSpeed, currentAngularSpeed);

            if(ret1 != 0 || ret2 != 0 || ret3 != 0 )
            {
                printf("InitRun set failed!\n");
                initParaFlag = false;
            }
            else
            {
                printf("Init_speed:%d, %f\n", currentLinearSpeed, currentAngularSpeed);
                lastAngularSpeed = currentAngularSpeed;
                lastLinearSpeed = currentLinearSpeed;
                runDistance = 0;
                arrivingFlag = false;

                initParaFlag = true;
            }
        }
    }
    else
    {   int SpeedFinalFinal;
        float Kdistance;
        int finalDelayDistance;

        finalDelayDistance = calFinalDelayDis(&SpeedFinalFinal, &Kdistance);

        //check if the agv reach the target point
        if(currentpose.xPos <= (0 + finalDelayDistance))//20171225
        {
            printf("Backwardtoline arrive the station,finalDelayDistance:%d.\n",finalDelayDistance);
            printf("arrive the station\n");
            memset(&lastPose, 0, sizeof(lastPose));
            mode = lowSpeedMode;
            runningMode = StopMode;
            initParaFlag = false;
            isChangeSpeedInit = false;
            arrivingFlag = false;

            POSE tempPose = GetCurrentPose();//WRC
            printf("target pose: (%d %d %f) ", targetPose.xPos, targetPose.yPos, targetPose.angle);
            printf("current pose: (%d %d %f)\n", tempPose.xPos, tempPose.yPos, tempPose.angle);

            if (ActionLiftUpDown ==targetPoint.action)
            {
                controler->StopCar(true);
            }

            return 1;
        }
        else if( (currentpose.xPos <= DISTANCE_100_MM*5) && ActionLiftUpDown ==targetPoint.action)
        {            
            targetSpeed = -((float)(abs(currentpose.xPos))/DISTANCE_100_MM/5 * BASE_LINEAR_SPEED + SpeedFinalFinal);
            currentLinearSpeed = targetSpeed;
            arrivingFlag = true;
            mode = lowSpeedMode;
        }


        //run forward following the line
        angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, 0.0);
        distance = -(int)currentpose.yPos;

        if(false == arrivingFlag)
        {
            static int referenceDistance;
            referenceDistance = DISTANCE_1000_MM*2.2;//2.5

            if(currentpose.xPos <= (0 + referenceDistance) )//near target point, slow down to arrive
            {
                targetSpeed = -backwardSpeedFinal;
                aspeed = 15;//50
                dspeed = 15;//50
                arrivingFlag = true;
                mode = lowSpeedMode;
            }
        }

        runDistance = (int)agvMathUtils->getDistanceBetweenPose(currentpose, lastPose);
        //control the two wheels when the control period comes
        if(runDistance > DISTANCE_10_MM * abs(currentLinearSpeed) / BASE_LINEAR_SPEED/2)
        {
            printf("\n*****************************************\nRunDistance:%d.  ", runDistance);
            ManageWheelSpeed();
            wheelControl(distance, angleDiff);
            lastPose = currentpose;
        }

    }

    return 0;
}

#define RUNARC_SPPED_MMS 500
int CAgvMoveHelperForklift::ForwardToArc(POSE currentpose, POSE targetpose)
{
    static runMode mode = lowSpeedMode;
    static bool arrivingFlag = false;
    //static int savedTargetSpeed = 0;
    static POSE lastPose = {0, 0, 0.0};
    static PATHDIRECTION pathdirection = anticlockwise;

    static CIRCLE circle;
    float angleDiff;
    int distance;
    int runDistance;

    POSE tempPose = currentpose;

    currentpose = agvMathUtils->transformPose(currentpose, -LIDAR_TO_TAIL_LENGTH);
    targetpose = agvMathUtils->transformPose(targetpose, -LIDAR_TO_TAIL_LENGTH);
    //transform from global coordinate to relative coordinate
    currentpose = agvMathUtils->transformCoordinate(targetpose, currentpose);
    memset(&targetpose, 0, sizeof(POSE));

    if(false == initParaFlag)
    {
        printf("\n~~~~~~~\n~~~~~~~\n  ForwardToArc-start__startPose: (%d, %d, %f)\n", tempPose.xPos, tempPose.yPos, tempPose.angle);

        if(currentpose.xPos >= 0.0)
        {
            printf("start Pose error!!! current pose: (%d %d)\n", currentpose.xPos, currentpose.yPos);
            return -1;
        }
        else
        {
            circle = agvMathUtils->getCircleCenter(currentpose, targetpose);
            if((circle.center.x > currentpose.xPos && circle.center.y < 0.0)
                    || (circle.center.x < currentpose.xPos && circle.center.y > 0.0))
            {
                pathdirection = clockwise;
            }
            else
            {
                pathdirection = anticlockwise;
            }

            //set linear speed and angular speed
            currentLinearSpeed = initSpeed+50;
            targetSpeed = RUNARC_SPPED_MMS;
            //savedTargetSpeed = targetSpeed;
            aspeed = calAspeed()*2;
            int radius = sqrt(circle.radius * circle.radius + wheelBase * wheelBase);
            baseAngularSpeed = (float)currentLinearSpeed / radius;
            // left vel > right vel
            if(clockwise == pathdirection)
            {
                baseAngularSpeed = -baseAngularSpeed;
            }
            else
            {
            }
            printf("base angular speed:%f\n", baseAngularSpeed);

            currentAngularSpeed = baseAngularSpeed;

            int ret1 = s_Controler->SetDirection(0);
            int ret2 = s_Controler->SetBreak(false);
            int ret3 = controler->SetCarSpeed(currentLinearSpeed, currentAngularSpeed);

            if(ret1 != 0 || ret2 != 0 || ret3 != 0 )
            {
                printf("InitRun set failed!\n");
                initParaFlag = false;
            }
            else
            {
                lastAngularSpeed = baseAngularSpeed;
                lastPose = currentpose;
                runDistance = 0;
                arrivingFlag = false;
                initParaFlag = true;
            }

        }
    }
    else
    {
        //run forward following the arc
        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, forward, pathdirection);
        POINT curpoint = {currentpose.xPos, currentpose.yPos};
        distance = (int)agvMathUtils->getDistanceBetweenPoints(curpoint, circle.center);

        if(clockwise == pathdirection)
        {
            distance = (int)(distance - circle.radius);
        }
        else
        {
            distance = (int)(circle.radius - distance);
        }

        if(false == arrivingFlag)
        {
            // almost near the station, slow down the agv
            if(currentpose.xPos > -(STANDARD_RADIUS/200 * currentLinearSpeed / BASE_LINEAR_SPEED * 30)
                    && currentpose.xPos > - DISTANCE_1000_MM*0.6)//42
            {
                arrivingFlag = true;
                targetSpeed = BASE_LINEAR_SPEED + 50;
                dspeed = calDspeed()*1.4;//2.8
                mode = lowSpeedMode;
            }
        }

        float runAngle = fabs(currentpose.angle - lastPose.angle);
        runDistance = (int)agvMathUtils->getDistanceBetweenPose(currentpose, lastPose);
        //control the two wheels when the control period comes
        if(runDistance > DISTANCE_10_MM * currentLinearSpeed / BASE_LINEAR_SPEED || runAngle > ANGLE_1_DEGREE)
        {
            printf("\n*****************************************\nRunDistance:%d. runAngle:%f.\n ", runDistance,runAngle);

            ManageWheelSpeed();

            //fuzzy control the motor speed
            wheelControl(distance, angleDiff);
            lastPose = currentpose;
        }

        int absSpeed;
        int finalDelayDistance;
        absSpeed = getAbsSpeed();
        finalDelayDistance = fabs((float)DELAY_DISTANCE_OFFSET/ (float)BASE_LINEAR_SPEED * absSpeed);
        if(finalDelayDistance > 2*DELAY_ARC_DISTANCE_OFFSET)
        {
            finalDelayDistance = 2*DELAY_DISTANCE_OFFSET;
        }

        //check if the agv reach the target point
        if(currentpose.xPos >= (0 - finalDelayDistance))
        {
            //arrive the station, reset paras.
            printf("arrive the station\n");
            memset(&lastPose, 0, sizeof(lastPose));
            memset(&circle, 0, sizeof(circle));
            arrivingFlag = false;
            runningMode = StopMode;
            mode = lowSpeedMode;
            initParaFlag = false;
            return 1;
        }
    }

    return 0;
}

int CAgvMoveHelperForklift::BackwardToArc(POSE currentpose, POSE targetpose)
{
    static runMode mode = lowSpeedMode;
    static bool arrivingFlag = false;
    //static int savedTargetSpeed = 0;
    static POSE lastPose = {0, 0, 0.0};
    static PATHDIRECTION pathdirection = anticlockwise;

    static CIRCLE circle;
    float angleDiff;
    int distance;
    int runDistance;

    POSE tempPose = currentpose;

    currentpose = agvMathUtils->transformPose(currentpose, -LIDAR_TO_TAIL_LENGTH);
    targetpose = agvMathUtils->transformPose(targetpose, -LIDAR_TO_TAIL_LENGTH);
    //transform from global coordinate to relative coordinate
    currentpose = agvMathUtils->transformCoordinate(targetpose, currentpose);
    memset(&targetpose, 0, sizeof(POSE));

    if(false == initParaFlag)
    {
        printf("\n~~~~~~~\n~~~~~~~\n  BackwardToArc-start__startPose: (%d, %d, %f)\n", tempPose.xPos, tempPose.yPos, tempPose.angle);

        if(currentpose.xPos <= 0.0)
        {
            printf("start Pose error!!! current pose: (%d %d)\n", currentpose.xPos, currentpose.yPos);
            return -1;
        }
        else
        {
            circle = agvMathUtils->getCircleCenter(currentpose, targetpose);

            if((circle.center.x > currentpose.xPos && circle.center.y > 0.0)
                    || (circle.center.x < currentpose.xPos && circle.center.y < 0.0))
            {
                pathdirection = clockwise;
            }
            else
            {
                pathdirection = anticlockwise;
            }

            //set linear speed and angular speed
            currentLinearSpeed = -initSpeed-50;
            targetSpeed = -RUNARC_SPPED_MMS;
            //savedTargetSpeed = targetSpeed;
            aspeed = calAspeed();
            int radius = sqrt(circle.radius * circle.radius + wheelBase * wheelBase);
            baseAngularSpeed = (float)currentLinearSpeed / radius;
            // left vel > right vel
            if(clockwise == pathdirection)
            {
                baseAngularSpeed = -baseAngularSpeed;
            }
            else
            {
            }
            printf("base angular speed:%f\n", baseAngularSpeed);

            currentAngularSpeed = baseAngularSpeed;
            int ret1 = s_Controler->SetDirection(1);
            int ret2 = s_Controler->SetBreak(false);
            int ret3 = controler->SetCarSpeed(currentLinearSpeed, currentAngularSpeed);

            if(ret1 != 0 || ret2 != 0 || ret3 != 0 )
            {
                printf("InitRun set failed!\n");
                initParaFlag = false;
            }
            else
            {
                lastAngularSpeed = baseAngularSpeed;
                lastPose = currentpose;
                runDistance = 0;
                arrivingFlag = false;
                initParaFlag = true;
            }

        }
    }
    else
    {
        //run forward following the arc
        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, backward, pathdirection);
        POINT curpoint = {currentpose.xPos, currentpose.yPos};
        distance = (int)agvMathUtils->getDistanceBetweenPoints(curpoint, circle.center);

        if(clockwise == pathdirection)
        {
            distance = -(int)(distance - circle.radius);
        }
        else
        {
            distance = -(int)(circle.radius - distance);
        }

        if(false == arrivingFlag)
        {
            if(ActionCharge == targetPoint.action || ActionLiftUpDown == targetPoint.action)
            {
                // almost near the station, slow down the agv
                if(currentpose.xPos < (STANDARD_RADIUS/200 * abs(currentLinearSpeed) / BASE_LINEAR_SPEED * 30))//42
                {
                    arrivingFlag = true;
                    targetSpeed = -BASE_LINEAR_SPEED -50 ;
                    dspeed = calDspeed();
                }
            }
            else //normal run
            {
                // almost near the station, slow down the agv
                if(currentpose.xPos < (STANDARD_RADIUS/200 * abs(currentLinearSpeed) / BASE_LINEAR_SPEED * 30))//42
                {
                    arrivingFlag = true;
                    targetSpeed = -BASE_LINEAR_SPEED - 50;
                    dspeed = calDspeed()*0.5;//1
                }
            }

        }

        float runAngle = fabs(currentpose.angle - lastPose.angle);
        runDistance = (int)agvMathUtils->getDistanceBetweenPose(currentpose, lastPose);
        //control the two wheels when the control period comes
        if(runDistance > DISTANCE_10_MM * abs(currentLinearSpeed) / BASE_LINEAR_SPEED || runAngle > ANGLE_1_DEGREE)
        {
            printf("\n*****************************************\nRunDistance:%d. runAngle:%f.\n ", runDistance,runAngle);

            ManageWheelSpeed();
            //fuzzy control the motor speed
            wheelControl(distance, angleDiff);
            lastPose = currentpose;
        }        

        int absSpeed;
        int finalDelayDistance;
        absSpeed = getAbsSpeed();
        finalDelayDistance = fabs((float)DELAY_DISTANCE_OFFSET/ (float)BASE_LINEAR_SPEED * absSpeed);
        if(finalDelayDistance > 2*DELAY_ARC_DISTANCE_OFFSET)
        {
            finalDelayDistance = 2*DELAY_DISTANCE_OFFSET;
        }

        //check if the agv reach the target point
        if(currentpose.xPos <= (0.0 + finalDelayDistance))
        {
            //arrive the station, reset paras.
            printf("arrive the station\n");
            memset(&lastPose, 0, sizeof(lastPose));
            memset(&circle, 0, sizeof(circle));
            arrivingFlag = false;
            runningMode = StopMode;
            mode = lowSpeedMode;
            initParaFlag = false;
            return 1;
        }
    }

    return 0;
}

int CAgvMoveHelperForklift::Rotate(float angle)
{
    int ret = 0;
    if(0.0 == angle)
    {
        return 1;
    }
    else if(angle < 0.0)
    {
        ret = RotateLeft(-angle);
    }
    else
    {
        ret = RotateRight(angle);
    }

    return ret;
}


#define TURNINGSPEED_MMS 400
#define TURNINGMIDSPEED_MMS 150//100
#define ARRIVINGSPEED_MMS 50
int CAgvMoveHelperForklift::RotateLeft(float angle)
{

    float currentCarAngle = 0.0;
    float decAngle = 0.0;
    static float lastCarAngle = 0.0;
    static float rotateCarAngle = 0.0;

    if(angle <= 0.0)
    {
        return -1;
    }

    currentCarAngle = GetCurrentPose().angle;

    //init the rotate Left parameters
    if(false == initParaFlag)
    {
        lastCarAngle = currentCarAngle;
        rotateCarAngle = 0.0;

        //wrc: don't receive the XML setting speed.
        turningSpeed = TURNINGSPEED_MMS;
        turningArriveSpeed = ARRIVINGSPEED_MMS;
        printf("TuringSpeed: %d, angle:%f.\n",turningSpeed,angle);

        targetSpeed = (int)(angle / ANGLE_180_DEGREE * turningSpeed*1.75);//1.25//1.75
        if(targetSpeed > turningSpeed)
        {
            targetSpeed = turningSpeed;
        }
        else if(targetSpeed < TURNINGMIDSPEED_MMS)
        {
            targetSpeed = TURNINGMIDSPEED_MMS;
        }
        currentLinearSpeed = targetSpeed;
        printf("current linear speed: %d\n", currentLinearSpeed);
        int ret1 = controler->RotateLeft(currentLinearSpeed);

        if (ret1 != 0)
        {
            printf("\n  rotateLeft-start_init failed!\n");
            initParaFlag = false;
        }
        else
        {
            printf("\n  rotateLeft-start_init para finish!\n");
            initParaFlag = true;
        }

    }
    else //integrate the rotate angle  in every rotate period
    {
        decAngle = currentCarAngle - lastCarAngle;

        if(decAngle < -ANGLE_270_DEGREE && decAngle > -ANGLE_360_DEGREE)
        {
            decAngle = decAngle + ANGLE_360_DEGREE;
        }
        else if(decAngle > ANGLE_270_DEGREE && decAngle < ANGLE_360_DEGREE)
        {
            decAngle = decAngle - ANGLE_360_DEGREE;
        }

        rotateCarAngle += decAngle;
        lastCarAngle = currentCarAngle;
    }

    float needRotateAngle = 0.0;
    if(angle <= DELAY_ANGLE_OFFSET)
    {
        needRotateAngle = angle / 3;
    }
    else
    {
        needRotateAngle = angle - DELAY_ANGLE_OFFSET;
    }

    //check if current rotate angle reach the target  roate angle, reset all the parameters
    if(rotateCarAngle >=  needRotateAngle)
    {
        printf("rotate left finish! rotate angle:%f, angle: %f\n", rotateCarAngle, angle);
        controler->StopCar(false);
        usleep(100*1000);//10*1000
        if(ActionCharge ==  targetPoint.action || (ActionLiftUpDown == targetPoint.action && true == targetPoint.needStop) )
        {
            controler->SetBreak(true);
            printf("After break usleep(400*1000);\n");
            usleep(400*1000);
        }

        initParaFlag = false;
        isChangeSpeedInit = false;
        lastCarAngle = 0.0;
        rotateCarAngle = 0.0;
        //arrivingFlag = false;
        runningMode = StopMode;

        return 1;
    }
    else
    {
        //WRC:change below 4 lines to here.
        //slow down the rotate speed when almost arrive
        if(angle - rotateCarAngle <= 24 && rotateCarAngle > 0.55 * angle)//20degree,0.66
        {
            ChangeRotateSpeedByAngle(angle - rotateCarAngle, left);
        }
        return 0;
    }
}

int CAgvMoveHelperForklift::RotateRight(float angle)
{
    float currentCarAngle = 0.0;
    float decAngle = 0.0;
    static float lastCarAngle = 0.0;
    static float rotateCarAngle = 0.0;

    if(angle <= 0.0)
    {
        return -1;
    }

    currentCarAngle = GetCurrentPose().angle;

    //init the rotate right parameters
    if(false == initParaFlag)
    {        
        lastCarAngle = currentCarAngle;
        rotateCarAngle = 0.0;

        //wrc: don't receive the XML setting speed.
        turningSpeed = TURNINGSPEED_MMS;
        turningArriveSpeed = ARRIVINGSPEED_MMS;
        printf("TuringSpeed: %d, angle:%f.\n",turningSpeed,angle);

        targetSpeed = (int)(angle / ANGLE_180_DEGREE * turningSpeed*1.75);//1.25//1.75
        if(targetSpeed > turningSpeed)
        {
            targetSpeed = turningSpeed;
        }
        else if(targetSpeed < TURNINGMIDSPEED_MMS)
        {
            targetSpeed = TURNINGMIDSPEED_MMS;
        }
        currentLinearSpeed = targetSpeed;
        printf("current linear speed: %d\n", currentLinearSpeed);

        int ret1 = controler->RotateRight(currentLinearSpeed);
        if (ret1 != 0)
        {
            printf("\n  rotateRight-start_init failed!\n");
            initParaFlag = false;
        }
        else
        {
            printf("\n  rotateRight-start_init para finish!\n");
            initParaFlag = true;
        }

    }
    else //integrate the rotate angle  in every rotate period
    {
        decAngle = lastCarAngle - currentCarAngle;
        if(decAngle < -ANGLE_270_DEGREE && decAngle > -ANGLE_360_DEGREE)
        {
            decAngle = decAngle + ANGLE_360_DEGREE;
        }
        else if(decAngle > ANGLE_270_DEGREE && decAngle < ANGLE_360_DEGREE)
        {
            decAngle = decAngle - ANGLE_360_DEGREE;
        }

        rotateCarAngle += decAngle;
        lastCarAngle = currentCarAngle;
        //cout<<"Rotated__CarAngle:"<<rotateCarAngle;
    }

    float needRotateAngle = 0.0;
    if(angle <= DELAY_ANGLE_OFFSET)
    {
        needRotateAngle = angle / 3;
    }
    else
    {
        needRotateAngle = angle - DELAY_ANGLE_OFFSET;
    }
    //cout<<"    needRotateAngle:"<<needRotateAngle<<endl;

    //check if current rotate angle reach the target  roate angle, reset all the parameters
    if(rotateCarAngle >=  needRotateAngle)
    {
        printf("rotate right finish! rotate angle:%f, angle: %f\n", rotateCarAngle, angle);
        controler->StopCar(false);
        usleep(100*1000);//10*1000
        if(ActionCharge ==  targetPoint.action || (ActionLiftUpDown == targetPoint.action && true == targetPoint.needStop) )
        {
            controler->SetBreak(true);
            printf("After break usleep(400*1000);\n");
            usleep(400*1000);
        }

        initParaFlag = false;
        isChangeSpeedInit = false;
        lastCarAngle = 0.0;
        rotateCarAngle = 0.0;
        runningMode = StopMode;

        return 1;
    }
    else
    {
        //WRC:change below 4 lines to here.
        //slow down the rotate speed when almost arrive
        if(angle - rotateCarAngle <= 24 && rotateCarAngle > 0.58 * angle)//20degree,0.66
        {
            ChangeRotateSpeedByAngle(angle - rotateCarAngle, right);
        }

        return 0;
    }
}

/****************************************************************************/

POSE CAgvMoveHelperForklift::GetCurrentPose()
{
    POSE new_pose;

    new_pose.xPos = s_Lidar->xPos;
    new_pose.yPos = s_Lidar->yPos;
    new_pose.angle = s_Lidar->angle;

    return new_pose;
}


#define LOWSPEEDALLOWED 300 //mm/s
void CAgvMoveHelperForklift::ManageWheelSpeed()
{
    int savedLinearSpeed = currentLinearSpeed;
    //forward mode
    if(targetSpeed >= 0 && currentLinearSpeed >= 0)
    {
        if(1 == BarrierStatus || 2 == BarrierStatus)//1:Far 2:Mid
        {
            printf("FarBarrierOn!!! Speed Down!!!\n");            
            if(currentLinearSpeed > LOWSPEEDALLOWED)
            {
                currentLinearSpeed = LOWSPEEDALLOWED;
            }
            else
            {
            }

        }
        else
        {
            //decrease the speed
            if(currentLinearSpeed > targetSpeed)
            {
                currentLinearSpeed = currentLinearSpeed - dspeed;
                if(currentLinearSpeed < targetSpeed)
                {
                    currentLinearSpeed = targetSpeed;
                }
            }//increase the speed
            else if(currentLinearSpeed < targetSpeed)
            {
                currentLinearSpeed = currentLinearSpeed + aspeed;
                if(currentLinearSpeed > targetSpeed)
                {
                    currentLinearSpeed = targetSpeed;
                }
            }
        }
        baseAngularSpeed = baseAngularSpeed * ((float)currentLinearSpeed / savedLinearSpeed);
    }//backward mode
    else if(targetSpeed <= 0 && currentLinearSpeed <= 0)
    {
        //decrease the speed
        if(currentLinearSpeed < targetSpeed)
        {
            currentLinearSpeed = currentLinearSpeed + dspeed;
            if(currentLinearSpeed > targetSpeed)
            {
                currentLinearSpeed = targetSpeed;
            }
        }//increase the speed
        else if(currentLinearSpeed > targetSpeed)
        {
            currentLinearSpeed = currentLinearSpeed - aspeed;
            if(currentLinearSpeed < targetSpeed)
            {
                currentLinearSpeed = targetSpeed;
            }
        }
        else
        {

        }

        baseAngularSpeed = baseAngularSpeed * ((float)currentLinearSpeed / savedLinearSpeed);
    }
    else
    {
        printf("invalid speed paras!!!!!!!!!!!\n");
        printf("target speed: %d, current linear speed: %d, s_controler->speed:%d.\n", targetSpeed, currentLinearSpeed, s_Controler->speed);
    }

    int absSpeed = getAbsSpeed();
    printf("Speed: (%d, %d), (%d, %d). absSpeed:%d.\n", currentLinearSpeed, targetSpeed, aspeed, dspeed, absSpeed);

}

#define BASE_K_ALPHA 9//14//11//10//7//8--OK
#define DEAD_ZONE_DISTANCE 3//20 //mm//3mm
#define DEAD_ZONE_ANGLE 0.3//1 //degree//0.3
#define LEFT_RIGHT_ERROR_OFFSET 0//-15 //MM
//#define GAIN 1.0//1.7//1.3
int CAgvMoveHelperForklift::wheelControl(int distance, float carAngle)
{

    float output = 0.0;
    float Kp = 1.0,Kd = 0.0;
    if(abs(distance) < DEAD_ZONE_DISTANCE && fabs(carAngle) < DEAD_ZONE_ANGLE)
    {
        currentAngularSpeed = baseAngularSpeed;
    }
    else
    {
        agvMathUtils->limitThrehold(&distance, -DISTANCE_1000_MM, DISTANCE_1000_MM);

        float K_alpha = BASE_K_ALPHA / sqrt(fabs((float)currentLinearSpeed / BASE_LINEAR_SPEED));
        //printf("K_alpha: %f\n", K_alpha);
        output = distance + K_alpha * abs(currentLinearSpeed) * sin(carAngle * PI / ANGLE_180_DEGREE);
        output = output + LEFT_RIGHT_ERROR_OFFSET;
        output = -output / 50;//200

        if (GoBackwardMode == runningMode || BackwardToArcMode == runningMode
                || ActionCharge == targetPoint.action) //fork cargo or actioncharge
        {
            Kp = 1.3;//1.3
            Kd = 0.0;
        }
        else if(ForwardToArcMode == runningMode)
        {
            Kp = 0.7;
            Kd = 0.0;//0.0
        }
        else
        {
            Kp = 1.0;
            Kd = 8.0;
        }


        //PDcontrol
        static float lastoutput = 0.0;
        float incrementOutput = 0.0;
        if(abs(currentLinearSpeed) <= 600)
        {
            incrementOutput = 0.0;
        }
        else
        {
            incrementOutput = output - lastoutput;
        }

        printf("IncrementOutput:%f. output0:%f \n",incrementOutput,output);
        lastoutput = output;
        agvMathUtils->limitThrehold(&incrementOutput, -fabs(output/5), fabs(output/5));//2
        output = output* Kp + incrementOutput* Kd;//4.0//10.0


        agvMathUtils->limitThrehold(&output, -5.0, 5.0);

        output = output * (PI / ANGLE_180_DEGREE);

        currentAngularSpeed = baseAngularSpeed + output;
        agvMathUtils->limitThrehold(&currentAngularSpeed, baseAngularSpeed - MAX_ANGULAR_SPEED_FORWARD, baseAngularSpeed + MAX_ANGULAR_SPEED_FORWARD);
    }

    POSE currentPose = GetCurrentPose();//printf currentpose for view
    printf("<<%d, %.3f>>--DisDiff,AngleDiff  Base:%f Output: %f CurrentPose:(%d,%d,%.2f).\n", distance, carAngle, baseAngularSpeed, output,
           currentPose.xPos,currentPose.yPos,currentPose.angle);

    if(lastAngularSpeed != currentAngularSpeed || lastLinearSpeed != currentLinearSpeed)
    {
        controler->SetCarSpeed(currentLinearSpeed, currentAngularSpeed);
        lastLinearSpeed = currentLinearSpeed;
        lastAngularSpeed = currentAngularSpeed;
        //printf("**Actually ajuction angle: %f.\n\n", s_Controler->angle);
    }

    return 0;
}


void CAgvMoveHelperForklift::ChangeRotateSpeedByAngle(float angle, ROTATE_DIRECTION direction)
{
    static float lastAngle = 0;
    static int deltaSpeed = 0;

    if(false == isChangeSpeedInit)
    {
        deltaSpeed = (currentLinearSpeed - turningArriveSpeed) / SPEED_CHANGE_TIMES ;//+10
        printf("deltaSpeed: %d", deltaSpeed);
        lastAngle = angle;
        isChangeSpeedInit = true;
    }
    else
    {
        if(fabs(angle - lastAngle) > ANGLE_1_DEGREE)
        {
            cout<<"RemainedRotateCarAngle:"<<angle<<endl;
            printf("current speed: %d\n", currentLinearSpeed);
            currentLinearSpeed = currentLinearSpeed - deltaSpeed;

            if(currentLinearSpeed < turningArriveSpeed)
            {
                currentLinearSpeed = turningArriveSpeed;
            }
            printf("deltaSpeed: %d\n", deltaSpeed);
            printf("current line speed: %d, %d\n", currentLinearSpeed, turningArriveSpeed);
            targetSpeed = currentLinearSpeed;

            lastAngle = angle;

//            if(left == direction)
//            {
//                controler->RotateLeft(currentLinearSpeed);
//            }
//            else
//            {
//                controler->RotateRight(currentLinearSpeed);
//            }
            controler->RunForward(currentLinearSpeed);
        }
    }
}

//#define __8_SECOND 8
//#define __7_PERIODS 7
#define __adSpeed_SECOND 4
#define __adSpeed_PERIODS 15//5 //wrc: ajustion counts per second
int CAgvMoveHelperForklift::calAspeed()
{
    //aspeed = abs(targetSpeed - currentLinearSpeed) / (8 * 6);//original
    aspeed = abs(targetSpeed - currentLinearSpeed) / (__adSpeed_SECOND * __adSpeed_PERIODS);
    if(aspeed < MIN_ASPEED)
    {
        aspeed = MIN_ASPEED;
    }

    return aspeed;
}

int CAgvMoveHelperForklift::calDspeed()
{
    dspeed = abs(currentLinearSpeed - targetSpeed) / (__adSpeed_SECOND * __adSpeed_PERIODS);
    if(dspeed < MIN_DSPEED)
    {
        dspeed = MIN_DSPEED;
    }

    return dspeed;
}

int CAgvMoveHelperForklift::calTargetSpeedByPathDistance(int distance)
{
    int speed;

    if(distance > DISTANCE_1000_MM * 7)
    {
        speed = SPEED_100_MM_PER_SECOND * 15;//10
    }
    else if(distance > DISTANCE_1000_MM * 5)
    {
        speed = SPEED_100_MM_PER_SECOND * 13;
    }
    else if(distance > DISTANCE_1000_MM * 3)
    {
        speed = SPEED_100_MM_PER_SECOND * 10;//5
    }
    else if(distance > DISTANCE_1000_MM * 2)
    {
        speed = SPEED_100_MM_PER_SECOND * 5;//3
    }
    else if(distance > DISTANCE_1000_MM * 0.5)//1
    {
        speed = SPEED_100_MM_PER_SECOND * 3;
    }
    else
    {
        speed = SPEED_100_MM_PER_SECOND * 1.5;
    }

    if(speed > MAXSPEEDALLOWED)
    {
        speed = MAXSPEEDALLOWED;
    }

    return speed;
}

void CAgvMoveHelperForklift::forwardSpeedInit(int *savedTargetSpeed, runMode *mode)
{
    if(ActionCharge == targetPoint.action)
    {
        targetSpeed = GoChargeSpeed_mm_s;
        currentLinearSpeed = GoChargeSpeed_mm_s;
        *savedTargetSpeed = GoChargeSpeed_mm_s;
        *mode = lowSpeedMode;
    }
    else
    {
        if(s_Controler->speed > *savedTargetSpeed && s_Controler->speed < 65536/2)
        {
            *mode = highSpeedMode;
            currentLinearSpeed = *savedTargetSpeed;
        }
        else if(s_Controler->speed > ForwardInitDefaultSpeed && s_Controler->speed < 65536/2)
        {
            *mode = lowSpeedMode;
            currentLinearSpeed = s_Controler->speed;//wrc:keep speed
        }
        else
        {
            *mode = lowSpeedMode;
            currentLinearSpeed = ForwardInitDefaultSpeed;
        }
    }

    baseAngularSpeed = 0.0;
    currentAngularSpeed = 0.0;
}

int CAgvMoveHelperForklift::calFinalDelayDis(int *SpeedFinalFinal, float *Kdistance)
{
    int absSpeed;
    int finalDelayDistance;
    if( 1001 == carID)
    {
        *SpeedFinalFinal = 45;//1#che:45 //2#che:35
        *Kdistance = 0.65;//1#che:0.65 //2#che:0.75
    }
    else
    {
        *SpeedFinalFinal = 35;//1#che:45 //2#che:35
        *Kdistance = 0.75;//1#che:0.65 //2#che:0.75
    }

    if(s_Controler->speed > 65536/2)
    {
         absSpeed =  65536-s_Controler->speed;
    }
    else
    {
        absSpeed = s_Controler->speed;
    }

    if((ActionCharge ==targetPoint.action && GoForwardMode == runningMode ) ||
        (ActionLiftUpDown ==targetPoint.action && GoBackwardMode == runningMode))
    {
        if(abs(currentLinearSpeed) < 65)
        {
            finalDelayDistance = absSpeed * (*Kdistance);
        }
        else
        {
            finalDelayDistance = FinalDelayDefault_DISTANCE;
        }
    }
    else
    {
        finalDelayDistance = fabs((float)DELAY_DISTANCE_OFFSET/ (float)BASE_LINEAR_SPEED * absSpeed);
        if(finalDelayDistance > 2*DELAY_DISTANCE_OFFSET)
        {
            finalDelayDistance = 2*DELAY_DISTANCE_OFFSET;
        }
    }
    return finalDelayDistance;
}

int CAgvMoveHelperForklift::getAbsSpeed(void)
{
    int absSpeed;
    if(s_Controler->speed > 65536/2)
    {
         absSpeed =  65536-s_Controler->speed;
    }
    else
    {
        absSpeed = s_Controler->speed;
    }
    return absSpeed;
}


void CAgvMoveHelperForklift::clearSubpath(void)
{
    for(uint i=0;i<(sizeof(subpath)/sizeof(subpath[0]));i++)
    {
        subpath[i].runmode = 0;
        subpath[i].target.xPos = 0;
        subpath[i].target.yPos = 0;
        subpath[i].target.angle = 0.0;
        subpath[i].distance = 0;
        subpath[i].speed = 0;
    }
    subpathSize = 0;
}

void CAgvMoveHelperForklift::setSubpath(int index, POSE target, int mode)
{
    subpath[index].target = target;
    subpath[index].runmode = mode;
}

#define MAXTURNINGRADIUS 4000
#define MINTURNINGRADIUS 1000
/******************************************************************************************/

int CAgvMoveHelperForklift::SplitToSubTarget(POSE currentpose, POSE targetpose, DIRECTION direction)
{
    float angleDiff = 0.0;
    float targetAngle = 0.0;
    int subindex = 0;
    POSE subtarget = {0, 0, 0.0};
    POSE savedCurrentPose = {0, 0, 0.0};
    bool mirrorFlag = false;
    CIRCLE circle;
    int x = 0;
    float rad_angle = 0.0;
    int length_AE = 0;
    int length_BE = 0;


    clearSubpath();

    if(currentpose.angle >= ANGLE_360_DEGREE || currentpose.angle < 0.0
            || targetpose.angle >= ANGLE_360_DEGREE || targetpose.angle < 0.0)
    {
        printf("input pose error!!!\n");
        return -1;
    }

    currentpose = agvMathUtils->transformPose(currentpose, -LIDAR_TO_TAIL_LENGTH);
    targetpose = agvMathUtils->transformPose(targetpose, -LIDAR_TO_TAIL_LENGTH);

    currentpose = agvMathUtils->transformCoordinate(targetpose, currentpose);

    printf("current pose: (%d, %d, %f)\n", currentpose.xPos, currentpose.yPos, currentpose.angle);

    //WRC:20171217 calculate auto-adjust radius    
    if (forward == direction)
    {
        POSE tempPose = agvMathUtils->getPoseFrom_x_y_alpha(-400,0,0.0);//400mm remained to run straight
        CIRCLE tempCircle = agvMathUtils->getCircleCenter(currentpose, tempPose);
        STANDARD_RADIUS = tempCircle.radius;
        if(STANDARD_RADIUS >= abs(currentpose.xPos)-400 && abs(currentpose.yPos)> abs(currentpose.xPos)-400)
        {
            STANDARD_RADIUS = 2800; //for special condition at HX.
        }
    }
    else
    {
        POSE tempPose = agvMathUtils->getPoseFrom_x_y_alpha(400,0,0.0);
        CIRCLE tempCircle = agvMathUtils->getCircleCenter(currentpose, tempPose);
        STANDARD_RADIUS = tempCircle.radius;
        if(STANDARD_RADIUS >= abs(currentpose.xPos)-400 && abs(currentpose.yPos)> abs(currentpose.xPos)-400)
        {
            STANDARD_RADIUS = 2800;
        }
    }

    if(fabs(currentpose.angle-90.0)<=20.0 ||fabs(currentpose.angle-270.0)<=20.0)//if it's a 90degree turning
    {
        STANDARD_RADIUS = SavedXMLSTANDARD_RADIUS;
    }
    else if(STANDARD_RADIUS > MAXTURNINGRADIUS)
    {
        STANDARD_RADIUS = MAXTURNINGRADIUS;
    }

    if(backward == direction || STANDARD_RADIUS < MINTURNINGRADIUS)
    {
        STANDARD_RADIUS = MINTURNINGRADIUS;
    }
    printf("STANDARD_RADIUS = %d.\n",STANDARD_RADIUS);


    if(forward == direction)
    {
        if(currentpose.xPos > 0)
        {
            printf("the agv is ahead of the target pose!!! current pose: (%d %d)\n", currentpose.xPos, currentpose.yPos);
            return -1;
        }
        else
        {
            savedCurrentPose = currentpose;
            if(currentpose.yPos < 0)
            {
                mirrorFlag = true;
                currentpose.yPos = -currentpose.yPos;
                if(0.0 == currentpose.angle)
                {

                }
                else
                {
                    currentpose.angle = ANGLE_360_DEGREE - currentpose.angle;
                }
            }
            else
            {
                mirrorFlag = false;
            }

            //calculate the sub target path
            //follow the line.
            if(currentpose.yPos <= DISTANCE_200_MM && currentpose.yPos >= 0)
            {
                currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

                //wrc-20171226
                if(abs(currentpose.xPos)<DISTANCE_200_MM*1.5)
                {
                    targetAngle = 0.0;
                }
                else
                {
                    targetAngle = atan2(-currentpose.yPos, -currentpose.xPos);
                    targetAngle = targetAngle * ANGLE_180_DEGREE / PI;
                }


                printf("target angle: %f, current angle: %f\n", targetAngle, currentpose.angle);

                angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, targetAngle);
                printf("angle diff: %f\n", angleDiff);

                //if the car angle is to big, we had to rotate the car before follow the line.
                //that means the car have to do two steps, including rotate and line.
                if(fabs(angleDiff) > ANGLE_20_DEGREE)
                {
                    subpathSize = 2;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                    setSubpath(subindex++, subtarget, RotateLeftMode);
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoForwardMode);
                }//the car angle is small, so only including one step: follow line.
                else
                {
                    subpathSize = 1;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoForwardMode);
                }
            }//first follow the arc, then follow the line.
            else if(currentpose.yPos <= STANDARD_RADIUS && currentpose.yPos > DISTANCE_200_MM)
            {
                //            printf("y: %d\n", currentpose.yPos);
                x = sqrt(STANDARD_RADIUS * STANDARD_RADIUS
                         - (STANDARD_RADIUS - currentpose.yPos) * (STANDARD_RADIUS - currentpose.yPos));
                //            printf("x: %d\n", x);
                circle = agvMathUtils->getCircleFromCenterAndRadius(currentpose.xPos + x, STANDARD_RADIUS, STANDARD_RADIUS);

                angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, forward, anticlockwise);
                printf("angle diff: %f\n", angleDiff);

                //if the car angle is to big, we had to rotate the car before follow the arc.
                //that means the car have to do three steps, including rotate, arc and line.
                if(fabs(angleDiff) > ANGLE_20_DEGREE)
                {
                    subpathSize = 3;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                    setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + x, 0, 0.0);
                    setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoForwardMode);//forward
                }//the car angle is small, so only including two step: follow arc, and then follow line.
                else
                {
                    subpathSize = 2;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + x, 0, 0.0);
                    setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoForwardMode);//forward
                }
            }
            else if(currentpose.yPos > STANDARD_RADIUS)
            {
                //current pose is in the second or third quadrant
                if(currentpose.angle >= ANGLE_90_DEGREE && currentpose.angle <= ANGLE_270_DEGREE)
                {
                    currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

                    targetAngle = -ANGLE_90_DEGREE;
                    printf("target angle: %f, current angle: %f\n", targetAngle, currentpose.angle);

                    angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, targetAngle);
                    printf("angle diff: %f\n", angleDiff);

                    //if the car angle is to big, we had to rotate the car before follow the vertical line.
                    //that means the car have to do four steps, including rotate, vertical line, arc and line.
                    if(fabs(angleDiff) > ANGLE_20_DEGREE)
                    {
                        subpathSize = 4;
                        subindex = 0;
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                        setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos, STANDARD_RADIUS, ANGLE_270_DEGREE);
                        setSubpath(subindex++, subtarget, GoForwardMode);//forward vertical line
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + STANDARD_RADIUS, 0, 0.0);
                        setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                        setSubpath(subindex++, subtarget, GoForwardMode);//forward
                    }//the car angle is small, so only including three steps: follow vertical line, arc, and then follow line.
                    else
                    {
                        subpathSize = 3;
                        subindex = 0;
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos, STANDARD_RADIUS, ANGLE_270_DEGREE);
                        setSubpath(subindex++, subtarget, GoForwardMode);//forward vertical line
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + STANDARD_RADIUS, 0, 0.0);
                        setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                        setSubpath(subindex++, subtarget, GoForwardMode);//forward
                    }
                }//current pose is in the first quadrant
                else if(currentpose.angle >= 0.0 && currentpose.angle < ANGLE_90_DEGREE)
                {
                    //the distance to the target line is bigger than 2R
                    if(currentpose.yPos > 2 * STANDARD_RADIUS)
                    {
                        currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

                        targetAngle = 0.0;
                        printf("target angle: %f, current angle: %f\n", targetAngle, currentpose.angle);

                        angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, targetAngle);
                        printf("angle diff: %f\n", angleDiff);

                        //if the car angle is to big, we had to rotate the car before follow the vertical line.
                        //that means the car have to do five steps, including rotate, arc, vertical line, arc and line.
                        if(fabs(angleDiff) > ANGLE_20_DEGREE)
                        {
                            subpathSize = 5;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                            setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + STANDARD_RADIUS, currentpose.yPos - STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + STANDARD_RADIUS, STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward vertical line
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + 2 * STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward
                        }//the car angle is small, so only including four steps: arc, follow vertical line, arc, and then follow line.
                        else
                        {
                            subpathSize = 4;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + STANDARD_RADIUS, currentpose.yPos - STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + STANDARD_RADIUS, STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward vertical line
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + 2 * STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward
                        }
                    }//the distance to the target line is smaller than 2R
                    else
                    {
                        x = sqrt(STANDARD_RADIUS * STANDARD_RADIUS - (currentpose.yPos - STANDARD_RADIUS) * (currentpose.yPos - STANDARD_RADIUS));
                        length_AE = currentpose.yPos - STANDARD_RADIUS;
                        length_BE = STANDARD_RADIUS - x;
                        //            printf("x: %d\n", x);
                        circle = agvMathUtils->getCircleFromCenterAndRadius(currentpose.xPos - x, STANDARD_RADIUS, STANDARD_RADIUS);

//                        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, forward, anticlockwise);//wrc:why
                        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, forward, clockwise);
                        printf("angle diff: %f\n", angleDiff);

                        //if the car angle is to big, we had to rotate the car before follow the vertical line.
                        //that means the car have to do four steps, including rotate, arc, arc and line.
                        if(fabs(angleDiff) > ANGLE_20_DEGREE)
                        {
                            subpathSize = 4;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                            setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE, STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE + STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward
                        }//the car angle is small, so only including three steps: arc, arc, and then follow line.
                        else
                        {
                            subpathSize = 3;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE, STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE + STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward
                        }
                    }
                }//current pose is in the fourth quadrant
                else if(currentpose.angle > ANGLE_270_DEGREE && currentpose.angle < ANGLE_360_DEGREE)
                {
                    rad_angle = (ANGLE_90_DEGREE + currentpose.angle - ANGLE_360_DEGREE) * PI / ANGLE_180_DEGREE;
                    length_AE = STANDARD_RADIUS * sin(rad_angle);
                    printf("length AE: %dmm\n", length_AE);

                    if(currentpose.yPos > (STANDARD_RADIUS + length_AE))
                    {
                        length_BE = STANDARD_RADIUS - STANDARD_RADIUS * cos(rad_angle);
                        printf("BE: %d, AE: %d\n", length_BE, length_AE);


                        //the car have to do four steps, including arc, vertical line, arc and line.
                        subpathSize = 4;
                        subindex = 0;
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE, currentpose.yPos - length_AE, ANGLE_270_DEGREE);
                        setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE, STANDARD_RADIUS, ANGLE_270_DEGREE);
                        setSubpath(subindex++, subtarget, GoForwardMode);//forward vertical line
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE + STANDARD_RADIUS, 0, 0.0);
                        setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                        setSubpath(subindex++, subtarget, GoForwardMode);//forward
                    }
                    else
                    {
                        x = sqrt(STANDARD_RADIUS * STANDARD_RADIUS - (currentpose.yPos - STANDARD_RADIUS) * (currentpose.yPos - STANDARD_RADIUS));
                        length_AE = currentpose.yPos - STANDARD_RADIUS;
                        length_BE = STANDARD_RADIUS - x;

                        circle = agvMathUtils->getCircleFromCenterAndRadius(currentpose.xPos - x, STANDARD_RADIUS, STANDARD_RADIUS);

                        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, forward, clockwise);
                        printf("angle diff: %f\n", angleDiff);

                        //if the car angle is to big, we had to rotate the car before follow the vertical line.
                        //that means the car have to do four steps, including rotate, arc, arc and line.
                        if(fabs(angleDiff) > ANGLE_20_DEGREE)
                        {
                            subpathSize = 4;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                            setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE, STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE + STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward
                        }//the car angle is small, so only including three steps: arc, arc, and then follow line.
                        else
                        {
                            subpathSize = 3;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE, STANDARD_RADIUS, ANGLE_270_DEGREE);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos + length_BE + STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, ForwardToArcMode);//forward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoForwardMode);//forward
                        }
                    }
                }
            }
        }
    }//negative direction
    else
    {
        if(currentpose.xPos < 0)
        {
            printf("the agv is behind the target pose!!! current pose: (%d %d)\n", currentpose.xPos, currentpose.yPos);
            return -1;
        }
        else
        {
            savedCurrentPose = currentpose;
            if(currentpose.yPos < 0)
            {
                mirrorFlag = true;
                currentpose.yPos = -currentpose.yPos;
                if(0.0 == currentpose.angle)
                {

                }
                else
                {
                    currentpose.angle = ANGLE_360_DEGREE - currentpose.angle;
                }
            }
            else
            {
                mirrorFlag = false;
            }

            //follow the line.
            if(currentpose.yPos <= DISTANCE_200_MM && currentpose.yPos >= 0)
            {
                currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

                //wrc-20171226
                if(abs(currentpose.xPos)<DISTANCE_200_MM*1.5)
                {
                    targetAngle = 0.0;
                }
                else
                {
                    targetAngle = atan2(currentpose.yPos, currentpose.xPos);
                    targetAngle = targetAngle * ANGLE_180_DEGREE / PI;
                }

                printf("target angle: %f, current angle: %f\n", targetAngle, currentpose.angle);

                angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, targetAngle);
                printf("angle diff: %f\n", angleDiff);

                //if the car angle is to big, we had to rotate the car before follow the line.
                //that means the car have to do two steps, including rotate and line.
                if(fabs(angleDiff) > ANGLE_20_DEGREE)
                {
                    subpathSize = 2;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                    setSubpath(subindex++, subtarget, RotateLeftMode);
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoBackwardMode);
                }//the car angle is small, so only including one step: follow line.
                else
                {
                    subpathSize = 1;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoBackwardMode);
                }
            }//first follow the arc, then follow the line.
            else if(currentpose.yPos <= STANDARD_RADIUS && currentpose.yPos > DISTANCE_200_MM)
            {
                x = sqrt(STANDARD_RADIUS * STANDARD_RADIUS
                         - (STANDARD_RADIUS - currentpose.yPos) * (STANDARD_RADIUS - currentpose.yPos));

                circle = agvMathUtils->getCircleFromCenterAndRadius(currentpose.xPos - x, STANDARD_RADIUS, STANDARD_RADIUS);

                angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, backward, anticlockwise);
                printf("angle diff: %f\n", angleDiff);

                //if the car angle is to big, we had to rotate the car before follow the arc.
                //that means the car have to do three steps, including rotate, arc and line.
                if(fabs(angleDiff) > ANGLE_20_DEGREE)
                {
                    subpathSize = 3;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                    setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - x, 0, 0.0);
                    setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                }//the car angle is small, so only including two step: follow arc, and then follow line.
                else
                {
                    subpathSize = 2;
                    subindex = 0;
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - x, 0, 0.0);
                    setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                    subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                    setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                }
            }
            else if(currentpose.yPos > STANDARD_RADIUS)
            {
                //current pose is in the second or third quadrant
                //first follow the vertical line, then follow then arc, then follow the line
                if(currentpose.angle >= ANGLE_90_DEGREE && currentpose.angle <= ANGLE_270_DEGREE)
                {
                    currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

                    targetAngle = ANGLE_90_DEGREE;
                    printf("target angle: %f, current angle: %f\n", targetAngle, currentpose.angle);

                    angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, targetAngle);
                    printf("angle diff: %f\n", angleDiff);

                    //if the car angle is to big, we had to rotate the car before follow the vertical line.
                    //that means the car have to do four steps, including rotate, vertical line, arc and line.
                    if(fabs(angleDiff) > ANGLE_20_DEGREE)
                    {
                        subpathSize = 4;
                        subindex = 0;
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                        setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos, STANDARD_RADIUS, ANGLE_90_DEGREE);
                        setSubpath(subindex++, subtarget, GoBackwardMode);//backward vertical line
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - STANDARD_RADIUS, 0, 0.0);
                        setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                        setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                    }//the car angle is small, so only including three steps: follow vertical line, arc, and then follow line.
                    else
                    {
                        subpathSize = 3;
                        subindex = 0;
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos, STANDARD_RADIUS, ANGLE_90_DEGREE);
                        setSubpath(subindex++, subtarget, GoBackwardMode);//backward vertical line
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - STANDARD_RADIUS, 0, 0.0);
                        setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                        setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                    }
                }
                else if(currentpose.angle > ANGLE_270_DEGREE && currentpose.angle < ANGLE_360_DEGREE)
                {
                    //the distance to the target line is bigger than 2R
                    if(currentpose.yPos > 2 * STANDARD_RADIUS)
                    {
                        currentpose.angle = agvMathUtils->transformAngle(currentpose.angle, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

                        targetAngle = 0.0;
                        printf("target angle: %f, current angle: %f\n", targetAngle, currentpose.angle);

                        angleDiff = agvMathUtils->getAngleDiff(currentpose.angle, targetAngle);
                        printf("angle diff: %f\n", angleDiff);

                        //if the car angle is to big, we had to rotate the car before follow the vertical line.
                        //that means the car have to do five steps, including rotate, arc, vertical line, arc and line.
                        if(fabs(angleDiff) > ANGLE_20_DEGREE)
                        {
                            subpathSize = 5;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                            setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - STANDARD_RADIUS, currentpose.yPos - STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - STANDARD_RADIUS, STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward vertical line
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - 2 * STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                        }//the car angle is small, so only including four steps: arc, follow vertical line, arc, and then follow line.
                        else
                        {
                            subpathSize = 4;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - STANDARD_RADIUS, currentpose.yPos - STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - STANDARD_RADIUS, STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward vertical line
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - 2 * STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                        }
                    }//the distance to the target line is smaller than 2R
                    else
                    {
                        x = sqrt(STANDARD_RADIUS * STANDARD_RADIUS - (currentpose.yPos - STANDARD_RADIUS) * (currentpose.yPos - STANDARD_RADIUS));
                        length_AE = currentpose.yPos - STANDARD_RADIUS;
                        length_BE = STANDARD_RADIUS - x;

                        printf("x: %d\n", x);
                        circle = agvMathUtils->getCircleFromCenterAndRadius(currentpose.xPos + x, STANDARD_RADIUS, STANDARD_RADIUS);
                        printf("circle: (%f %f %f)\n", circle.center.x, circle.center.y, circle.radius);

                        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, backward, clockwise);//not anticlockwise--20180126
                        printf("angle diff: %f\n", angleDiff);

                        //if the car angle is to big, we had to rotate the car before follow the vertical line.
                        //that means the car have to do four steps, including rotate, arc, arc and line.
                        if(fabs(angleDiff) > ANGLE_20_DEGREE)
                        {
                            subpathSize = 4;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                            setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE, STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE - STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                        }
                        else
                        {
                            subpathSize = 3;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE, STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE - STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                        }
                    }
                }//current pose is in the fourth quadrant
                else if(currentpose.angle >= 0.0 && currentpose.angle < ANGLE_90_DEGREE)
                {
                    rad_angle = (ANGLE_90_DEGREE - currentpose.angle) * PI / ANGLE_180_DEGREE;
                    length_AE = STANDARD_RADIUS * sin(rad_angle);

                    if(currentpose.yPos > (STANDARD_RADIUS + length_AE))
                    {
                        length_BE = STANDARD_RADIUS - STANDARD_RADIUS * cos(rad_angle);
                        printf("BE: %d, AE: %d\n", length_BE, length_AE);
                        //the car have to do four steps, including arc, vertical line, arc and line.
                        subpathSize = 4;
                        subindex = 0;
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE, currentpose.yPos - length_AE, ANGLE_90_DEGREE);
                        setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE, STANDARD_RADIUS, ANGLE_90_DEGREE);
                        setSubpath(subindex++, subtarget, GoBackwardMode);//backward vertical line
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE - STANDARD_RADIUS, 0, 0.0);
                        setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                        subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                        setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                    }
                    else
                    {
                        x = sqrt(STANDARD_RADIUS * STANDARD_RADIUS - (currentpose.yPos - STANDARD_RADIUS) * (currentpose.yPos - STANDARD_RADIUS));
                        length_AE = currentpose.yPos - STANDARD_RADIUS;
                        length_BE = STANDARD_RADIUS - x;

                        circle = agvMathUtils->getCircleFromCenterAndRadius(currentpose.xPos + x, STANDARD_RADIUS, STANDARD_RADIUS);

                        angleDiff = agvMathUtils->getRotateAngle(currentpose, circle.center, backward,clockwise);
                        printf("angle diff: %f\n", angleDiff);

                        //if the car angle is to big, we had to rotate the car before follow the vertical line.
                        //that means the car have to do four steps, including rotate, arc, arc and line.
                        if(fabs(angleDiff) > ANGLE_20_DEGREE)
                        {
                            subpathSize = 4;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, angleDiff);
                            setSubpath(subindex++, subtarget, RotateLeftMode);//rotate
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE, STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE - STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                        }//the car angle is small, so only including three steps: arc, arc, and then follow line.
                        else
                        {
                            subpathSize = 3;
                            subindex = 0;
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE, STANDARD_RADIUS, ANGLE_90_DEGREE);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(currentpose.xPos - length_BE - STANDARD_RADIUS, 0, 0.0);
                            setSubpath(subindex++, subtarget, BackwardToArcMode);//backward arc
                            subtarget = agvMathUtils->getPoseFrom_x_y_alpha(0, 0, 0.0);
                            setSubpath(subindex++, subtarget, GoBackwardMode);//backward
                        }
                    }
                }
            }
        }
    }

    if(mirrorFlag)
    {
        //if mirror side, the rotate angle is opposite.
        if(RotateLeftMode == subpath[0].runmode || RotateRightMode == subpath[0].runmode)
        {
            subpath[0].target.angle = -subpath[0].target.angle;
            if( -ANGLE_180_DEGREE == subpath[0].target.angle)
            {
                subpath[0].target.angle = ANGLE_180_DEGREE;
            }
        }

        //the angle is the mirror angle, the y pos is the mirror y pos.
        for(int i=0;i<subpathSize;i++)
        {
            if(RotateLeftMode != subpath[i].runmode && RotateRightMode != subpath[i].runmode)
            {
                subpath[i].target.yPos = -subpath[i].target.yPos;

                if(subpath[i].target.angle == 0.0)
                {

                }
                else
                {
                    subpath[i].target.angle = ANGLE_360_DEGREE - subpath[i].target.angle;
                }
            }
        }
    }

    if(RotateLeftMode == subpath[0].runmode || RotateRightMode == subpath[0].runmode)
    {
        if( -ANGLE_180_DEGREE == subpath[0].target.angle)
        {
            subpath[0].target.angle = ANGLE_180_DEGREE;
        }
    }

    //check if the sub path is back of the final target.
    //It guarantees the agv always running in front of the target.
    for(int i=0;i<subpathSize;i++)
    {
        //the sub path is behind the final target.
        if((forward == direction && subpath[i].target.xPos  > 0)
                || (backward == direction && subpath[i].target.xPos < 0))
        {
            printf("*****************************************\n");
            printf("the sub path is behind the final target.\n");
            printf("*****************************************\n");
            clearSubpath();
            subpathSize = 0;
            return -1;
        }
    }

    printf("**********************************************\n");
    printf("before simply and transform:\n");
    for(int i=0;i<subpathSize;i++)
    {
        printf("NO %d: target: (%d, %d, %f), mode: %d\n", i+1,
               subpath[i].target.xPos, subpath[i].target.yPos, subpath[i].target.angle,
               subpath[i].runmode);
    }


    printf("**********************************************\n");
    printf("after simply:\n");
    int distanceArray[5] = {0};
    if(subpathSize > 1)
    {
        if(RotateLeftMode == subpath[0].runmode || RotateRightMode == subpath[0].runmode)
        {
            distanceArray[0] = 0;
            distanceArray[1] = agvMathUtils->getDistanceBetweenPose(savedCurrentPose, subpath[1].target);
        }
        else
        {
            distanceArray[0] = agvMathUtils->getDistanceBetweenPose(savedCurrentPose, subpath[0].target);
            distanceArray[1] = agvMathUtils->getDistanceBetweenPose(subpath[0].target, subpath[1].target);
        }

        for(int i=2;i<subpathSize;i++)
        {
            distanceArray[i] = agvMathUtils->getDistanceBetweenPose(subpath[i-1].target, subpath[i].target);
        }

        SUBPATH copyOfSubPath[subpathSize];
        int index = 0;
        for(int i=0;i<subpathSize-1;i++)
        {

            if(RotateLeftMode == subpath[i].runmode || RotateRightMode == subpath[i].runmode
                    || distanceArray[i] > DISTANCE_200_MM)
            {
                copyOfSubPath[index].target = subpath[i].target;
                copyOfSubPath[index].runmode = subpath[i].runmode;
                copyOfSubPath[index].distance = distanceArray[i];
                index++;
            }
        }
        //wrc: the last one path should not be deleted when it needs to stop precisely
        if(true == targetPoint.needStop || RotateLeftMode == subpath[subpathSize-1].runmode
                || RotateRightMode == subpath[subpathSize-1].runmode || distanceArray[subpathSize-1] > DISTANCE_200_MM)
        {
            copyOfSubPath[index].target = subpath[subpathSize-1].target;
            copyOfSubPath[index].runmode = subpath[subpathSize-1].runmode;
            copyOfSubPath[index].distance = distanceArray[subpathSize-1];
            index++;
        }
        else
        {
            //none
        }

        clearSubpath();
        subpathSize = index;
        for(int i=0;i<subpathSize;i++)
        {
            subpath[i] = copyOfSubPath[i];
        }
    }
    else
    {
        distanceArray[0] = agvMathUtils->getDistanceBetweenPose(currentpose, subpath[0].target);
        subpath[0].distance = distanceArray[0];
    }

    for(int i=0;i<subpathSize;i++)
    {
        if(GoForwardMode == subpath[i].runmode || ForwardToArcMode == subpath[i].runmode)//actually runArc has its own speed, not here set.
        {
            subpath[i].speed = calTargetSpeedByPathDistance(subpath[i].distance);
        }
        else if(GoBackwardMode == subpath[i].runmode || BackwardToArcMode == subpath[i].runmode)
        {
            subpath[i].speed = -calTargetSpeedByPathDistance(subpath[i].distance);
        }
        else
        {
            subpath[i].speed = BASE_LINEAR_SPEED;
        }
    }

    for(int i=0;i<subpathSize;i++)
    {
        printf("NO %d: target: (%d, %d, %f), mode: %d, distance: %d, speed: %d\n", i+1,
               subpath[i].target.xPos, subpath[i].target.yPos, subpath[i].target.angle,
               subpath[i].runmode, subpath[i].distance, subpath[i].speed);
    }

    printf("****************************************\n");
    printf("after transform: \n");
    for(int i=0;i<subpathSize;i++)
    {
        if(subpath[i].runmode != RotateLeftMode && subpath[i].runmode != RotateRightMode)
        {
            //convert from relative coordinate to global coordinate
            subpath[i].target = agvMathUtils->reTransformCoordinate(targetpose, subpath[i].target);
            //convert from forklift to lidar coordinate
            subpath[i].target = agvMathUtils->transformPose(subpath[i].target, LIDAR_TO_TAIL_LENGTH);
        }

        printf("NO %d: target: (%d, %d, %f), mode: %d, distance: %d, speed: %d\n", i+1,
               subpath[i].target.xPos, subpath[i].target.yPos, subpath[i].target.angle,
               subpath[i].runmode, subpath[i].distance, subpath[i].speed);
    }

    printf("**********************************************\n");
    return 0;
}

#define  NAVIGATION_RADAR_TO_ANGLECHECK_DEVICE  150//O'A:L:mm
#define  DISTANCE_BEFORE_FORK                   300//BC:mm
#define  ANGLECHECK_DEVICE_TO_WHEELS            1960//O'D:d:mm
#define  LOADCAR_LENGTH                         500//mm
#define  RADAR_TO_TAIL                          2110//AD:mm

#define  MIN_DISTANCE_BEFORE_ADJUST             2200//mm
#define  MIN_ANGLE_DIFF                         1//degree
#define  MIN_DISTANCE_NOT_ADJUSTING             10//mm
#define  MAX_ADJUST_FORWARD_DISTANCE            100//mm

#define  ANGLE2RAD(x)                           ((x) * PI / 180)
#define  RAD2ANGLE(x)                           ((x) * 180 / PI)
/**
 * @brief AttitudeAdjustToFork
 * @param forkliftCurrentPos:point A(xA,yA,beta)(world coordinate)
 * @param loadCarPos:point C'(x'C,y'C)(relative coordinate)
 * @return
 */

int CAgvMoveHelperForklift::PosePackage(POSE pose,int x, int y, float angle)
{
    pose.xPos  = x;
    pose.yPos  = y;
    pose.angle = angle;
    return 0;
}

int CAgvMoveHelperForklift::AttitudeAdjustToFork(POSE forkliftCurrentPos, POSE loadCarPos)
{
    POSE positionBeforeFork        = {0, 0, 0.0};//(x'B,y'B)
    POSE positionBeforeForkInWorld = {0, 0, 0.0};//(xB,yB)
    POSE loadCarPosInWorld         = {0, 0, 0.0};//(xC,yC)
    POSE targetBeforeRotate        = {0, 0, 0.0};//(x'E,y'E)
    POSE targetBeforeRotateInWorld = {0, 0, 0.0};//(xE,yE)
    POSE subTarget                 = {0, 0, 0.0};

    float angle_BD_X      = 0.0;//theta:in the relative coordinate
    float angle_CB_X      = 0.0;//in the world coordinate
    float angle_CB_X_rela = 0.0;//in the relative coordinate
    float angleDiff       = 0.0;
    float angle_CO_X      = 0.0;

    int subIndex = 0;

    cout<<endl;
    cout<<"-------------------------AttituadeAdjustToFork():--------------------"<<endl<<endl;

    angle_CO_X = RAD2ANGLE(atan2(loadCarPos.yPos, loadCarPos.xPos));

    if((loadCarPos.angle < 0) || (45 < loadCarPos.angle && loadCarPos.angle < 135)
        || (loadCarPos.angle > 180) || (loadCarPos.yPos < 0))
    {
        cout<<"Angle Error!"<<endl;
        cout<<"loadCar Angle:  "<<loadCarPos.angle<<endl;
        return -1;
    }

    if(loadCarPos.yPos <= 0)
    {
        cout<<"The load-car position error! loadCarPos.yPos = "<<loadCarPos.yPos<<endl;
        return -1;
    }

    if(sqrt(loadCarPos.xPos * loadCarPos.xPos + loadCarPos.yPos * loadCarPos.yPos) < MIN_DISTANCE_BEFORE_ADJUST)
    {
        cout<<"The loadCarPosition is too near the forklift! distance:  "
            <<sqrt(loadCarPos.xPos * loadCarPos.xPos + loadCarPos.yPos * loadCarPos.yPos)<<endl;
        return -1;
    }

    cout<<endl;
    cout<<"Adjust to fork starting.."<<endl;
    cout<<"forkliftCurrentPos:A("<<forkliftCurrentPos.xPos<<", "
                                 <<forkliftCurrentPos.yPos<<", "
                                 <<forkliftCurrentPos.angle<<")"<<endl<<endl;

    cout<<"loadCarPos:C'("<<loadCarPos.xPos<<", "
                          <<loadCarPos.yPos<<", "
                          <<loadCarPos.angle<<")"<<endl<<endl;

    clearSubpath();

    if(0 <= loadCarPos.angle && loadCarPos.angle <= 90)//(x'B,y'B)
    {
        positionBeforeFork.xPos = loadCarPos.xPos + DISTANCE_BEFORE_FORK * sin(ANGLE2RAD(loadCarPos.angle));
        positionBeforeFork.yPos = loadCarPos.yPos - DISTANCE_BEFORE_FORK * cos(ANGLE2RAD(loadCarPos.angle));
    }
    else if(90 < loadCarPos.angle && loadCarPos.angle <= 180)//(x'B,y'B)
    {
        positionBeforeFork.xPos = loadCarPos.xPos - DISTANCE_BEFORE_FORK * sin(ANGLE2RAD(loadCarPos.angle));
        positionBeforeFork.yPos = loadCarPos.yPos + DISTANCE_BEFORE_FORK * cos(ANGLE2RAD(loadCarPos.angle));
    }

    cout<<"B'("<<positionBeforeFork.xPos<<", "
              <<positionBeforeFork.yPos<<", "
              <<positionBeforeFork.angle<<")"<<endl;

    positionBeforeForkInWorld = Relative2WorldCoordinate(positionBeforeFork, forkliftCurrentPos);//B
    loadCarPosInWorld         = Relative2WorldCoordinate(loadCarPos, forkliftCurrentPos);//C
    loadCarPosInWorld.angle   = RAD2ANGLE(atan2(loadCarPosInWorld.yPos - positionBeforeForkInWorld.yPos,
                                                loadCarPosInWorld.xPos - positionBeforeForkInWorld.xPos));
    cout<<"world-coordinate: B("<<positionBeforeForkInWorld.xPos<<", "
                                <<positionBeforeForkInWorld.yPos<<", "
                                <<positionBeforeForkInWorld.angle<<")"<<endl;
    //cout<<"world-coordinate: C("<<loadCarPosInWorld.xPos<<", "
    //                            <<loadCarPosInWorld.yPos<<", "
    //                            <<loadCarPosInWorld.angle<<")"<<endl;

    //calculate the angle B'_D'_X in the relative coordinate.
    angle_BD_X = RAD2ANGLE(atan2(positionBeforeFork.yPos - ANGLECHECK_DEVICE_TO_WHEELS,
                                 positionBeforeFork.xPos));

    //calculate the angle C_B_X in the world coordinate.
    angle_CB_X = RAD2ANGLE(atan2(loadCarPosInWorld.yPos - positionBeforeForkInWorld.yPos,
                                 loadCarPosInWorld.xPos - positionBeforeForkInWorld.xPos));
    //calculate the angle C'_B'_X in the relative coordinate.
    angle_CB_X_rela = RAD2ANGLE(atan2(loadCarPos.yPos - positionBeforeFork.yPos,
                                      loadCarPos.xPos - positionBeforeFork.xPos));

    angleDiff  = fabs(angle_BD_X - 90);//re

    if(angle_CB_X < 0)//wor
    {
        angle_CB_X = 360 + angle_CB_X;
    }

    //Calculate the angle which the forklift final forks (in the world coordinate).
    loadCarPosInWorld.angle = 180 + angle_CB_X;

    if(loadCarPosInWorld.angle >= 360)
    {
        loadCarPosInWorld.angle = loadCarPosInWorld.angle - 360;
    }

    cout<<"loadCarPosInWorld: C("<<loadCarPosInWorld.xPos<<", "
                                 <<loadCarPosInWorld.yPos<<", "
                                 <<loadCarPosInWorld.angle<<")"<<endl;

    loadCarPosInWorld = CAgvMathUtils::transformPose(loadCarPosInWorld, -LOADCAR_LENGTH);//F

    cout<<"position to fork :F("<<loadCarPosInWorld.xPos<<", "
                                <<loadCarPosInWorld.yPos<<", "
                                <<loadCarPosInWorld.angle<<")"<<endl;

    cout<<endl;
    cout<<"world:angle_CB_X: "<<angle_CB_X<<endl<<endl;
    cout<<"relative:angle_B'D'_X: "<<angle_BD_X<<"    angleDiff: "<<angleDiff<<endl;
    cout<<endl;

    if(angleDiff <= MIN_ANGLE_DIFF && abs(loadCarPos.xPos) < MIN_DISTANCE_NOT_ADJUSTING)//situation 0
    {
        cout<<endl;
        cout<<"---------- situation 0 ----------"<<endl;
        cout<<"The angleDiff is too small,so the car just go back follow the straight line!"<<endl;
        cout<<"----step 1:backward to target:("<<loadCarPosInWorld.xPos<<", "
                                               <<loadCarPosInWorld.yPos<<", "
                                               <<loadCarPosInWorld.angle<<")"<<endl;
        cout<<endl;

        subpathSize = 1;
        subIndex    = 0;
        PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);

        setSubpath(subIndex++, subTarget, GoBackwardMode);
    }
    else
    {
        //load-car is in the right and line-CB is cross Y-axis
        if((loadCarPos.xPos > 0) && (0 < angle_CB_X_rela && angle_CB_X_rela < 90))
        {
            //calculating the cross-point of line-CB & Y-axis;(0, axisY_CB)(relative coordinate)
            targetBeforeRotate.yPos         = (int)Calculate2PointCrossYaxis(loadCarPos, positionBeforeFork);//E'
            cout<<"the cross E'="<<targetBeforeRotate.yPos<<endl;
            targetBeforeRotateInWorld       = Relative2WorldCoordinate(targetBeforeRotate, forkliftCurrentPos);//E
            targetBeforeRotateInWorld.angle = forkliftCurrentPos.angle;

            if(fabs(targetBeforeRotate.yPos - ANGLECHECK_DEVICE_TO_WHEELS) < MIN_DISTANCE_NOT_ADJUSTING)//situation 1
            {
                cout<<endl;
                cout<<"---------- situation 1 ----------"<<endl;
                cout<<"E is near of D:just rotating and backward:"<<endl;
                cout<<"----step 1:rotating right to:("<<forkliftCurrentPos.xPos<<", "
                                                      <<forkliftCurrentPos.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step 2:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                      <<loadCarPosInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 2;
                subIndex    = 0;
                PosePackage(subTarget,forkliftCurrentPos.xPos, forkliftCurrentPos.yPos, loadCarPosInWorld.angle);
                subTarget   = CAgvMathUtils::transformPose(subTarget, -RADAR_TO_TAIL);
                setSubpath(subIndex++, subTarget, RotateRightMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
            else if(targetBeforeRotate.yPos > (ANGLECHECK_DEVICE_TO_WHEELS + MIN_DISTANCE_NOT_ADJUSTING)
                    && targetBeforeRotate.yPos < loadCarPos.yPos)//situation 2
            {
                cout<<endl;
                cout<<"---------- situation 2 ----------"<<endl;
                cout<<"E is ahead of D:Adjusting includes 3 steps:backward -> rotate right -> backward:"<<endl;
                cout<<"----step 1: backward to target("<<targetBeforeRotateInWorld.xPos<<", "
                                                       <<targetBeforeRotateInWorld.yPos<<", "
                                                       <<targetBeforeRotateInWorld.angle<<")"<<endl;
                cout<<"----step 2:rotate right to:("<<targetBeforeRotateInWorld.xPos<<", "
                                                    <<targetBeforeRotateInWorld.yPos<<", "
                                                    <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step 3:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                      <<loadCarPosInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 3;
                subIndex    = 0;
                subTarget   = targetBeforeRotateInWorld;
                setSubpath(subIndex++, subTarget, GoBackwardMode);
                PosePackage(subTarget,targetBeforeRotateInWorld.xPos, targetBeforeRotateInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, RotateRightMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
            else if(targetBeforeRotate.yPos < (ANGLECHECK_DEVICE_TO_WHEELS - MIN_DISTANCE_NOT_ADJUSTING)
                    && targetBeforeRotate.yPos > MAX_ADJUST_FORWARD_DISTANCE)//situation 3
            {
                cout<<endl;
                cout<<"---------- situation 3 ----------"<<endl;
                cout<<"E is behind of D:Adjusting includes 3 steps:forward -> rotate right -> backward:"<<endl;
                cout<<"----step 1: forward to target("<<targetBeforeRotateInWorld.xPos<<", "
                                                      <<targetBeforeRotateInWorld.yPos<<", "
                                                      <<targetBeforeRotateInWorld.angle<<")"<<endl;
                cout<<"----step 2:rotate right to:("<<targetBeforeRotateInWorld.xPos<<", "
                                                    <<targetBeforeRotateInWorld.yPos<<", "
                                                    <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step 3:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                      <<loadCarPosInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 3;
                subIndex    = 0;
                subTarget   = targetBeforeRotateInWorld;
                setSubpath(subIndex++, subTarget, GoForwardMode);
                PosePackage(subTarget,targetBeforeRotateInWorld.xPos, targetBeforeRotateInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, RotateRightMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
            else //situation 5
            {
                cout<<"situation 5: The length of O'E = "<<targetBeforeRotate.yPos<<endl<<endl;

                positionBeforeForkInWorld.angle = forkliftCurrentPos.angle - angleDiff;

                cout<<"forkliftCurrentPos.angle="<<forkliftCurrentPos.angle<<endl;
                cout<<"positionBeforeForkInWorld.angle="<<positionBeforeForkInWorld.angle<<endl<<endl;

                if(positionBeforeForkInWorld.angle < 0)
                {
                    positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle + 360;
                }
                else if(positionBeforeForkInWorld.angle > 360)
                {
                    positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle - 360;
                }

                cout<<"---------- situation 5 ----------"<<endl;
                cout<<"Adjusting includes 4 steps:rotate right -> backward -> rotate left -> backward:"<<endl;
                cout<<"----step1: rotate right(A) to :("<<forkliftCurrentPos.xPos<<", "
                                                        <<forkliftCurrentPos.yPos<<", "
                                                        <<positionBeforeForkInWorld.angle<<")"<<endl;
                cout<<"----stpe2:backward to subTarget:Point B("<<positionBeforeForkInWorld.xPos<<", "
                                                                <<positionBeforeForkInWorld.yPos<<", "
                                                                <<positionBeforeForkInWorld.angle<<")"<<endl;
                cout<<"----step3:rotate left(B) to:("<<positionBeforeForkInWorld.xPos<<", "
                                                     <<positionBeforeForkInWorld.yPos<<", "
                                                     <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step4:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                     <<loadCarPosInWorld.yPos<<", "
                                                     <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 4;
                subIndex    = 0;
                PosePackage(subTarget,forkliftCurrentPos.xPos, forkliftCurrentPos.yPos, loadCarPosInWorld.angle);
                subTarget   = CAgvMathUtils::transformPose(subTarget, -RADAR_TO_TAIL);
                setSubpath(subIndex++, subTarget, RotateRightMode);
                PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, positionBeforeForkInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
                PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, RotateLeftMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
        }//load-car is in the left and line-CB is cross Y-axis
        else if((loadCarPos.xPos < 0) && (90 < angle_CB_X_rela && angle_CB_X_rela < 180))
        {
            //calculating the cross-point of line-CB & Y-axis;(0, axisY_CB)(relative coordinate)
            targetBeforeRotate.yPos         = (int)Calculate2PointCrossYaxis(loadCarPos, positionBeforeFork);//E'
            cout<<"the cross E'="<<targetBeforeRotate.yPos<<endl;
            targetBeforeRotateInWorld       = Relative2WorldCoordinate(targetBeforeRotate, forkliftCurrentPos);//E
            targetBeforeRotateInWorld.angle = forkliftCurrentPos.angle;

            if(fabs(targetBeforeRotate.yPos - ANGLECHECK_DEVICE_TO_WHEELS) < MIN_DISTANCE_NOT_ADJUSTING)//situation 1'
            {
                cout<<endl;
                cout<<"---------- situation 1' ----------"<<endl;
                cout<<"E is near of D:just rotating and backward:"<<endl;
                cout<<"----step 1:rotating left to:("<<forkliftCurrentPos.xPos<<", "
                                                     <<forkliftCurrentPos.yPos<<", "
                                                     <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step 2:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                      <<loadCarPosInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 2;
                subIndex    = 0;
                PosePackage(subTarget,forkliftCurrentPos.xPos, forkliftCurrentPos.yPos, loadCarPosInWorld.angle);
                subTarget   = CAgvMathUtils::transformPose(subTarget, -RADAR_TO_TAIL);
                setSubpath(subIndex++, subTarget, RotateLeftMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
            else if(targetBeforeRotate.yPos > (ANGLECHECK_DEVICE_TO_WHEELS + MIN_DISTANCE_NOT_ADJUSTING)
                    && targetBeforeRotate.yPos < loadCarPos.yPos)//situation 2'
            {
                cout<<endl;
                cout<<"---------- situation 2' ----------"<<endl;
                cout<<"E is ahead of D:Adjusting includes 3 steps:backward -> rotate left -> backward:"<<endl;
                cout<<"----step 1: backward to target("<<targetBeforeRotateInWorld.xPos<<", "
                                                       <<targetBeforeRotateInWorld.yPos<<", "
                                                       <<targetBeforeRotateInWorld.angle<<")"<<endl;
                cout<<"----step 2:rotate left to:("<<targetBeforeRotateInWorld.xPos<<", "
                                                   <<targetBeforeRotateInWorld.yPos<<", "
                                                   <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step 3:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                      <<loadCarPosInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 3;
                subIndex    = 0;
                subTarget   = targetBeforeRotateInWorld;
                setSubpath(subIndex++, subTarget, GoBackwardMode);
                PosePackage(subTarget,targetBeforeRotateInWorld.xPos, targetBeforeRotateInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, RotateLeftMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
            else if(targetBeforeRotate.yPos < (ANGLECHECK_DEVICE_TO_WHEELS - MIN_DISTANCE_NOT_ADJUSTING)
                    && targetBeforeRotate.yPos > MAX_ADJUST_FORWARD_DISTANCE) //situation 3'
            {
                cout<<endl;
                cout<<"---------- situation 3' ----------"<<endl;
                cout<<"E is behind of D:Adjusting includes 3 steps:forward -> rotate left -> backward:"<<endl;
                cout<<"----step 1: forward to target("<<targetBeforeRotateInWorld.xPos<<", "
                                                      <<targetBeforeRotateInWorld.yPos<<", "
                                                      <<targetBeforeRotateInWorld.angle<<")"<<endl;
                cout<<"----step 2:rotate left to:("<<targetBeforeRotateInWorld.xPos<<", "
                                                   <<targetBeforeRotateInWorld.yPos<<", "
                                                   <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step 3:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                      <<loadCarPosInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 3;
                subIndex    = 0;
                subTarget   = targetBeforeRotateInWorld;
                setSubpath(subIndex++, subTarget, GoForwardMode);
                PosePackage(subTarget,targetBeforeRotateInWorld.xPos, targetBeforeRotateInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, RotateLeftMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }
            else //situation 5'
            {
                cout<<"situation 5': The length of O'E = "<<targetBeforeRotate.yPos<<endl<<endl;

                positionBeforeForkInWorld.angle = forkliftCurrentPos.angle - angleDiff;

                cout<<"forkliftCurrentPos.angle="<<forkliftCurrentPos.angle<<endl;
                cout<<"positionBeforeForkInWorld.angle="<<positionBeforeForkInWorld.angle<<endl<<endl;

                if(positionBeforeForkInWorld.angle < 0)
                {
                    positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle + 360;
                }
                else if(positionBeforeForkInWorld.angle > 360)
                {
                    positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle - 360;
                }

                cout<<"---------- situation 5' ----------"<<endl;
                cout<<"Adjusting includes 4 steps:rotate left -> backward -> rotate right -> backward:"<<endl;
                cout<<"----step1: rotate left(A) to :("<<forkliftCurrentPos.xPos<<", "
                                                       <<forkliftCurrentPos.yPos<<", "
                                                       <<positionBeforeForkInWorld.angle<<")"<<endl;
                cout<<"----stpe2:backward to subTarget:Point B("<<positionBeforeForkInWorld.xPos<<", "
                                                                <<positionBeforeForkInWorld.yPos<<", "
                                                                <<positionBeforeForkInWorld.angle<<")"<<endl;
                cout<<"----step3:rotate right(B) to:("<<positionBeforeForkInWorld.xPos<<", "
                                                      <<positionBeforeForkInWorld.yPos<<", "
                                                      <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<"----step4:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                     <<loadCarPosInWorld.yPos<<", "
                                                     <<loadCarPosInWorld.angle<<")"<<endl;
                cout<<endl;

                subpathSize = 4;
                subIndex    = 0;
                PosePackage(subTarget,forkliftCurrentPos.xPos, forkliftCurrentPos.yPos, positionBeforeForkInWorld.angle);
                subTarget   = CAgvMathUtils::transformPose(subTarget, -RADAR_TO_TAIL);
                setSubpath(subIndex++, subTarget, RotateLeftMode);
                PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, positionBeforeForkInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
                PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, RotateRightMode);
                PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
                setSubpath(subIndex++, subTarget, GoBackwardMode);
            }

        }
        else if(angle_BD_X < 90)//situation 4
        {
            positionBeforeForkInWorld.angle = forkliftCurrentPos.angle - angleDiff;

            cout<<"forkliftCurrentPos.angle="<<forkliftCurrentPos.angle<<endl;
            cout<<"positionBeforeForkInWorld.angle="<<positionBeforeForkInWorld.angle<<endl<<endl;

            if(positionBeforeForkInWorld.angle < 0)
            {
                positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle + 360;
            }
            else if(positionBeforeForkInWorld.angle > 360)
            {
                positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle - 360;
            }

            cout<<"---------- situation 4 ----------"<<endl;
            cout<<"Adjusting includes 4 steps:rotate right -> backward -> rotate left -> backward:"<<endl;
            cout<<"----step1: rotate right(A) to :("<<forkliftCurrentPos.xPos<<", "
                                                    <<forkliftCurrentPos.yPos<<", "
                                                    <<positionBeforeForkInWorld.angle<<")"<<endl;
            cout<<"----stpe2:backward to subTarget:Point B("<<positionBeforeForkInWorld.xPos<<", "
                                                            <<positionBeforeForkInWorld.yPos<<", "
                                                            <<positionBeforeForkInWorld.angle<<")"<<endl;
            cout<<"----step3:rotate left(B) to:("<<positionBeforeForkInWorld.xPos<<", "
                                                 <<positionBeforeForkInWorld.yPos<<", "
                                                 <<loadCarPosInWorld.angle<<")"<<endl;
            cout<<"----step4:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                 <<loadCarPosInWorld.yPos<<", "
                                                 <<loadCarPosInWorld.angle<<")"<<endl;
            cout<<endl;

            subpathSize = 4;
            subIndex    = 0;
            PosePackage(subTarget,forkliftCurrentPos.xPos, forkliftCurrentPos.yPos, positionBeforeForkInWorld.angle);
            subTarget   = CAgvMathUtils::transformPose(subTarget, -RADAR_TO_TAIL);
            setSubpath(subIndex++, subTarget, RotateRightMode);
            PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, positionBeforeForkInWorld.angle);
            setSubpath(subIndex++, subTarget, GoBackwardMode);
            PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, loadCarPosInWorld.angle);
            setSubpath(subIndex++, subTarget, RotateLeftMode);
            PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
            setSubpath(subIndex++, subTarget, GoBackwardMode);
        }
        else if(angle_BD_X > 90)//situation 4'
        {
            positionBeforeForkInWorld.angle = forkliftCurrentPos.angle + angleDiff;

            cout<<"forkliftCurrentPos.angle="<<forkliftCurrentPos.angle<<endl;
            cout<<"positionBeforeForkInWorld.angle="<<positionBeforeForkInWorld.angle<<endl;

            if(positionBeforeForkInWorld.angle < 0)
            {
                positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle + 360;
            }
            else if(positionBeforeForkInWorld.angle > 360)
            {
                positionBeforeForkInWorld.angle = positionBeforeForkInWorld.angle - 360;
            }

            cout<<"---------- situation 4' ----------"<<endl;
            cout<<"Adjusting includes 4 steps:rotate left -> backward -> rotate right -> backward:"<<endl;
            cout<<"----step1: rotate left(A) to :("<<forkliftCurrentPos.xPos<<", "
                                                   <<forkliftCurrentPos.yPos<<", "
                                                   <<positionBeforeForkInWorld.angle<<")"<<endl;
            cout<<"----stpe2:backward to subTarget:Point B("<<positionBeforeForkInWorld.xPos<<", "
                                                            <<positionBeforeForkInWorld.yPos<<", "
                                                            <<positionBeforeForkInWorld.angle<<")"<<endl;
            cout<<"----step3:rotate right(B) to:("<<positionBeforeForkInWorld.xPos<<", "
                                                  <<positionBeforeForkInWorld.yPos<<", "
                                                  <<loadCarPosInWorld.angle<<")"<<endl;
            cout<<"----step4:backward to target("<<loadCarPosInWorld.xPos<<", "
                                                 <<loadCarPosInWorld.yPos<<", "
                                                 <<loadCarPosInWorld.angle<<")"<<endl;
            cout<<endl;

            subpathSize = 4;
            subIndex    = 0;
            PosePackage(subTarget,forkliftCurrentPos.xPos, forkliftCurrentPos.yPos, positionBeforeForkInWorld.angle);
            subTarget   = CAgvMathUtils::transformPose(subTarget, -RADAR_TO_TAIL);
            setSubpath(subIndex++, subTarget, RotateLeftMode);
            PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, positionBeforeForkInWorld.angle);
            setSubpath(subIndex++, subTarget, GoBackwardMode);
            PosePackage(subTarget,positionBeforeForkInWorld.xPos, positionBeforeForkInWorld.yPos, loadCarPosInWorld.angle);
            setSubpath(subIndex++, subTarget, RotateRightMode);
            PosePackage(subTarget,loadCarPosInWorld.xPos, loadCarPosInWorld.yPos, loadCarPosInWorld.angle);
            setSubpath(subIndex++, subTarget, GoBackwardMode);
        }
    }

    cout<<endl;
    //retranform to radar position:
    for(int i=0;i<subpathSize;i++)
    {
        //if(subpath[i].runmode != RotateLeftMode && subpath[i].runmode != RotateRightMode)
        //{
            //convert from forklift to lidar coordinate
            subpath[i].target = CAgvMathUtils::transformPose(subpath[i].target, RADAR_TO_TAIL);
            subpath[i].speed  = -BASE_LINEAR_SPEED;
        //}

        cout<<"subpath["<<i<<"]: ("<<subpath[i].target.xPos<<", "
                                   <<subpath[i].target.yPos<<", "
                                   <<subpath[i].target.angle<<")"
                                   <<"   runMode:"<<subpath[i].runmode<<endl;
    }
    cout<<endl;

    return 0;
}

/**
 * @brief Relative2WorldCoordinate
 * @param relativePos:(x',y')
 * @param forkliftPoseInWorld:(xA,yA,beta)
 * @return
 */
POSE CAgvMoveHelperForklift::Relative2WorldCoordinate(POSE relativePos, POSE forkliftPoseInWorld)
{
    POSE worldPos;//(x,y)
    POSE relativeOrigin;//(x0,y0)
    float rotateAngle;

    rotateAngle = 90 + forkliftPoseInWorld.angle;

    //cout<<endl;
    //cout<<"--------Relative2WorldCoordanate()---------------"<<endl;

    if(rotateAngle > 360)
    {
        rotateAngle = rotateAngle - 360;
    }

    //cout<<"rotateAngle:"<<rotateAngle<<endl;

    float sinBeta = sin(ANGLE2RAD(rotateAngle));
    float cosBeta = cos(ANGLE2RAD(rotateAngle));

    //cout<<"sinBeta:"<<sinBeta<<"  "<<"cosBeta:"<<cosBeta<<endl;

    relativeOrigin.xPos = forkliftPoseInWorld.xPos - NAVIGATION_RADAR_TO_ANGLECHECK_DEVICE * cos(ANGLE2RAD(forkliftPoseInWorld.angle));
    relativeOrigin.yPos = forkliftPoseInWorld.yPos - NAVIGATION_RADAR_TO_ANGLECHECK_DEVICE * sin(ANGLE2RAD(forkliftPoseInWorld.angle));

    //cout<<"relativeOrigin:O'("<<relativeOrigin.xPos<<", "
    //                          <<relativeOrigin.yPos<<")"<<endl;

    worldPos.xPos = relativePos.xPos * cosBeta - relativePos.yPos * sinBeta + relativeOrigin.xPos;
    worldPos.yPos = relativePos.xPos * sinBeta + relativePos.yPos * cosBeta + relativeOrigin.yPos;

    //cout<<"relavitePos ("<<relativePos.xPos<<", "<<relativePos.yPos<<")"<<"   "
    //      "worldPos ("<<worldPos.xPos<<", "<<worldPos.yPos<<")"<<endl;

    return worldPos;
}

/**
 * @brief  Calculate2PointCrossYaxis
 * @param  pointA
 * @param  pointB
 * @return y of Y-position
 */
float CAgvMoveHelperForklift::Calculate2PointCrossYaxis(POSE pointA, POSE pointB)
{
    if(pointA.xPos == pointB.xPos)
    {
        cout<<"x1 == x2: retuen "<<DBL_MAX<<endl;
        return DBL_MAX;
    }

    return (pointB.yPos - (pointB.yPos -pointA.yPos) / (pointB.xPos - pointA.xPos) * pointB.xPos);
}












