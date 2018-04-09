/*
 * AgvSteeringBaseControler.cpp
 *
 *  Created on: 2017-6-20
 *      Author: zxq
 */

#include "AgvSteeringBaseControler.h"

#define LOGTAG  "SteerBase"


CAgvSteeringBaseControler::CAgvSteeringBaseControler() :
        CAgvBaseControler(CAgvBaseControler::SteeringDrive)
{
    setting = &CAgvSetting::Instance();
    STEER_ANGLE_OFFSET = setting->SteerAngleOffset;
    LogInfo(LOGTAG, "xml---------> STEER_ANGLE_OFFSET = %f\n",STEER_ANGLE_OFFSET);
    ROTATELEFT_STEERANGLE_OFFSET = setting->RotateLeftSteerAngleOffset;
    LogInfo(LOGTAG, "xml---------> ROTATELEFT_STEERANGLE_OFFSET = %f\n",ROTATELEFT_STEERANGLE_OFFSET);
    ROTATERIGHT_STEERANGLE_OFFSET = setting->RotateRightSteerAngleOffset;
    LogInfo(LOGTAG, "xml---------> ROTATERIGHT_STEERANGLE_OFFSET = %f\n",ROTATERIGHT_STEERANGLE_OFFSET);
    CAN_ID = atoi(CAgvSetting::Instance().Id.c_str());
}

CAgvSteeringBaseControler::~CAgvSteeringBaseControler()
{
}

int CAgvSteeringBaseControler::Init()
{
    if (controler == NULL)
    {
        return -1;
    }
    int ret = controler->ResetDistance();
    return ret;
}

int CAgvSteeringBaseControler::RunForward(int speed)
{
    if (controler == NULL)
    {
        return -1;
    }
    int ret = controler->SetDirection(0);
    if (ret != 0)
    {
        LogError(LOGTAG, "Set direction forward failed!:%d",ret);
        return -1;
    }

    ret = controler->SetBreak(false);
//    printf("ret: %d\n", ret);
    if (ret != 0)
    {
        LogError(LOGTAG, "Set break invalid failed!:%d",ret);
        return -1;
    }

    int speed_rpm = MM_S_to_RPM(speed);
//    printf("speed_rpm: %d\n", speed_rpm);

    ret = controler->SetSpeed(speed_rpm);
//    printf("ret: %d\n", ret);

    currentDirection = 1;
    currentLineSpeed = speed;
    currentAngularSpeed = CalculateAngularSpeed(currentLineSpeed, controler->angle);

    return ret;
}

int CAgvSteeringBaseControler::RollBack(int speed)
{
    if (controler == NULL)
    {
        return -1;
    }
    int ret = controler->SetDirection(1);
    if (ret != 0)
    {
        LogError(LOGTAG, "Set direction backward failed!:%d",ret);
        return -1;
    }

    int speed_rpm = MM_S_to_RPM(speed);
    ret = controler->SetSpeed(speed_rpm);
    ret = controler->SetBreak(false);

    currentDirection = 2;
    currentLineSpeed = 0.0 - speed;
    currentAngularSpeed = CalculateAngularSpeed(currentLineSpeed, controler->angle);

    return ret;
}

int CAgvSteeringBaseControler::TurnLeft(int speed)
{
    if (SetSteeringAngle(ANGLE_90_DEGREE) < 0)
    {
        return -1;
    }

    return RunForward(speed);
}

int CAgvSteeringBaseControler::TurnRight(int speed)
{
    if (SetSteeringAngle(-ANGLE_90_DEGREE) < 0)
    {
        return -1;
    }

    return RunForward(speed);

}

int CAgvSteeringBaseControler::RotateLeft(int speed)
{
    if (SetSteeringAngle(ANGLE_90_DEGREE + ROTATELEFT_STEERANGLE_OFFSET) < 0)
    {
        return -1;
    }
    usleep(600*1000);//wrc

//    if (SetSteeringAngle(88.4) < 0)
//    {
//        return -1;
//    }

    return RunForward(speed);
}

int CAgvSteeringBaseControler::RotateRight(int speed)
{
    //if (SetSteeringAngle(-ANGLE_90_DEGREE - STEER_ANGLE_OFFSET/2.5) < 0) //wrc: for behavior like rotateleft.//1#car
    if (SetSteeringAngle(-ANGLE_90_DEGREE + ROTATERIGHT_STEERANGLE_OFFSET) < 0) //2#car
    {
        return -1;
    }
    usleep(600*1000);//wrc
    return RunForward(speed);
}

/*
 * speed: unit, rpm;
 *
 *
 *
 * */
int CAgvSteeringBaseControler::SetSteeringSpeed(int speed)
{
    if (controler == NULL)
    {
        return -1;
    }

    return controler->SetSpeed(speed);
}

/*
 * angle: unit, degree
 *
 * */

int CAgvSteeringBaseControler::SetSteeringAngle(float angle)
{
    if (controler == NULL)
    {
        return -1;
    }

    angle = angle + STEER_ANGLE_OFFSET;

    if (angle < -ANGLE_90_DEGREE)
    {
        angle = -ANGLE_90_DEGREE;
    }
    else if (angle > ANGLE_90_DEGREE)
    {
        angle = ANGLE_90_DEGREE;
    }

    printf("Rightnow_angle:%f, Angle_plusOffset:%f. \n",controler->angle, angle);

    angle = getTransformedAngle(angle);


    if (angle < -ANGLE_90_DEGREE)
    {
        angle = -ANGLE_90_DEGREE;
    }
    else if (angle > ANGLE_90_DEGREE)
    {
        angle = ANGLE_90_DEGREE;
    }

    return controler->SetAngle(angle);
}

int CAgvSteeringBaseControler::RPM_to_MM_S(int speed_rpm)
{
    int speed_mm_s = 0;
    float wheelLength = 2 * PI * wheelRadius;

    // transform speed: mm/s to rpm
    speed_mm_s = speed_rpm * wheelLength / (60 * gearRatio);

    return speed_mm_s;
}

int CAgvSteeringBaseControler::MM_S_to_RPM(int speed_mm_s)
{
    int speed_rpm = 0;
    float wheelLength = 2 * PI * wheelRadius;

    // transform speed: mm/min to rpm
    speed_rpm = speed_mm_s * 60 * gearRatio / wheelLength;

    return speed_rpm;
}

int CAgvSteeringBaseControler::SetCarSpeed(int linearSpeed, float angularSpeed)
{
    if (controler == NULL)
    {
        return -1;
    }

    int ret = -1;

//    static int lastSetAngle = 0;
    float angle = 0.0;
    int speed = 0;

    angle = CalculateWheelAngle(linearSpeed, angularSpeed);
    printf("CalculateWheelAngle:%f  ", angle);
    targetAngulaSpeed = angularSpeed;
    ret = SetSteeringAngle(angle);

    targetLineSpeed = linearSpeed;
    // transform speed: mm/s to rpm
    speed = MM_S_to_RPM(linearSpeed);
    ret = SetSteeringSpeed(abs(speed));

//    if(targetAngulaSpeed != angularSpeed)
//    {
//        angle = CalculateWheelAngle(linearSpeed, angularSpeed);
//        printf("angle:%d\n", angle);
//        targetAngulaSpeed = angularSpeed;
//        ret = SetSteeringAngle((float)angle);
//    }
//    else
//    {
//        ret = 0;
//    }

//    if(abs(angle - lastSetAngle) >= ANGLE_1_DEGREE)
//    {
//        lastSetAngle = angle;
//        targetAngulaSpeed = angularSpeed;

//    }
//    else
//    {
//        ret = 0;
//    }

//    if(targetLineSpeed != linearSpeed)
//    {
//        targetLineSpeed = linearSpeed;

//        // transform speed: mm/s to rpm
//        speed = MM_S_to_RPM(linearSpeed);
//        ret = SetSteeringSpeed(abs(speed));
//    }
//    else
//    {
//        ret = 0;
//    }


//    targetLineSpeed = linearSpeed;
//    targetAngulaSpeed = angularSpeed;

//    angle = CalculateWheelAngle(linearSpeed, angularSpeed);

////    printf("angle: %f, speed: %d\n", angle, speed);

//    ret = SetSteeringAngle(angle);

////    printf("reta: %d\n", ret);

//    ret = SetSteeringSpeed(abs(speed));
////    printf("retb: %d\n", ret);

    return ret;
}

int CAgvSteeringBaseControler::StopCar(bool enbraker)
{

    if (controler == NULL)
    {
        return -1;
    }
    int ret = controler->SetSpeed(0);    
    if (enbraker)
    {
        controler->SetBreak(true);
    }
    targetLineSpeed = 0;
    targetAngulaSpeed = 0.0;

    currentDirection = 0;

    return ret;
}

int CAgvSteeringBaseControler::SetEnable(bool enable)
{
    if (controler == NULL)
    {
        return -1;
    }

    return controler->SetEnable(enable);
}

int CAgvSteeringBaseControler::CheckError(void)
{
    if (controler == NULL)
    {
        return -1;
    }
    errorNum =  controler->runErrorCode * 256 +  controler->turnErrorCode;
    return errorNum;
}

int CAgvSteeringBaseControler::CheckSpeed(void)
{
    if ( controler == NULL)
    {
        return -1;
    }
    int speed =  controler->speed;
    currentLineSpeed = RPM_to_MM_S(speed);
    currentAngularSpeed = CalculateAngularSpeed(currentLineSpeed,  controler->angle);
    currentDirection =  controler->direction;

    return speed;
}

int CAgvSteeringBaseControler::SetAccSpeed(int accSpeed)
{
    return 0;
}

int CAgvSteeringBaseControler::SetDecSpeed(int decSpeed)
{
    return 0;
}

int CAgvSteeringBaseControler::SetBreak(bool enable)
{
    if ( controler == NULL)
    {
        return -1;
    }

    int ret = controler->SetBreak(enable);

    return ret;
}

int CAgvSteeringBaseControler::GetCAN_ID(void)
{
    return CAN_ID;
}

/*input:
 * speed, unit: mm/s
 * angle, unit: degree
 *
 * output:
 * angularSpeed, unit:rad/s
 * */

float CAgvSteeringBaseControler::CalculateAngularSpeed(int speed, float angle)
{
    if (wheelBase == 0)
    {
        LogError(LOGTAG, "WheelBase is not set yet!");
        return 0.0;
    }

    if(angle > ANGLE_90_DEGREE || angle < -ANGLE_90_DEGREE)
    {
        LogError(LOGTAG, "input wheel angle error!");
    }
    float angularSpeed = 0.0;
    angularSpeed = speed * sin(angle * PI / ANGLE_180_DEGREE) / wheelBase;

    return angularSpeed;
}

/*input:
 * speed, unit: mm/s
 * angularSpeed, unit:rad/s
 *
 * output:
 * angle, unit: degree
 * */
float CAgvSteeringBaseControler::CalculateWheelAngle(int speed, float angularSpeed)
{
    if (wheelBase == 0)
    {
        LogError(LOGTAG, "WheelBase is not set yet!");
        return 0.0;
    }
    if (speed == 0)
    {
        LogError(LOGTAG, "LinearSpeed is zero!");
        return 0.0;
    }

    float angle = 0.0;

    if(angularSpeed * wheelBase / speed > 1.0)
    {
        angle = ANGLE_90_DEGREE;
    }
    else if(angularSpeed * wheelBase / speed < -1.0)
    {
        angle = -ANGLE_90_DEGREE;
    }
    else
    {
        angle = asin(angularSpeed * wheelBase / speed) * ANGLE_180_DEGREE / PI;
    }

//    printf("angle: %f\n", angle);

    return angle;

}

float CAgvSteeringBaseControler::getTransformedAngle(float angle)
{
    //from test data transform
    float set_Angle[25]= {-90,-30,-25,-23,-20,-15,-10,-8,-5,-3,-2,-1,0,1,2,3,5,8,10,15,20,23,25,30,90};

    //Number 1

    float feedback_Angle_Num1[25] = {-90.00,-30.10,-24.94,-22.79,-19.58,-14.22,-8.88,\
                                -6.72,-3.50,-1.36,-0.28,-0.08,-0.08,-0.08,0.12,\
                                1.18,3.34,6.56,8.72,14.06,19.42,22.64,24.78,29.88,90.00};

    //Number 2
    float feedback_Angle_Num2[25] = {-90.00,-29.96,-25.12,-23.10,-20.10,-15.10,-10.10,\
                               -8.10,-5.10,-3.10,-2.10,-1.14,-0.08,0.86,1.90,2.86,\
                               4.90,7.90,9.90,14.88,19.92,22.94,24.88,29.88,90.00};

    float feedback_Angle[25] = {0};

    if(CAN_ID == 1001)
    {
        memcpy(feedback_Angle,feedback_Angle_Num1,sizeof(float)*25);
    }
    else if(CAN_ID == 1002)
    {
        memcpy(feedback_Angle,feedback_Angle_Num2,sizeof(float)*25);
    }
    else
    {
        memcpy(feedback_Angle,feedback_Angle_Num2,sizeof(float)*25);//default value
    }

    int j=0;
    float angleShouldSet;

    if(angle <= -90.0)
    {
        angleShouldSet = -90.0;
        return angleShouldSet;
    }
    else if(angle >= 90.0)
    {
        angleShouldSet = 90.0;
        return angleShouldSet;
    }
    else
    {
        for(j=0; j<25; j++)
        {
            if((angle-feedback_Angle[j]) >= 0 && (angle-feedback_Angle[j+1]) < 0)
            {
                float k = (set_Angle[j]-set_Angle[j+1])/(feedback_Angle[j]-feedback_Angle[j+1]);
                angleShouldSet = k*(angle-feedback_Angle[j])+ set_Angle[j];
                break;
            }
        }
    }

    return angleShouldSet;

}
