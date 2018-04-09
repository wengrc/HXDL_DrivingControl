/*
 * AgvSteeringBaseControler.h
 *
 *  Created on: 2017-6-20
 *      Author: zxq
 */

#ifndef AGVSTEERINGBASECONTROLER_H_
#define AGVSTEERINGBASECONTROLER_H_

#include "AgvBaseControler.h"

class CAgvSteeringBaseControler : public CAgvBaseControler
{
    public:
        CAgvSteeringBaseControler();

        virtual ~CAgvSteeringBaseControler();

        int Init();

        int RunForward(int speed);

        int RollBack(int speed);

        int TurnLeft(int speed);

        int TurnRight(int speed);

        int RotateLeft(int speed);

        int RotateRight(int speed);

        int SetCarSpeed(int linearSpeed, float angularSpeed);

        int StopCar(bool enbrake);

        int SetEnable(bool enable);

        int CheckError(void);

        int CheckSpeed(void);

        int SetAccSpeed(int accSpeed);

        int SetDecSpeed(int decSpeed);

        int SetBreak(bool enable);

        int GetCAN_ID(void);

    private:

        int SetSteeringSpeed(int speed);

        int SetSteeringAngle(float angle);

        float CalculateAngularSpeed(int speed, float angle);

        float CalculateWheelAngle(int speed, float angularSpeed);

        int MM_S_to_RPM(int speed_mm_s);

        int RPM_to_MM_S(int speed_rpm);

        float getTransformedAngle(float angle);


        CAgvSetting * setting;
        float         STEER_ANGLE_OFFSET;
        float         ROTATELEFT_STEERANGLE_OFFSET;
        float         ROTATERIGHT_STEERANGLE_OFFSET;
        int           CAN_ID;
};

#endif /* AGVSTEERINGBASECONTROLER_H_ */
