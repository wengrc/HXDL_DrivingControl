/*
 * AgvRoutePlanner.cpp
 *
 *  Created on: 2016-10-19
 *      Author: zxq
 */

#include <stdlib.h>

#include "AgvRoutePlanner.h"
#include "../AgvPublic.h"


#define LOGTAG  "RoutePlanner"
#define PATHFILE    "lastpath"

CAgvRoutePlanner::CAgvRoutePlanner()
{
    xPos = 0;
    yPos = 0;
    carAngle = 0;
    carDirection = 0;
    targetPathId = 0;
    lastPathId = -1;
    pathList.reserve(1024);
    wheelBase = 0;
    lastStationPoint = 0;
    isWayPointFinished = false;
    setting = &CAgvSetting::Instance();
}

CAgvRoutePlanner::~CAgvRoutePlanner()
{
    Stop();
    clearPathList();
}

int CAgvRoutePlanner::Start()
{
    string ip = CAgvSetting::Instance().ServerIp;
    int port  = CAgvSetting::Instance().ServerPort;
    int agvid = atoi(CAgvSetting::Instance().Id.c_str());

    scheduler.SetServerIp(ip);
    scheduler.SetServerPort(port);
    scheduler.SetAgvId(agvid);

    loadPathInfo();

    return scheduler.StartSchedule();
}

int CAgvRoutePlanner::Stop()
{
    return scheduler.StopSchedule();
}


static const char *RunModeString[] = {
                                    "Stop",
                                    "Forward",
                                    "Backward",
                                    "Save1",
                                    "Save2",
                                    "TurnLeft",
                                    "TurnRight",
                                    "Save3",
                                    "Save4",
                                    "Save5",
                                    "Save6",
                                    "Save7",
                                    "Save8",
};


static const char *ActionString[] = {
                                     "Run",               //0x00 保持运行
                                     "Stop",              //0x01 停站
                                     "SpeedDown",         //0x04 减速到最小速度
                                     "ResumeSpeed",       //0x05 加速到正常速度
                                     "LiftUpDown",        //0x06 停止后叉货
                                     "Charge",            //0x08 充电
};

int CAgvRoutePlanner::OnGetNewPath(const vector<StationInfo*> &list)
{
    clearPathList();
    DEBUG_INFO(LOGTAG, "Get Server's new path list:[%d]", list.size());
    StationInfo *info;
    for (unsigned int i = 0; i < list.size(); i++)
    {
        info = list[i];
        WayPoint *path = new WayPoint();
        path->id = info->id;
        path->xPos = info->xPos;
        path->yPos = info->yPos;
        path->angle = info->pathAngle;
        path->runMode = info->mode;
        path->action = info->action;
        path->actionFinish = false;
        if (info->liftHeight == 0xFFFF)
        {
            path->liftHeight = -1;
        }
        else
        {
            path->liftHeight = info->liftHeight;
        }
        pathList.push_back(path);
    }

    if (pathList.size() > 0)
    {
        targetPathId = 0;
        lastPathId = -1;
        SendEvent(evApplyPath, 0, NULL);

        DEBUG_INFO(LOGTAG, "Convert it to Car's  PathPoint list:[%d]", pathList.size());
        for (unsigned int i = 0;i < pathList.size(); i++)
        {
            WayPoint *point = pathList[i];
            printf("PathPoint[%d]:id:%d x:%d y:%d angle:%f runmode:%s action:%s\n",
                    i, point->id, point->xPos, point->yPos,
                    point->angle, RunModeString[point->runMode], ActionString[point->action]);
        }

    }
    savePathInfo(true);
    return pathList.size();

}

int CAgvRoutePlanner::savePathInfo(bool all)
{
    setting->PathPointCount = pathList.size();
    setting->TargetPointId = targetPathId;
    setting->LastPointId = lastPathId;
    //setting->LastStationPoint = lastStationPoint;

    if (all)
    {
        char buf[1024] = {0};
        WayPoint *point = NULL;
        FILE *fp = fopen(PATHFILE, "w+");
        if (fp == NULL)
        {
            LogError("SavePathList", "Open path file %s failed! %s", PATHFILE, strerror(errno));
            return -1;
        }
        for (unsigned int i = 0; i < pathList.size(); i++)
        {
            point = pathList[i];
            sprintf(buf, "{%d,%d,%d,%d,%f,%d,%d,%d}\n",
                          point->id, point->cardId,
                          point->xPos, point->yPos, point->angle, point->runMode,
                          point->action, point->liftHeight);
            fputs(buf, fp);
        }
        fclose(fp);

    }

    setting->SaveSectionValue("TargetPoinId", targetPathId);
    setting->SaveSectionValue("LastPointId", lastPathId);
    //setting->SaveSectionValue("LastStationPoint", lastStationPoint);
    LogInfo(LOGTAG, "Save PathList to file finished!");
    return pathList.size();
}

int CAgvRoutePlanner::loadPathInfo()
{
    clearPathList();

    DEBUG_INFO(LOGTAG, "Reading PathList from file...");
    FILE *fp = fopen(PATHFILE, "r");
    if (fp == NULL)
    {
        LogError("LoadPathList", "Open path file %s failed! %s", PATHFILE, strerror(errno));
        return -1;
    }

    char buf[256] = {0}, linebuf[256] = {0}, *ph = NULL, *pt = NULL;
    WayPoint *point = NULL;
    int i = 0;
    while(fgets(linebuf, sizeof(linebuf) - 1, fp))
    {
        if (linebuf[0] == '#')
        {
            continue;
        }
        pt = ph = linebuf;
        do {
            if (*ph != '{')
            {
                ++ph;
                ++pt;
            }
            else        //找到了头字符
            {
                if (*pt != '}')
                {
                    ++pt;
                }
                else    //找到了尾字符
                {
                    strncpy(buf, ph, pt - ph + 1);
                    point = new WayPoint;
                    memset(point, 0, sizeof(WayPoint));
                    if (sscanf(buf, "{%d,%d,%d,%d,%lf,%d,%d,%d}",
                               &point->id, &point->cardId,
                               &point->xPos, &point->yPos, &point->angle, &point->runMode,
                               &point->action, &point->liftHeight) == 8)
                    {
                        pathList.push_back(point);

                        printf("PathPoint[%d]:id:%d x:%d y:%d mode:%s angle:%lf action:%s liftheight:%d\n",
                               i, point->id, point->xPos, point->yPos,
                               RunModeString[point->runMode], point->angle, ActionString[point->action], point->liftHeight);
                        i++;
                    }
                    else
                    {
                        delete point;
                    }
                    ++ph;
                    ++pt;
                }
            }

        }while(*ph != 0 && *pt != 0);
    }
    fclose(fp);

    setting->PathPointCount = pathList.size();

    LogInfo(LOGTAG, "Load pathList form file: targetPathId:%d,lastPathId:%d,pathListCount:%d",
            setting->TargetPointId, setting->LastPointId, setting->PathPointCount);

    targetPathId = setting->TargetPointId;
    lastPathId = setting->LastPointId;
    //lastStationPoint = setting->LastStationPoint;

    return pathList.size();
}

int CAgvRoutePlanner::RequestNewPath()
{
    return scheduler.RequestNewPath(xPos, yPos, lastStationPoint);
}

void CAgvRoutePlanner::ResetPathList()
{
    clearPathList();
    savePathInfo(false);
}

int CAgvRoutePlanner::ReloadPathList()
{
    ResetPathList();
    return loadPathInfo();
}


int CAgvRoutePlanner::InsertPoint(WayPoint *first, WayPoint *second)
{
    pathList.insert(pathList.begin()+targetPathId+1,first);
    pathList.insert(pathList.begin()+targetPathId+2,second);
    return 0;
}


bool CAgvRoutePlanner::MoveToNextStation()
{
    bool ret = false;
    printf("**** Move to next station ****\n");
    //printf("Before: targetPathId=%d,lastPathId = %d\n",targetPathId,lastPathId);
    if (pathList.size() > 0)
    {
        if (targetPathId < ((int)pathList.size() - 1))
        {
            lastPathId = targetPathId;
            targetPathId++;
            ret = true;
        }

        //printf("After: targetPathId=%d,lastPathId = %d\n",targetPathId,lastPathId);

        if(0 <= lastPathId && lastPathId < pathList.size())
        {
            lastStationPoint = GetLastStation()->id;
        }
        savePathInfo(false);
    }

    return ret;
}

bool CAgvRoutePlanner::MoveToLastStation()
{
    bool ret = false;
    printf("**** Move to last station ****\n");
    //printf("Before: targetPathId=%d,lastPathId = %d\n",targetPathId,lastPathId);
    if (pathList.size() > 0)
    {
        if(targetPathId >= 1)
        {
            targetPathId--;
            lastPathId = targetPathId;
            lastPathId--;
            ret = true;
        }

        //printf("After: targetPathId=%d,lastPathId = %d\n",targetPathId,lastPathId);

        if(0 <= lastPathId && lastPathId < pathList.size())
        {
            lastStationPoint = GetLastStation()->id;
        }
        savePathInfo(false);
    }

    return ret;
}

void CAgvRoutePlanner::CleanLastStationPoint()
{
    lastStationPoint = 0;
}


int CAgvRoutePlanner::OnActionFinished(int lastLiftHight)
{
    WayPoint *target = GetTargetStation();

    if (target->actionFinish)
    {
        LogWarn(LOGTAG, "Current way point's action has been finished!");
        return 1;
    }
    target->actionFinish = true;

    LogInfo(LOGTAG, "Current way point's action %d is finished!", target->action);

    LogInfo(LOGTAG, "lastLiftHight = %d, target->liftHeight = %d!", lastLiftHight,target->liftHeight);
    if (target->liftHeight >= 0 && target->action == ActionLiftUpDown)
    {
        if(target->liftHeight < lastLiftHight)//Down
        {
            scheduler.ReportTaskStatus(target->id, 2);
        }
        else if(target->liftHeight > lastLiftHight)//Up
        {
            scheduler.ReportTaskStatus(target->id, 1);
        }
    }
    return 0;
}


WayPoint* CAgvRoutePlanner::GetTargetStation()
{
    WayPoint *target = NULL;
    if (pathList.size() > 0)
    {
       if (targetPathId >= 0 && targetPathId < (int)pathList.size())
       {
           target = pathList[targetPathId];
       }
    }
    return target;
}

WayPoint* CAgvRoutePlanner::GetLastStation()
{
    WayPoint *last = NULL;
    if (pathList.size() > 0)
    {
       if (lastPathId >= 0 && lastPathId < (int)pathList.size())
       {
           last = pathList[lastPathId];
       }
    }
    return last;
}

void CAgvRoutePlanner::UpdateCarInfo(int x, int y, int angle, int speed, int power, int status)
{
    xPos = x;
    yPos = y;
    carAngle = angle;

    scheduler.SetXPos(x);
    scheduler.SetYPos(y);
    scheduler.SetCarAngle(angle);
    scheduler.SetCarSpeed(speed);
    scheduler.SetPowerLevel(power);
    scheduler.SetStatus(status);
    scheduler.SetLastPoint(lastStationPoint);

}


void CAgvRoutePlanner::clearPathList()
{
    for (unsigned int i = 0; i < pathList.size(); i++)
    {
        delete pathList[i];
    }
    pathList.clear();
    targetPathId = 0;
    lastPathId = -1;
    //lastStationPoint = 1;
    carDirection = 0;
}

void CAgvRoutePlanner::RequestSystemTimeSynchronization()
{
    scheduler.RequestSystemTimeSynchronization();
}
