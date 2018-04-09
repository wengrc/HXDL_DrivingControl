/*
 * AgvEventHelper.cpp
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#include "AgvEventHelper.h"

CAgvEventHelper *CAgvEventHelper::instance = NULL;

CAgvEventHelper::CAgvEventHelper()
{

}

CAgvEventHelper::~CAgvEventHelper()
{
    Destroy();
}

CAgvEventHelper& CAgvEventHelper::Instance()
{
    if (instance == NULL)
    {
        instance = new CAgvEventHelper();
    }
    return *instance;
}

int CAgvEventHelper::Init()
{
    ClearAll();

    return pthread_mutex_init(&eventLocker, NULL);
}

void CAgvEventHelper::Destroy()
{
    ClearAll();
    pthread_mutex_destroy(&eventLocker);
    DeregisterAllListener();
}

int CAgvEventHelper::PushEvent(int event, const char *name, int param,
                               int priority,
                               void *data)
{
    CAgvEvent *evt = new CAgvEvent();
    evt->event = event;
    evt->name = string(name);
    evt->param = param;
    evt->priority = priority;
    evt->data = data;

    return PushEvent(evt);
}

int CAgvEventHelper::PushEvent(CAgvEvent *event)
{
    if (event == NULL)
    {
        return -1;
    }

    if (event->priority >= PriorityMax)
    {
        event->priority = Urgent;
    }
    pthread_mutex_lock(&eventLocker);

    eventList[event->priority].push_back(event);

    pthread_mutex_unlock(&eventLocker);

    return 0;
}

CAgvEvent *CAgvEventHelper::PopEvent()
{
    CAgvEvent *pevt = NULL;
    for (int i = PriorityMax - 1; i >= 0; i--)
    {
        if (!eventList[i].empty())
        {
            pthread_mutex_lock(&eventLocker);
            pevt = *eventList[i].begin();
            eventList[i].pop_front();
            pthread_mutex_unlock(&eventLocker);
            break;
        }
    }

    return pevt;
}

int CAgvEventHelper::DeleteEvent(int event)
{
    CAgvEvent *pevt = NULL;
    pthread_mutex_lock(&eventLocker);

    for (int i = 0; i < PriorityMax; i++)
    {
        for (list<CAgvEvent *>::iterator it = eventList[i].begin();
                it != eventList[i].end();)
        {
            pevt = *it;
            if (pevt->event == event)
            {
                it = eventList[i].erase(it);
                delete pevt;
            }
            else
            {
                ++it;
            }
        }
    }

    pthread_mutex_unlock(&eventLocker);

    return 0;
}

int CAgvEventHelper::ClearAll()
{
    CAgvEvent *pevt = NULL;
    pthread_mutex_lock(&eventLocker);
    for (int i = 0; i < PriorityMax; i++)
    {
        for (list<CAgvEvent *>::iterator it = eventList[i].begin();
                it != eventList[i].end();)
        {
            pevt = *it;
            delete pevt;
            it = eventList[i].erase(it);
        }
    }

    pthread_mutex_unlock(&eventLocker);

    return 0;
}

int CAgvEventHelper::RegisterListener(IAgvEventListener* listener)
{
    if (listener == NULL)
    {
        return -1;
    }

    bool exist = false;
    for (list<IAgvEventListener *>::iterator it = listenerList.begin();
            it != listenerList.end(); ++it)
    {
        if (*it == listener)
        {
            exist = true;
            break;
        }
    }

    if (!exist)
    {
        listenerList.push_back(listener);
    }

    return 0;
}

int CAgvEventHelper::DeRegisterListener(IAgvEventListener* listener)
{
    listenerList.remove(listener);
    return 0;
}

void CAgvEventHelper::ProcessEvents(CAgvEvent* evt)
{
    IAgvEventListener *listener;
    for (list<IAgvEventListener *>::iterator it = listenerList.begin();
            it != listenerList.end(); ++it)
    {
        listener = *it;
        if (listener->HandleEvent(evt))
        {
            break;
        }
    }
}

void CAgvEventHelper::DeregisterAllListener()
{
    for (list<IAgvEventListener *>::iterator it = listenerList.begin();
            it != listenerList.end();)
    {
        if (*it != NULL)
        {
            delete *it;
        }
        it = listenerList.erase(it);
    }
}

