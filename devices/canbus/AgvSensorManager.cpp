/*
 * AgvSensorManager.cpp
 *
 *  Created on: 2016-11-24
 *      Author: zxq
 */

#include "AgvSensorManager.h"

#define LOGTAG      "SensorManager"


CAgvSensorManager::CAgvSensorManager(CAgvCanOperator *op):IAgvEventListener()
{
    canOperator = op;
    agvLogs = &CAgvLogs::Instance();
    filters.reserve(MAXSENSORS);
}

CAgvSensorManager::~CAgvSensorManager()
{
    CloseAllSensors();

    for (vector<struct can_filter *>::iterator it = filters.begin(); it != filters.end();)
    {
        delete (*it);
        it = filters.erase(it);
    }

    for (vector<CAgvCanSensor *>::iterator it = sensorList.begin(); it != sensorList.end();)
    {
        delete (*it);
        it = sensorList.erase(it);
    }
}

void CAgvSensorManager::SetCanOperator(CAgvCanOperator *op)
{
    canOperator = op;
}

/*static bool SensorCanIdSort(const CAgvCanSensor &s1, const CAgvCanSensor &s2)
{
    if (s1->canId < s2->canId)
    {
        return true;
    }
    return false;
}*/

bool CAgvSensorManager::AddSensor(CAgvCanSensor *sensor)
{
    if (sensor == NULL)
    {
        return false;
    }

    bool exist = false;
    for (unsigned int i = 0;i < sensorList.size(); i++)
    {
        if (sensorList[i] == sensor)
        {
            exist = true;
            break;
        }
    }
    if (!exist)
    {
        sensorList.push_back(sensor);
        sort(sensorList.begin(), sensorList.end()/*, SensorCanIdSort*/);
    }
    if (sensor->rCanId > 0)
    {
        exist = false;
        for (unsigned int i = 0; i < filters.size(); i++)
        {
            if (filters[i]->can_id == sensor->rCanId)
            {
                exist = true;
                break;
            }
        }
        if (!exist)
        {
            struct can_filter *filter = new can_filter;
            filter->can_id = sensor->rCanId;
            filter->can_mask = CAN_EFF_MASK;

            filters.push_back(filter);
        }
    }

    return true;
}



bool CAgvSensorManager::RemoveSensor(CAgvCanSensor *sensor)
{
    if (sensor == NULL)
    {
        return false;
    }

    if (sensor->rCanId > 0)
    {
        for (vector<can_filter*>::iterator it = filters.begin(); it != filters.end(); ++it)
        {
            if ((*it)->can_id == sensor->rCanId)
            {
                delete (*it);
                filters.erase(it);
                break;
            }
        }
    }

    for (vector<CAgvCanSensor*>::iterator it = sensorList.begin(); it != sensorList.end(); ++it)
    {
        if (*it == sensor)
        {
            sensorList.erase(it);
            sort(sensorList.begin(), sensorList.end()/*, SensorCanIdSort*/);
            return true;
        }
    }

    return false;
}

int  CAgvSensorManager::InitAllSensors()
{
    if (canOperator == NULL)
    {
        return -1;
    }

    CAgvCanSensor *sensor = NULL;
    for (unsigned int i = 0; i < sensorList.size(); i++)
    {
        sensor = sensorList[i];
        canOperator->AddCanFilter(sensor->rCanId, CAN_EFF_MASK);
        sensor->Init(canOperator);
    }

    canOperator->AddCanListener(this);
    CAgvEventHelper::Instance().RegisterListener(this);
    return 0;
}

int  CAgvSensorManager::CloseAllSensors()
{
    CAgvCanSensor *sensor = NULL;
    for (unsigned int i = 0; i < sensorList.size(); i++)
    {
        sensor = sensorList[i];
        sensor->Close();
    }

    CAgvEventHelper::Instance().DeRegisterListener(this);
    LogInfo(LOGTAG, "Close all sensors...");
    return 0;
}

int CAgvSensorManager::SendHeartBeat()
{
    if (canOperator == NULL)
    {
        return -1;
    }

    unsigned char  data[8];
    memset ( data, 0, sizeof(data) );
    data[0] = 0xff;
    data[1] = 0xff;

    for (unsigned int i = 0; i < sensorList.size(); i++)
    {
        sensorList[i]->SendData(data, 2);
    }

    return 0;
}

int CAgvSensorManager::HandlerData(unsigned int canId, const unsigned char* data, int len)
{
    CAgvCanSensor *sensor = NULL;
/*
    DEBUG_INFO(LOGTAG, "Get Can[%X] data[%d]:", canId, len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
*/
    for (unsigned int i = 0;i < sensorList.size(); i++)
    {
        sensor = sensorList[i];

//        printf("%s canid:%X\n", sensor->Name(), sensor->canId);
        if (canId == sensor->rCanId || canId == ES0_CANID)
        {
            sensor->ParseData(data, len);
        }
    }

    return 0;
}

bool CAgvSensorManager::HandleEvent(CAgvEvent *evt)
{
    if (evt->event == ev100msTimer)
    {
        CAgvCanSensor *sensor = NULL;
        int sensorNum = 0;
        static int laseSensorNum = 0;

        for (unsigned int i = 0; i < sensorList.size(); i++)
        {
            sensor = sensorList[i];

            sensor->heartBeatCounter++;
            if (sensor->heartBeatCounter >= 50)
            {
                sensor->heartBeatCounter = 0;
                sensor->isAlive = false;
                //LogError(LOGTAG, "%s [0x%04X, 0x%04X] is offline!", sensor->Name(), sensor->rCanId, sensor->wCanId);
            }

            if(sensor->isAlive == true)
            {
                sensorNum++;
            }
        }

        if(laseSensorNum!= sensorNum)
        {
            laseSensorNum = sensorNum;

            if(sensorNum != sensorList.size())
            {
                printf("sensorNum = %d,sensorList.size() = %d\n",sensorNum,sensorList.size());
                SendUrgentEvent(evSensorOffline, 0, NULL);
            }
            else if(sensorNum == sensorList.size())
            {
                printf("All Sensor Online\n");
                SendUrgentEvent(evSensorOnline, 0, NULL);
            }
        }
    }
    return false;
}


