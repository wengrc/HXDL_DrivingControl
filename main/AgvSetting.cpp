/*
 * AgvSetting.cpp
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#include "AgvSetting.h"

#include "../utils/AgvXmlSetting.h"

extern const char *VERSION;

CAgvSetting *CAgvSetting::instance = NULL;


CAgvSetting::CAgvSetting():CAgvXmlSetting()
{
    Version = VERSION;
    Disable = false;
}

CAgvSetting::~CAgvSetting()
{
}

CAgvSetting &CAgvSetting::Instance()
{
    if (instance == NULL)
    {
        instance = new CAgvSetting();
    }
    return *instance;
}

int CAgvSetting::LoadAll(const char *fname)
{
    LoadContents(fname);

    alarmVoltage            = GetSectionValue("AlarmVoltage",   24000);
    maxTimeOfLostPos        = GetSectionValue("MaxTimeOfLostPos", 150);
    Id                      = GetSectionValue("Id",             "1001");
    Pid                     = GetSectionValue("Pid",            "MF01161001001");
    Version                 = GetSectionValue("Version",        VERSION);
    Disable                 = GetSectionValue("Disable",        false);
    Speed                   = GetSectionValue("Speed",          50);
    Torison                 = GetSectionValue("Torison",        1000);
    MaxAngle                = GetSectionValue("MaxAngle",       9000);
    UltraSonicMinDistance   = GetSectionValue("UltraSonicMinDistance", 300);
    UltraSonicMidDistance   = GetSectionValue("UltraSonicMidDistance", 600);
    UltraSonicMaxDistance   = GetSectionValue("UltraSonicMaxDistance", 800);
    AvoidMinDistance        = GetSectionValue("AvoidMinDistance",   500);
    AvoidMidDistance        = GetSectionValue("AvoidMidDistance",   800);
    AvoidMaxDistance        = GetSectionValue("AvoidMaxDistance",   1000);
    IsStopCarForScheOffLine = GetSectionValue("IsStopCarForScheOffLine",    0);
    Volume                  = GetSectionValue("Volume",         5);
    WifiError               = GetSectionValue("WifiError",      false);
    Essid                   = GetSectionValue("Essid",          "ZJAGV");
    WifiEnable              = GetSectionValue("WifiEnable",     0);
    ApEnable                = GetSectionValue("ApEnable",       0);
    EthernetEnable          = GetSectionValue("EthernetEnable", 1);
    ModemEnable             = GetSectionValue("ModemEnable",    0);

    //2018-03-29
    //UseDhcp                 = GetSectionValue("UseDhcp",        0);
    UseDhcp                 = false;

    LocalIp                 = GetSectionValue("LocalIp",        "192.168.1.222");
    ServerIp                = GetSectionValue("ServerIp",       "192.168.1.11");
    ServerPort              = GetSectionValue("ServerPort",     8623);
    BatteryType             = GetSectionValue("BatteryType",    1);
    ForwardSpeed            = GetSectionValue("ForwardSpeed",   900);
    BackwardSpeed           = GetSectionValue("BackwardSpeed",  650);
    TurningSpeed            = GetSectionValue("TurningSpeed",   400);
    IsAutoLoop              = GetSectionValue("IsAutoLoop", 0);
    PathPointCount          = GetSectionValue("PathPointCount", 0);
    TargetPointId           = GetSectionValue("TargetPointId",  0);
    LastPointId             = GetSectionValue("LastPointId",    -1);
    LastStationPoint        = GetSectionValue("LastStationPoint",    1);
    RepairMode              = GetSectionValue("RepairMode",0);
    NavigatorOffset         = GetSectionValue("NavigatorOffset",0000);


    SteerAngleOffset        = GetSectionValue("SteerAngleOffset",(double)3.48);
    RotateLeftSteerAngleOffset = GetSectionValue("RotateLeftSteerAngleOffset",(double)0);
    RotateRightSteerAngleOffset = GetSectionValue("RotateRightSteerAngleOffset",(double)2.3);
    DelayAngleOffset        = GetSectionValue("DelayAngleOffset",(double)7.0);
    DelayArcDistanceOffset  = GetSectionValue("DelayArcDistanceOffset",60);
    DelayDistanceOffset     = GetSectionValue("DelayDistanceOffset",160);
    StandardRadius          = GetSectionValue("StandardRadius",1500);
    AngleOffset             = GetSectionValue("AngleOffset",(double)0);
    LidarLeftRightOffsetMM  = GetSectionValue("LidarLeftRightOffsetMM",10);

    CarWheelRadius          = GetSectionValue("CarWheelRadius",130);
    CarWheelBase            = GetSectionValue("CarWheelBase",1200);
    GearRedutionRatio       = GetSectionValue("GearRedutionRatio",(double)14);
    LidarToTailLength       = GetSectionValue("LidarToTailLength",842);
    return 0;
}

int CAgvSetting::SaveAll()
{
    SaveSectionValue("Id",                      Id.c_str());
    SaveSectionValue("Pid",                     Pid.c_str());
    SaveSectionValue("Version",                 Version.c_str());
    SaveSectionValue("Disable",                 Disable);
    SaveSectionValue("Speed",                   Speed);
    SaveSectionValue("Torison",                 Torison);
    SaveSectionValue("MaxAngle",                MaxAngle);
    SaveSectionValue("UltraSonicMinDistance",   UltraSonicMinDistance);
    SaveSectionValue("UltraSonicMidDistance",   UltraSonicMidDistance);
    SaveSectionValue("UltraSonicMaxDistance",   UltraSonicMaxDistance);
    SaveSectionValue("AvoidMinDistance",        AvoidMinDistance);
    SaveSectionValue("AvoidMidDistance",        AvoidMidDistance);
    SaveSectionValue("AvoidMaxDistance",        AvoidMaxDistance);
    SaveSectionValue("IsStopCarForScheOffLine", IsStopCarForScheOffLine);
    SaveSectionValue("Volume",                  Volume);
    SaveSectionValue("Essid",                   Essid.c_str());
    SaveSectionValue("WifiEnable",              WifiEnable);
    SaveSectionValue("ApEnable",                ApEnable);
    SaveSectionValue("EthernetEnable",          EthernetEnable);
    SaveSectionValue("ModemEnable",             ModemEnable);
    SaveSectionValue("UseDhcp",                 UseDhcp);
    SaveSectionValue("WifiError",               WifiError);
    SaveSectionValue("LocalIp",                 LocalIp.c_str());
    SaveSectionValue("ServerIp",                ServerIp.c_str());
    SaveSectionValue("ServerPort",              ServerPort);

    SaveSectionValue("BatteryType",             BatteryType);

    SaveSectionValue("ForwardSpeed",            ForwardSpeed);
    SaveSectionValue("BackwardSpeed",           BackwardSpeed);
    SaveSectionValue("TurningSpeed",            TurningSpeed);

    SaveSectionValue("IsAutoLoop",              IsAutoLoop);

    SaveSectionValue("PathPointCount",          PathPointCount);
    SaveSectionValue("TargetPointId",           TargetPointId);
    SaveSectionValue("LastPointId",             LastPointId);
    SaveSectionValue("LastStationPoint",        LastStationPoint);

    return 0;
}

