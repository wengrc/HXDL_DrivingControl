/*
 * AgvSensorManager.h
 *
 *  Created on: 2016-11-24
 *      Author: zxq
 */

#ifndef AGVSENSORMANAGER_H_
#define AGVSENSORMANAGER_H_

#include <vector>

#include "../../AgvPublic.h"
#include "AgvCanOperator.h"
#include "AgvSensor.h"

using namespace std;

class CAgvSensorManager : public IAgvCanListener, IAgvEventListener
{
    public:
        CAgvSensorManager(CAgvCanOperator *op = NULL);
        virtual ~CAgvSensorManager();

        void SetCanOperator(CAgvCanOperator *op);

        bool AddSensor(CAgvCanSensor *sensor);

        bool RemoveSensor(CAgvCanSensor *sensor);

        int  InitAllSensors();

        int  CloseAllSensors();

        int SendHeartBeat();

        int HandlerData(unsigned int canId, const unsigned char *data, int len);

        bool HandleEvent(CAgvEvent *evt);
    private:
        CAgvCanOperator         *canOperator;
        vector<can_filter*>     filters;
        CAgvLogs                *agvLogs;
        vector<CAgvCanSensor*>  sensorList;
};

#endif /* AGVSENSORMANAGER_H_ */
