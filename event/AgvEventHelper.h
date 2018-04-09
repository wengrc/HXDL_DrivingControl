/*
 * AgvEventHelper.h
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#ifndef AGVEVENTHELPER_H_
#define AGVEVENTHELPER_H_

#include <list>
#include "AgvEvent.h"

using namespace std;

class CAgvEventHelper
{
    public:

        static CAgvEventHelper &Instance();

        virtual ~CAgvEventHelper();

        int Init();

        void Destroy();

        int PushEvent(CAgvEvent *event);

        int PushEvent(int event, const char *name, int param, int priority, void *data);

        CAgvEvent *PopEvent();

        int DeleteEvent(int event);

        int ClearAll();

        int RegisterListener(IAgvEventListener *listener);

        int DeRegisterListener(IAgvEventListener *listener);

        void ProcessEvents(CAgvEvent *evt);

        void DeregisterAllListener();

    private:
        CAgvEventHelper();

    private:
        list<CAgvEvent *>           eventList[PriorityMax];
        pthread_mutex_t             eventLocker;
        list<IAgvEventListener *>   listenerList;

        static CAgvEventHelper      *instance;
};

#define SendEvent(event, param, data) do{CAgvEventHelper::Instance().PushEvent(event, #event, param, Normal, data);}while(0)

#define SendImportantEvent(event, param, data) do{CAgvEventHelper::Instance().PushEvent(event, #event, param, Important, data);}while(0)

#define SendUrgentEvent(event, param, data) do{CAgvEventHelper::Instance().PushEvent(event, #event, param, Urgent, data);}while(0)



#endif /* AGVEVENTHELPER_H_ */
