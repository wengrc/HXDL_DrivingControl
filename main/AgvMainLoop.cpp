/*
 * AgvMainLoop.cpp
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#include "AgvMainLoop.h"
#include "../controler/AgvCar.h"
#include "../devices/network/AgvWifi.h"
#include "../devices/network/AgvNetManager.h"
#include "../comm/AgvFinder.h"

extern CAgvFinder   agvReplier;

#define LOGTAG      "MainLoop"


CAgvMainLoop::CAgvMainLoop() : CAgvThread("MainLoop")
{
    loopEnable = false;
    CAgvEventHelper::Instance().Init();
}


CAgvMainLoop::~CAgvMainLoop()
{
    ExitLoop();
    CAgvEventHelper::Instance().ClearAll();
}


int CAgvMainLoop::StartLoop()
{
    loopEnable = true;
    return Start(true);
}


void CAgvMainLoop::ExitLoop()
{
    if (loopEnable)
    {
        loopEnable = false;
        Stop();
    }
}


void CAgvMainLoop::Run()
{
    int timerCount = 0;

    while (loopEnable)
    {
        usleep(10*1000);

        timerCount++;

        SendEvent(ev10msTimer, 0, NULL);

        if (timerCount % 10 == 0)
        {
            SendImportantEvent(ev100msTimer, 0, NULL);
        }
        if (timerCount % 50 == 0)
        {
            SendImportantEvent(ev500msTimer, 0, NULL);
        }
        if (timerCount % 100 == 0)
        {
            SendImportantEvent(ev1sTimer, 0, NULL);
            timerCount = 0;
        }
    }
}


int CAgvMainLoop::ExecLoop()
{
    LogInfo("MainLoop", "Start execute mainloop....");

    CAgvEvent       *evt;
    CAgvEventHelper &evHelper = CAgvEventHelper::Instance();

    while(loopEnable)
    {
        evt = evHelper.PopEvent();
        if (evt != NULL)
        {
            switch (evt->event)
            {
                case ev10msTimer:
                    break;
                case ev100msTimer:
                    break;
                case ev500msTimer:
                    break;
                case ev1sTimer:
                    break;
                case evSysClose:
                    {
                        LogInfo(LOGTAG, "Prepare to close system....");
                        ExitLoop();
                    }
                    break;
                case evPowerDown:
                    {
                        LogInfo(LOGTAG, "Power is down! Power off system now");
                        ExitLoop();
                    }
                    break;
                default:
                    break;
            }

            evHelper.ProcessEvents(evt);
            delete evt;
        }
        else
        {
            usleep(10*1000);
        }
    }

    CAgvEventHelper::Instance().Destroy();
    LogInfo("MainLoop", "Exit mainloop....");
    return 0;
}



