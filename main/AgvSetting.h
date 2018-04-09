/*
 * AgvSetting.h
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#ifndef AGVSETTING_H_
#define AGVSETTING_H_

#include <string>
#include <map>
#include <iostream>

#include "../utils/AgvXmlSetting.h"

using namespace std;

class CAgvSetting : public CAgvXmlSetting
{
    public:
        static CAgvSetting &Instance();

        virtual ~CAgvSetting();

        int LoadAll(const char *fname);

        int SaveAll();

    public:
        string      Id;
        string      Pid;
        string      Version;
        bool        Disable;
        int         Speed;
        int         Torison;
        int         MaxAngle;
        int         UltraSonicMinDistance;
        int         UltraSonicMidDistance;
        int         UltraSonicMaxDistance;
        int         AvoidMinDistance;
        int         AvoidMidDistance;
        int         AvoidMaxDistance;
        int         IsStopCarForScheOffLine;
        int         Volume;
        bool        WifiEnable;
        bool        ApEnable;
        bool        EthernetEnable;
        bool        ModemEnable;
        bool        UseDhcp;
        string      Essid;
        bool        WifiError;
        string      LocalIp;
        string      ServerIp;
        int         ServerPort;
        int         BatteryType;
        int         alarmVoltage;
        int         maxTimeOfLostPos;

        int         ForwardSpeed;
        int         BackwardSpeed;
        int         TurningSpeed;

        bool        IsAutoLoop;
        int         PathPointCount;
        int         TargetPointId;
        int         LastPointId;
        int         LastStationPoint;
        int         RepairMode;
        int         NavigatorOffset;
        float       SteerAngleOffset;
        float       RotateLeftSteerAngleOffset;
        float       RotateRightSteerAngleOffset;
        float       DelayAngleOffset;
        int         DelayArcDistanceOffset;
        int         DelayDistanceOffset;
        int         StandardRadius;
        float       AngleOffset;
        int         LidarLeftRightOffsetMM;
        int         CarWheelRadius;
        int         CarWheelBase;
        float       GearRedutionRatio;
        int         LidarToTailLength;


    private:
        CAgvSetting();

        static CAgvSetting *instance;
};



#endif /* AGVSETTING_H_ */
