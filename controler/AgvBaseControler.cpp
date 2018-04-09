/*
 * AgvBaseControler.cpp
 *
 *  Created on: 2017-6-19
 *      Author: zxq
 */

#include "AgvBaseControler.h"

CAgvBaseControler::CAgvBaseControler(int type)
{
    driveType = type;
    targetLineSpeed = 0;
    targetAngulaSpeed = 0;
    currentLineSpeed = 0;
    currentAngularSpeed = 0;
    currentAccSpeed = 0;
    currentDecSpeed = 0;
    currentDirection = 0;
    errorNum = 0;

    wheelSeperation = CarWheelSperation;
    controler = CAgvControler::Instance();

    setting = &CAgvSetting::Instance();
    wheelRadius = setting->CarWheelRadius;
    printf("xml---------> wheelRadius = %d\n",wheelRadius);
    wheelBase = setting->CarWheelBase;
    printf("xml---------> wheelBase = %d\n",wheelBase);
    gearRatio = setting->GearRedutionRatio;
    printf("xml---------> gearRatio = %f\n",gearRatio);
}

CAgvBaseControler::~CAgvBaseControler()
{
    // TODO Auto-generated destructor stub
}

