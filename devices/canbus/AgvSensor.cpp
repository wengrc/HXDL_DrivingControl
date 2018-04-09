/*
 * AgvSensor.cpp
 *
 *  Created on: 2016-11-17
 *      Author: zxq
 */

#include "AgvSensor.h"
#include "../../AgvPublic.h"


CAgvControler *CAgvControler::instance = NULL;
CAgvLidarSensor *CAgvLidarSensor::instance = NULL;

CAgvCanSensor::CAgvCanSensor(int rid, int wid, int moduleid, const char* name) :
        CAgvSensor(name)
{
    canOperator = NULL;
    rCanId = rid;
    wCanId = wid;
    moduleId = moduleid;
    isAlive = false;
    heartBeatCounter = 0;
}

CAgvCanSensor::~CAgvCanSensor()
{
}

int CAgvCanSensor::Init(CAgvCanOperator *op)
{
    heartBeatCounter = 0;
    canOperator = op;
    isAlive = false;
    return 0;
}

int CAgvCanSensor::SendData(const unsigned char *buf, unsigned int len)
{
    return sendCanData(wCanId, buf, len);
}

int CAgvCanSensor::sendCanData(int canid, const unsigned char *buf,
                               unsigned int len)
{
    if (canOperator == NULL || canid <= 0)
    {
        return -1;
    }
    return canOperator->SendData(canid, buf, len);
}

int CAgvCanSensor::ParseData(const unsigned char *buf, unsigned int len)
{
    if (buf == NULL || len == 0)
    {
        return -1;
    }

    heartBeatCounter = 0;

    if (isAlive == false)
    {
        isAlive = true;
        LogInfo("SensorMangaer", "%s [0x%04X,0x%04X] is online now!",sensorName.c_str(),rCanId, wCanId);
    }

    if (len == 2 && buf[0] == 0xFF && buf[1] == 0xFF)
    {
        return 0;
    }
    else
    {
        return ParseCanData(buf, len);
    }
}

int CAgvCanSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
//    DEBUG_INFO("CAgvCanSensor", "ParseCanData");
    if (buf == NULL || len == 0)
    {
        return -1;
    }
    return 0;
}

int CAgvCanSensor::Close()
{
    return 0;
}

CAgvLidarSensor::CAgvLidarSensor() :
        CAgvCanSensor(LIDAR_RCANID, LIDAR_WCANID, 1, "LidarSensor")
{
    xPos = 0.0;
    yPos = 0.0;
    angle = 0.0;
    mode = 0;
    errorCode = 0;

    setting = &CAgvSetting::Instance();

    NavigatorOffset = setting->GetSectionValue("NavigatorOffset", 0000);
    printf("NavigatorOffset = %d\n",NavigatorOffset);
    LIDAR_LEFT_RIGHT_OFFSET_MM = setting->LidarLeftRightOffsetMM;
    ANGLE_OFFSET = setting ->AngleOffset;

}

CAgvLidarSensor::~CAgvLidarSensor()
{
}

bool CAgvLidarSensor::IsDataReady()
{
    if (mode == 1)
    {
        return true;
    }
    return false;
}

CAgvLidarSensor *CAgvLidarSensor::Instance()
{
    if (instance == NULL)
    {
        instance = new CAgvLidarSensor();
    }
    return instance;
}

bool CAgvLidarSensor::IsHaveError()
{
    if (!IsDataReady() || errorCode != 0)
    {
        return true;
    }
    return false;
}

int CAgvLidarSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    int i = 0;
    static int x = 0, y = 0, m = 0, err = 0;;
    static float a = 0.0;

    if(len == 8)
    {
        x = CAgvUtils::Buffer2Bin(buf + i, 3);
        i += 3;
        y = CAgvUtils::Buffer2Bin(buf + i, 3);
        i += 3;
        a = CAgvUtils::Buffer2Bin(buf + i, 2) / 100.0;

//        if(NavigatorOffset > 1000)
//        {
//            a = a - ((float)(NavigatorOffset-1000)/10);
//        }
//        else
//        {
//            a = a + (float)(NavigatorOffset)/10;
//        }

        if(a < 0)
        {
            a = a + 360;
        }
        i += 2;
    }

    x = x - LIDAR_LEFT_RIGHT_OFFSET_MM * sin(PI * a / ANGLE_180_DEGREE);
    y = y + LIDAR_LEFT_RIGHT_OFFSET_MM * cos(PI * a / ANGLE_180_DEGREE);
    a = a + ANGLE_OFFSET;
    if(a >= ANGLE_360_DEGREE)
    {
        a = a - ANGLE_360_DEGREE;
    }
    else if(a < 0 )
    {
        a = a + ANGLE_360_DEGREE;
    }

    if (x != xPos || y != yPos || a != angle)
    {
        xPos = x;
        yPos = y;
        angle = a;      

        static int count = 0;
        count++;
        if (count >= 5)
        {
            count = 0;

            LogInfo("LidarSensor", "Location changed: X:%d Y:%d Angle:%f\n",xPos,yPos,angle);
        }

        POSITION *pos = new POSITION;
        pos->xPos = xPos;
        pos->yPos = yPos;
        pos->angle = angle;

        SendImportantEvent(evPosition, 0, pos);
    }



    if(buf[0] == 0x02 && len == 3)
    {
        i += 1;

        m = CAgvUtils::Buffer2Bin(buf + i, 1);  i += 1;
        err = CAgvUtils::Buffer2Bin(buf + i, 1); i += 1;
    }

    if (m != mode)
    {
        mode = m;
        LogInfo("LidarSensor", "Lidar mode change to %d\n", m);
    }
    if (err != errorCode)
    {
        errorCode = err;
        LogInfo("LidarSensor", "Get a ErrorCode:%d\n", err);
    }


    return i;
}

CAgvGyroscopeSensor::CAgvGyroscopeSensor() :
        CAgvCanSensor(GROSCOPE_RCANID, GROSCOPE_WCANID, 1, "GyroscopeSensor")
{
    xAcceleration = 0.0;
    yAcceleration = 0.0;
    zAcceleration = 0.0;
    courseAngle = 0.0;
    pitchAngle = 0.0;
    heelAngle = 0.0;
}

CAgvGyroscopeSensor::~CAgvGyroscopeSensor()
{
}

int CAgvGyroscopeSensor::ParseCanData(const unsigned char *buf,
                                      unsigned int len)
{
    int i = 1;
    switch (buf[0])
    {
        case 0x02:
            if (len < 13)
            {
                return -1;
            }
            xAcceleration = CAgvUtils::Buffer2Bin(buf + i, 2);
            i += 2;
            yAcceleration = CAgvUtils::Buffer2Bin(buf + i, 2);
            i += 2;
            zAcceleration = CAgvUtils::Buffer2Bin(buf + i, 2);
            i += 2;
            courseAngle = CAgvUtils::Buffer2Bin(buf + i, 2) / 100.0;
            i += 2;
            pitchAngle = CAgvUtils::Buffer2Bin(buf + i, 2) / 100.0;
            i += 2;
            heelAngle = CAgvUtils::Buffer2Bin(buf + i, 2) / 100.0;
            i += 2;

            break;
        default:
            i = 0;
            break;
    }

    return i;
}


CAgvControler::CAgvControler() :
        CAgvCanSensor(CARCONTROLER_RCANID, CARCONTROLER_WANID, 2,
                      "CarControler")
{
    direction = 0;
    speed = 0;
    angle = 0.0;
    distance = 0;
    runErrorCode = 0;
    turnErrorCode = 0;
}

CAgvControler::~CAgvControler()
{
}

CAgvControler *CAgvControler::Instance()
{
    if (instance == NULL)
    {
        instance = new CAgvControler();
    }
    return instance;
}

int CAgvControler::SetEnable(bool enable)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x11;
    buf[i++] = enable;
    int ret = SendData(buf, i);
    return ret;
}

int CAgvControler::SetDirection(int direct)
{
//    if (this->direction == direct + 1)
//    {
//        printf("\n This situation occur!.\n");
//        return 0;
//    }

    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x12;
    buf[i++] = direct;
    int ret = SendData(buf, i);
    LogInfo("Controler", "Set direction from %d to %d : %d", this->direction, direct, ret);
    return ret;
}

int CAgvControler::SetAngle(float angle)
{
    if (this->angle == angle)
    {
        return 0;
    }
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x13;
    i += CAgvUtils::Bin2Buffer(angle * 100, 2, buf + i);
    int ret = SendData(buf, i);
    LogInfo("Controler", "Set Angle from %f to %f : %d %02X%02X ", this->angle, angle, ret, buf[1],buf[2]);
    return ret;
}

int CAgvControler::SetSpeed(int speed)
{
    if (this->speed == speed)
    {
        return 0;
    }
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x14;
    i += CAgvUtils::Bin2Buffer(speed, 2, buf + i);
    int ret = SendData(buf, i);
    //LogInfo("Controler", "Set speed from %d to %d : %d", this->speed, speed, ret);
    return ret;
}

int CAgvControler::SetBreak(bool enable)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x15;
    buf[i++] = enable;
    int ret = SendData(buf, i);
    LogInfo("Controler", "Set break %s : %d", enable ? "Open":"Close", ret);
    return ret;
}

int CAgvControler::ResetDistance()
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x16;
    int ret = SendData(buf, i);
    //LogInfo("Controler", "Reset distance: %d", ret);
    return ret;
}


int CAgvControler::ParseCanData(const unsigned char *buf, unsigned int len)
{

    int i = 0;
    switch (buf[0])
    {
        case 0x01:
        {
            if (len < 8)
            {
                return -1;
            }
            i = 1;
            direction = CAgvUtils::Buffer2Bin(buf + i, 1);
            i += 1;
            angle = ((short)CAgvUtils::Buffer2Bin(buf + i, 2)) / 100.0;
            i += 2;
            speed = CAgvUtils::Buffer2Bin(buf + i, 2);
            i += 2;
            distance = CAgvUtils::Buffer2Bin(buf + i, 2);
            i += 2;

            break;
        }
        case 0x02:
        {
            if (len < 2)
            {
                return -1;
            }
            i = 1;
            runErrorCode = buf[i++];
            turnErrorCode = buf[i++];
            break;
        }
        default:
            i = 0;
            break;
    }
    return i;
}

CAgvLifter::CAgvLifter() :
        CAgvCanSensor(LIFTER_RCANID, LIFTER_WCANID, 2, "LiftControler")
{
    liftStatus = UnKnown;
    currentHeight = 0;
    targetHeight = -1;
}

CAgvLifter::~CAgvLifter()
{
}

int CAgvLifter::SetTargetHeight(int height)
{
    targetHeight = height;
    return 0;
}

int CAgvLifter::SetLiftUp(int speed, int height)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x11;
    buf[i++] = speed;
    i += CAgvUtils::Bin2Buffer(height, 2, buf + i);
    //printf("UP!!!!!!!!!!!!\n");
    return SendData(buf, i);
}

int CAgvLifter::SetLiftDown(int speed, int height)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x12;
    buf[i++] = speed;
    i += CAgvUtils::Bin2Buffer(height, 2, buf + i);
    //printf("DOWN!!!!!!!!!!!!\n");
    return SendData(buf, i);
}

int CAgvLifter::StopLift()
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x13;
    return SendData(buf, i);
}

//2018-03-26
int CAgvLifter::InitLift()
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x17;
    return SendData(buf, i);
}

bool CAgvLifter::LiftIsArrived(int height)
{
    printf("TargetHeight = %d, CurrentHeight = %d\n",height,currentHeight);

    if(currentHeight > 60000)
    {
        currentHeight = 0;
    }
    if (abs(height - currentHeight) < 50)//yjw:0308
    {
        return true;
    }
    return false;
}

int CAgvLifter::GetLiftStatus()
{
    return liftStatus;
}

bool CAgvLifter::LiftIsAtTop()
{
    if(AtTop == liftStatus)
    {
        return true;
    }
    return false;
}

bool CAgvLifter::LiftIsAtBottom()
{
    if(AtBottom == liftStatus)
    {
        return true;
    }
    return false;
}

int CAgvLifter::ParseCanData(const unsigned char *buf, unsigned int len)
{
    int i = 1, lstatus, height;

    switch (buf[0])
    {
        case 0x01:
            if (len < 5)
            {
                return -1;
            }
            height = CAgvUtils::Buffer2Bin(buf + i, 2);    i += 2;
            lstatus = buf[i++];

            static int count = 0;
            count ++;
            if(count > 5)
            {
                count = 0;
                printf("***********hight = %d\r\n",height);
            }

            if (height != currentHeight)
            {
                currentHeight = height;
                if(targetHeight >= 0)
                {
                    if (LiftIsArrived(targetHeight))
                    {
                        SendEvent(evLiftArrived, targetHeight, NULL);
                    }
                }
            }

            if (lstatus == AtBottom && liftStatus != AtBottom)
            {
                liftStatus = AtBottom;
                printf("[%d]----------liftStatus-----------\n",liftStatus);
                SendEvent(evLiftAtBottom, targetHeight, NULL);
            }
            else if (lstatus == AtTop && liftStatus != AtTop)
            {
                liftStatus = AtTop;
                printf("[%d]----------liftStatus-----------\n",liftStatus);
                SendEvent(evLiftArrived, targetHeight, NULL);
            }
            else if(lstatus != AtBottom && lstatus != AtTop)
            {
                if(liftStatus != Moving)
                {
                    printf("[%d]----------liftStatus-----------\n",liftStatus);
                    liftStatus = Moving;
                }
            }
            break;

            //2018-03-26
            case 0x02:
                SendEvent(evLifterInit, NULL, NULL);
                break;

//For debug --0308
#if 0
            case 0x03:
                printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
//                char *outBuffer1;
//                outBuffer1 = "echo M3 did not receive a heartbeat packet for a long time. >> /opt/error.txt";
//                system(outBuffer1);
                break;

#endif

        default:
            i = 0;
            break;
    }

    return i;
}



CAgvBarrierSensor::CAgvBarrierSensor() :
        CAgvCanSensor(BARRIER_RCANID, BARRIER_WCANID, 3, "BarrierSensors")
{
    SetSensorPos(CrashBarrier, 1, Front);
    SetSensorPos(Laser, 1, Front);
    SetSensorPos(Touch, 2, Front);
    SetSensorPos(Touch, 4, Front);

    SetSensorPos(Touch, 1, Back);//for test
    SetSensorPos(Touch, 3, Back);

}

CAgvBarrierSensor::~CAgvBarrierSensor()
{
}

int CAgvBarrierSensor::SetBarrierDistance(int type, int id, int status,int distance)
{
    static int lastDistance = -1;

    if(lastDistance == distance)
    {
        return 0;
    }
    lastDistance = distance;

    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x01;
    buf[i++] = type;//02
    buf[i++] = id;//01
    buf[i++] = status;//00
    buf[i++] = distance;
    return SendData(buf, i);

}

int CAgvBarrierSensor::ResetBarrierError()
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x02;
    return SendData(buf, i);
}

void CAgvBarrierSensor::SetSensorPos(int type, int id, int pos)
{
    int dev = (type << 4) + id;
    map<int, int>::iterator it = barrierPosMap.find(dev);
    if (it == barrierPosMap.end())
    {
        barrierPosMap.insert(pair<int, int>(dev, pos));
    }
    else
    {
        it->second = pos;
    }

}

int CAgvBarrierSensor::GetSensorPos(int type, int id)
{
    int dev = (type << 4) + id;
    int pos = Unkown;
    map<int, int>::iterator it = barrierPosMap.find(dev);
    if (it != barrierPosMap.end())
    {
        pos = it->second;
    }
    return pos;
}

int CAgvBarrierSensor::GetBarrierError(int side)
{
    int status = None, dev = 0, value = 0;
    for (map<int, int>::iterator it = barrierStatus.begin();
            it != barrierStatus.end(); ++it)
    {
        dev = it->first;
        value = it->second;
        if (side == Unkown || side == GetSensorPos(dev >> 4, dev & 0x0F))
        {
            if (value > status)
            {
                status = it->second;
            }
        }
    }

    return status;
}

int CAgvBarrierSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    return 0;//close for test
    int i = 1;
    switch (buf[0])
    {
        case 0x01:
        {
            if (len < 4)
            {
                return -1;
            }
            int type   = buf[i++];
            int id     = buf[i++];
            int status = buf[i++];

            int dev = (type << 4) + id;
            if (type >= SensorTypeMax)
            {
                break;
            }

            int pos = GetSensorPos(type, id);
            int lastBarrier = GetBarrierError(pos);//last status


            map<int, int>::iterator it;
            it = barrierStatus.find(dev);
            if (it == barrierStatus.end())
            {
                barrierStatus.insert(pair<int, int>(dev, status));
            }
            else if (it->second == status)
            {
                break;
            }
            else
            {
                it->second = status;
            }


             pos = GetSensorPos(type, id);
            int barrier = GetBarrierError(pos);//now status
            if(lastBarrier == barrier)
            {
                break;
            }

            dev += pos << 8;
            printf("barrier = %d,lastBarrier = %d,status = %d,pos = %d\n",barrier,lastBarrier,status,pos);
            switch (status)
            {
                case None:
                    {
                        SendUrgentEvent(evBarrierOk, dev, NULL);
                    }
                    break;

                case Far:
                    {
                        printf("---------------------far\n");
                        SendUrgentEvent(evFarBarrier, dev, NULL);
                    }
                    break;

                case Middle:
                    {
                        printf("---------------------mid\n");
                        SendUrgentEvent(evMidBarrier, dev, NULL);
                    }
                    break;

                case Near:
                    {
                        printf("---------------------near\n");
                        SendUrgentEvent(evNearBarrier, dev, NULL);
                    }
                    break;

                case Touched:
                    {
                       // SendUrgentEvent(evBarrierTouched, dev, NULL);
                    }
                    break;

                default:
                    break;
            }
            break;
        }
        default:
            i = 0;
            break;
    }

    return i;
}


CAgvCargoSensor::CAgvCargoSensor() : CAgvCanSensor(CARGO_RCANID, CARGO_WCANID, 3, "CargoSensors")
{
    isHaveCargo = false;
}

CAgvCargoSensor::~CAgvCargoSensor()
{
}

bool CAgvCargoSensor::IsHaveCargo()
{
    return isHaveCargo;
}

int CAgvCargoSensor::ParseCanData(const unsigned char* buf, unsigned int len)
{
    int i = 1;
    switch (buf[0])
    {
        case 0x01:
            if (len < 3)
            {
                return -1;
            }
            isHaveCargo = (buf[1] || buf[2]);
            i = 3;
            break;
        default:
            i = 0;
            break;
    }
    return i;
}



CAgvRgbLight::CAgvRgbLight() :
        CAgvCanSensor(RGBLIGHT_RCANID, RGBLIGHT_WCANID, 4, "RGBLights")
{
}

CAgvRgbLight::~CAgvRgbLight()
{
}

int CAgvRgbLight::SetLight(int side, int red, int green, int blue)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x11;
    buf[i++] = side;
    buf[i++] = red;
    buf[i++] = green;
    buf[i++] = blue;
    return SendData(buf, i);
}

CAgvButtonSensor::CAgvButtonSensor() :
        CAgvCanSensor(KEY_RCANID, KEY_WCANID, 4, "Buttons")
{
    memset(keyStatus, 0, sizeof(keyStatus));
}

CAgvButtonSensor::~CAgvButtonSensor()
{
}

int CAgvButtonSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    int i = 1;
    unsigned char keys[KeyMax] = { 0 };

    switch (buf[0])
    {
        case 0x01:
            if (len < (KeyMax + 1))
            {
                return -1;
            }

            for (int j = 0; j < KeyMax; j++)
            {
                keys[j] = buf[i++];
                if (keys[j] != keyStatus[j])
                {
                    if (j == UrgentStopKey && keys[j] == 1)
                    {
                        SendUrgentEvent(evUrgentStop, 0, NULL);
                    }
                    else if (j == StartOrStopKey)
                    {
                        if (keys[j] == 1)
                        {
                            SendEvent(evKeyStart, 0, NULL);
                        }
                        else if(keys[j] == 2)
                        {
                            SendUrgentEvent(evKeyStop, 0, NULL);
                        }

                        else if(keys[j] == 3)
                        {
                            //SendEvent(evTurnOnCharge, 0, NULL);
                        }
                        else if(keys[j] == 4)
                        {
                            //SendUrgentEvent(evTurnOffCharge, 0, NULL);
                        }
                    }
                    else if (j == AutoOrManualKey)
                    {
                        static int flag = 0;
                        if (keys[j] == 1)
                        {
                            if (flag != keys[j])
                            {
                                SendEvent(evKeyAutoMode, 0, NULL);
                                flag = keys[j];
                                printf("############Auto##############\r\n");
                            }
                        }
                        else if (keys[j] == 2)
                        {
                            if (flag != keys[j])
                            {
                                SendEvent(evKeyManualMode, 0, NULL);
                                flag = keys[j];
                                printf("############Manual##############\r\n");
                            }
                        }
                    }
                    else if (j == LiftUpOrDownKey)
                    {
                        static int flag = -1;
                        if (keys[j] == 1)
                        {
                            if (flag != 1)
                            {
                                flag = 1;
                                SendEvent(evKeyLiftUp, 0, NULL);
                            }
                        }
                        else if (keys[j] == 2)
                        {
                            if (flag != 2)
                            {
                                flag = 2;
                                SendEvent(evKeyLiftDown, 0, NULL);
                            }
                        }
                    }

                    keyStatus[j] = keys[j];
                }
            }
            break;
        default:
            i = 0;
            break;
    }

    return i;
}
/*******************************HMI************************************************************/
CAgvHMISensor::CAgvHMISensor() : CAgvCanSensor(HMI_RCANID, HMI_WCANID, 4, "HMI")
{
    setting = &CAgvSetting::Instance();

}

CAgvHMISensor::~CAgvHMISensor()
{

}

int CAgvHMISensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    string str;

    switch(buf[0])
    {
    case 0x01:
    {
        int Volume = buf[1];
        setting->SaveSectionValue("Volume", Volume);
        printf("****************ReadHMIPara Volume = %d\n",Volume);
        break;
    }
    case 0x02:
    {
        int Speed = buf[1];


        setting->SaveSectionValue("Speed", Speed);
        printf("****************ReadHMIPara Speed = %d\n",Speed);

        int ForwardSpeed;
        ForwardSpeed = (int)(((Speed*100)/62.8)/0.075);//cm/min--> r/min
        setting->SaveSectionValue("ForwardSpeed",ForwardSpeed);
        break;
    }
    case 0x07:
    {
        int ServerPort;
        ServerPort  = buf[1]<<8;
        ServerPort += buf[2];
        setting->SaveSectionValue("ServerPort", ServerPort);
        printf("****************ReadHMIPara ServerPort = %d\n",ServerPort);
        break;
    }
    case 0x08:
    {
        char Id[2];
        int  a;
        a  = buf[1] << 8;
        a += buf[2];
        sprintf(Id,"%d",a);

        setting->SaveSectionValue("Id", Id);
        printf("****************ReadHMIPara Id = %s\n",Id);
        break;
    }
    case 0x09:
    {
        static char ServerIp[16] = {0};
        static int Len = 0;
        switch (buf[1])
        {
        case 0x01:
            ServerIp[0] = buf[2];
            ServerIp[1] = buf[3];
            break;
        case 0x02:
            ServerIp[2] = buf[2];
            ServerIp[3] = buf[3];
            break;
        case 0x03:
            ServerIp[4] = buf[2];
            ServerIp[5] = buf[3];
            break;
        case 0x04:
            ServerIp[6] = buf[2];
            ServerIp[7] = buf[3];

            break;
        case 0x05:
            ServerIp[8] = buf[2];
            ServerIp[9] = buf[3];
            break;
        case 0x06:
            ServerIp[10] = buf[2];
            ServerIp[11] = buf[3];
            break;
        case 0x07:
            ServerIp[12] = buf[2];
            ServerIp[13] = buf[3];
            break;
        case 0x08:
        {
            ServerIp[14] = buf[2];
            ServerIp[15] = buf[3];

            for(int i=0;i<16;i++)
            {
                //printf("-%x-",ServerIp[i]);
                if(ServerIp[i] != 0x20)
                {
                    Len++;
                }
            }
            //printf("\n");
            Len++;

            //printf("Len = %d\n",Len);
            char tempbuf[Len];
            for(int i=0;i<Len;i++)
            {
                tempbuf[i] = ServerIp[i];
                //printf("-%x-",tempbuf[i]);
            }
            //printf("\n");

            tempbuf[Len-1] = '\0';
            str = (string)tempbuf;
            setting->SaveSectionValue("ServerIp", str.c_str());
            printf("****************ReadHMIPara ServerIp = %s\n",str.c_str());
            Len = 0;
            break;
        }

        default:
            break;
        }
        break;
    }
    case 0x0A:
    {
        char Pid[4];
        int  a;
        a  = buf[1] << 24;
        a += buf[2] << 16;
        a += buf[3] << 8;
        a += buf[4];
        sprintf(Pid,"%d",a);
        setting->SaveSectionValue("Pid",Pid);
        printf("****************ReadHMIPara Pid = %s\n",Pid);
        break;
    }
    case 0x11:
    {
        int RepairMode = buf[1];
        setting->SaveSectionValue("RepairMode",RepairMode);
        printf("****************ReadHMIPara RepairMode = %d\n",RepairMode);
        break;
    }
    case 0x12:
    {
        int BatteryType = buf[1];
        setting->SaveSectionValue("BatteryType",BatteryType);
        printf("****************ReadHMIPara BatteryType = %d\n",BatteryType);
        break;
    }
    case 0x13:
    {
        int ChargingCmd = buf[1];
        if(ChargingCmd == 0)
        {
            SendUrgentEvent(evTurnOffCharge, 0, NULL);
        }
        else if(ChargingCmd == 1)
        {
            SendEvent(evTurnOnCharge, 0, NULL);
        }
    }
    case 0x14:
    {
        int UseDhcp = buf[1];
        setting->SaveSectionValue("UseDhcp", UseDhcp);
        printf("****************ReadHMIPara UseDhcp = %d\n",UseDhcp);
        break;
    }
    case 0x15:
    {
        static char LocalrIp[16] = {0};
        static int Len = 0;
        switch (buf[1])
        {
        case 0x01:
            LocalrIp[0] = buf[2];
            LocalrIp[1] = buf[3];
            break;
        case 0x02:
            LocalrIp[2] = buf[2];
            LocalrIp[3] = buf[3];
            break;
        case 0x03:
            LocalrIp[4] = buf[2];
            LocalrIp[5] = buf[3];
            break;
        case 0x04:
            LocalrIp[6] = buf[2];
            LocalrIp[7] = buf[3];
            break;
        case 0x05:
            LocalrIp[8] = buf[2];
            LocalrIp[9] = buf[3];
            break;
        case 0x06:
            LocalrIp[10] = buf[2];
            LocalrIp[11] = buf[3];
            break;
        case 0x07:
            LocalrIp[12] = buf[2];
            LocalrIp[13] = buf[3];
            break;
        case 0x08:
        {
            LocalrIp[14] = buf[2];
            LocalrIp[15] = buf[3];

            for(int i=0;i<16;i++)
            {
                //printf("-%x-",LocalrIp[i]);
                if(LocalrIp[i] != 0x20)
                {
                    Len++;
                }
            }
            //printf("\n");
            Len++;

            //printf("Len = %d\n",Len);
            char tempbuf[Len];
            for(int i=0;i<Len;i++)
            {
                tempbuf[i] = LocalrIp[i];
                //printf("-%x-",tempbuf[i]);
            }
            //printf("\n");

            tempbuf[Len-1] = '\0';
            str = (string)tempbuf;
            setting->SaveSectionValue("LocalIp", str.c_str());
            printf("****************ReadHMIPara LocalIp = %s\n",str.c_str());
            Len = 0;
            break;
        }

        default:
            break;
        }
        break;
    }

    case 0x81:
    {
        if(buf[1] == 0xFF)
        {
            WriteHMIPara();
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

int CAgvHMISensor::ShowBatteryCapacity(int Capacity)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x82;
    buf[i++] = 0x01;
    buf[i++] = Capacity;
    return SendData(buf, i);
}

int CAgvHMISensor::ShowNetState(int State)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x82;
    buf[i++] = 0x02;
    buf[i++] = State;
    return SendData(buf, i);
}
int CAgvHMISensor::ShowCarState(int State)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x82;
    buf[i++] = 0x03;
    buf[i++] = State;
    return SendData(buf, i);
}

int CAgvHMISensor::WriteHMIPara()
{
    unsigned char buf[8] = { 0 };
    int i;

    int Volume;
    Volume = setting->GetSectionValue("Volume", 5);
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x01;
    buf[i++] = Volume;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara Volume = %d\n",Volume);


    int Speed;
    int ForwardSpeed;
    ForwardSpeed = setting->GetSectionValue("ForwardSpeed",   900);
    Speed = (int)(ForwardSpeed *0.075*0.628);//r/min-->m/min
    //Speed = setting->GetSectionValue("Speed", 50);
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x02;
    buf[i++] = Speed;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara Speed = %d\n",Speed);


    int ServerPort;
    ServerPort = setting->GetSectionValue("ServerPort", 8623);
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x07;
    buf[i++] = ServerPort >> 8;
    buf[i++] = ServerPort & 0xFF;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara ServerPort = %d\n",ServerPort);

    string Id;
    Id  = setting->GetSectionValue("Id", "1001");
    int id = atoi(Id.c_str());
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x08;
    buf[i++] = id >> 8;
    buf[i++] = id & 0xFF;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara id = %d\n",id);

    string ServerIp;
    ServerIp = setting->GetSectionValue("ServerIp", "192.168.1.11");
    printf("*********WriteHMIPara ServerIp = %s\n",ServerIp.c_str());
    char serverIp[32] = {0};
    strcpy(serverIp,ServerIp.c_str());
    int k = 0;
    for(int j=1; j<9; j++)
    {
        i = 0;
        buf[i++] = 0x81;
        buf[i++] = 0x09;
        buf[i++] = j;
        buf[i++] = serverIp[k++];
        buf[i++] = serverIp[k++];
        SendData(buf, i);
    }


    string Pid;
    Pid = setting->GetSectionValue("Pid",  "01161001001");
    int pid = atoi(Pid.c_str());
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x0A;
    buf[i++] = (pid >> 24) & 0xFF;
    buf[i++] = (pid >> 16) & 0xFF;
    buf[i++] = (pid >> 8) & 0xFF;
    buf[i++] = pid & 0xFF;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara pid = %d\n",pid);

    int RepairMode;
    RepairMode = setting->GetSectionValue("RepairMode",0);
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x11;
    buf[i++] = RepairMode;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara RepairMode = %d\n",RepairMode);

    int BatteryType;
    BatteryType   = setting->GetSectionValue("BatteryType",    1);
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x12;
    buf[i++] = BatteryType;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara BatteryType = %d\n",BatteryType);

    int UseDhcp;
    UseDhcp = setting->GetSectionValue("UseDhcp", 0);
    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0x14;
    buf[i++] = UseDhcp;
    //usleep(100*1000);
    SendData(buf, i);
    printf("*********WriteHMIPara UseDhcp = %d\n",UseDhcp);


    string LocalIp;
    LocalIp = setting->GetSectionValue("LocalIp", "192.168.1.222");
    printf("*********WriteHMIPara LocalIp = %s\n",LocalIp.c_str());
    char localIp[32] = {0};
    strcpy(localIp,LocalIp.c_str());
    int z = 0;
    for(int j=1; j<9; j++)
    {
        i = 0;
        buf[i++] = 0x81;
        buf[i++] = 0x15;
        buf[i++] = j;
        buf[i++] = localIp[z++];
        buf[i++] = localIp[z++];
        SendData(buf, i);
    }

    i = 0;
    buf[i++] = 0x81;
    buf[i++] = 0xFF;
    buf[i++] = 0xFF;
    //usleep(100*1000);
    SendData(buf, i);
}
/****************************************************************************************************/
static const unsigned char MagMaxValue = 200;
static const unsigned char MagFullValues[] = { 201, 193, 177, 197, 194, 213,
                                               183, 181, 196, 183, 187, 183,
                                               192, 198,
                                               194,
                                               181 };
CAgvMagSensor::CAgvMagSensor(int id, int mode) :
        CAgvCanSensor(id ? MAG2_RCANID : MAG1_RCANID,
                      id ? MAG2_WCANID : MAG1_WCANID,
                      5, "MagSensor")
{
    this->id = id;
    this->mode = mode;
    magValue = 0;
    middlePoint = 7.5;
    memset(magValues, 0, sizeof(magValues));
    memcpy(magFullValues, MagFullValues, 16);
}

CAgvMagSensor::~CAgvMagSensor()
{

}

int CAgvMagSensor::SetMagShreshold(int value)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x11;
    return SendData(buf, i);
}

int CAgvMagSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    /*
     DEBUG_INFO("MagSensor", "Get data[%d]:", len);
     for (unsigned int i = 0; i < len; i++) {
     printf("%02X ", buf[i]);
     }
     printf("\n");
     */
    int i = 1;
    switch (mode)
    {
        case NormalBitMode:
            if (len == 2)
            {
                magValue = CAgvUtils::Buffer2Bin(buf + i, 2);
                i += 2;
            }
            break;
        case DoubleWidthMode:
            if (len == 4)
            {
                magValue = CAgvUtils::Buffer2Bin(buf + i, 4);
                i += 4;
            }
            break;
        case DetailDataMode:

            for (unsigned int j = 0; j < len; j += 2)
            {
                magValues[buf[j]] = (unsigned char)((buf[j + 1] * MagMaxValue * 1.0)
                                                    / magFullValues[buf[j]]);
            }

            if (buf[0] == 12)
            {
                printf("MagValues:");
                for (int j = 0; j < 16; j++)
                {
                    printf("%d,", magValues[j]);
                }

                unsigned short magvalue = 0;
                const int threshold = 150;
                int max = 0;

                for (int i = 0; i < 16; i++)
                {
                    if (magValues[i] > threshold)     //大于阀值
                    {
                        printf("P[%d]-%d ", i, magValues[i]);
                        magvalue |= 1 << i;
                        if (0)
                        {
                            if (magValues[i] - max > 10)    //取顶点
                            {
                                max = magValues[i];
                                magvalue = 0;
                            }

                            if (i > 0 && i < 15)
                            {
                                magvalue |= (7 << (i - 1));
                            }
                            else if (i == 0)
                            {
                                magvalue |= (3 << i);
                            }
                            else if (i == 15)
                            {
                                magvalue |= (3 << (i - 1));
                            }
                        }
                    }

                }
                printf(" MagValue:%02X", magvalue);
                printf("\n");
            }

            i += len;

            break;

    }
    return i;
}

CAgvMagNailSensor::CAgvMagNailSensor(int id) :
        CAgvCanSensor(id ? MAG2_RCANID : MAG1_RCANID,
                      id ? MAG2_WCANID : MAG1_WCANID,
                      5, "MagNailSensor")
{
    this->id = id;
    nailAngle = 0.0;
}

CAgvMagNailSensor::~CAgvMagNailSensor()
{
}

int CAgvMagNailSensor::GetLastNailAngle()
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x02;
    return SendData(buf, i);
}

int CAgvMagNailSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    int i = 1;
    switch (buf[0])
    {
        case 0x24:
            if (len < 5)
            {
                return -1;
            }
            nailAngle = CAgvUtils::Buffer2Bin(buf + i, 2) / 100.0;
            i += 2;
            break;
        default:
            i = 0;
            break;
    }
    return i;
}

CAgvRfidReader::CAgvRfidReader() :
        CAgvCanSensor(RFID_RCANID, RFID_WCANID, 5, "RFIDReader")
{
    cardType = 0;
    cardNumber = 0;
}

CAgvRfidReader::~CAgvRfidReader()
{
}

int CAgvRfidReader::ParseCanData(const unsigned char *buf, unsigned int len)
{

    int i = 1;
    switch (buf[0])
    {
        case 0x01:
            if (len < 4)
            {
                return -1;
            }
            cardType = buf[i++];
            cardNumber = CAgvUtils::Buffer2Bin(buf + i, 2);
            i += 2;
            break;
        default:
            i = 0;
            break;
    }
    return i;
}

CAgvVoicePlayer::CAgvVoicePlayer() :
        CAgvCanSensor(VOICE_RCANID, VOICE_WCANID, 6, "VoicePlayer")
{
}

CAgvVoicePlayer::~CAgvVoicePlayer()
{
}

int CAgvVoicePlayer::SetVolume(int volume)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    buf[i++] = 0x11;
    buf[i++] = volume;
    DEBUG_INFO("VoicePlayer", "set volume to %d", volume);
    return SendData(buf, i);
}

int CAgvVoicePlayer::PlayVoice(int id, int times)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    if (id != MP3_MUTE)
    {
        DEBUG_INFO("VoicePlayer", "Play voice[%d] %d times...", id, times);
    }
    else
    {
        DEBUG_INFO("VoicePlayer", "Set voice player mute!");
    }
    buf[i++] = 0x12;
    buf[i++] = id;
    buf[i++] = times;
    return SendData(buf, i);
}


CAgvPowerSensor::CAgvPowerSensor(int type) : CAgvCanSensor(POWER_RCANID, POWER_WCANID, 6, "PowerSensor")
{
    SetBatteryType(type);

    voltage = 0;
    capacity = 0;
    electricity = 0;
    level = Empty;
    if (-1 == system("cd /sys/class/gpio &&  echo 102 > export"))
    {
        perror("system gpio");
    }
}


CAgvPowerSensor::~CAgvPowerSensor()
{

}


void CAgvPowerSensor::SetBatteryType(int type)
{
    batteryType = type;
    switch (type)
    {
        case Lead:
            minVoltage = 22000;//unit:mv
            maxVoltage = 26000;
            break;
        case Lithium:
            minVoltage = 23000;
            maxVoltage = 29000;
            break;
        default:
            break;
    }
}


int CAgvPowerSensor::SetChargeSwitch(bool enable)
{
    unsigned char buf[8] = { 0 };
    int i = 0;
    if (enable == CAgvPowerSensor::On)
    {
        buf[i++] = 0x11;
        buf[i++] = 0x01;
        SendData(buf, i);
        return 0;
    }
    else if (enable == CAgvPowerSensor::Off)
    {
        buf[i++] = 0x11;
        buf[i++] = 0x00;
        SendData(buf, i);
        return 0;
    }
    return -2;
}

int CAgvPowerSensor::SetAgvSleepSwitch(bool enable)
{
    if (enable == CAgvPowerSensor::On)
    {
        if (-1 == system( "cd /sys/class/gpio/gpio102  &&  echo out > direction && echo 0 > value"))
        {
            perror("system set gpio");
            return -1;
        }
        printf("set gpio success\n");
        return 0;
    }
    else if (enable == CAgvPowerSensor::Off)
    {
        if (-1 == system( "cd /sys/class/gpio/gpio102  &&  echo out > direction && echo 1 > value"))
        {
            perror("system set gpio");
            return -1;
        }
        printf("set gpio success\n");
        return 0;
    }
    return -2;
}

int CAgvPowerSensor::calcCapacity(float volt)
{
    int cap = 0;

    if(volt > minVoltage)
    {
        cap = ((volt - minVoltage) / (maxVoltage - minVoltage))* 100.0;
        if(cap < 0)
        {
            cap = 0;
        }
        if(cap > 100)
        {
            cap = 100;
        }
    }
    else
    {
        cap = 0;
    }
    return cap;
}


int CAgvPowerSensor::ParseCanData(const unsigned char *buf, unsigned int len)
{
    int i = 0, type = None, cap = 0, vol = 0, ele = 0, status = 0;

    static int averageTimes = 0;
    static int sumVol = 0;

    switch (buf[0])
    {
        case 0x01:
            {
                if (len < 3)
                {
                    return -1;
                }

                i++;
                type   = buf[i++];
                vol    = CAgvUtils::Buffer2Bin(buf + i, 2);     i += 2;
                status = buf[i++];
                ele    = CAgvUtils::Buffer2Bin(buf + i, 2);     i += 2;

                sumVol += vol;
                averageTimes++;
                vol = sumVol/averageTimes;

                if(averageTimes > 10)
                {
                    sumVol = 0;
                    averageTimes = 0;
                }

                cap = calcCapacity(vol);
            }
            break;
        default:
            break;
    }

    if (vol != voltage || cap != capacity || ele != electricity)
    {
        voltage = vol;
        capacity = cap;
        electricity = ele;

        /*
        printf("batteryType = %d,voltage = %d,capacity = %d,electricity = %d\n",\
               batteryType,voltage,capacity,electricity);
        */

        BATTERYMAG *batteryMag = new BATTERYMAG;
        batteryMag->vol = voltage;
        batteryMag->cap = capacity;
        batteryMag->ele = electricity;

        SendEvent(evBattery,NULL, batteryMag);
    }

    return i;
}
