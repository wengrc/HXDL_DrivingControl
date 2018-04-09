/*
 * AgvCarRunControl.cpp
 *
 *  Created on: 2017-5-4
 *      Author: zxq
 */

#include "AgvCarControler.h"
#include "AgvMoveHelper.h"
#include "AgvRoutePlanner.h"


CAgvMoveHelper::CAgvMoveHelper(const char *name, CAgvBaseControler *ctrler)
{
    this->name = name;
    isRunning = false;
    runningMode = 0;
    isFinished = false;
    isStopAfterFinish = false;
    needMoveLift = false;
    distance = 0;
    liftHeight = 0;
    controler = ctrler;
}

CAgvMoveHelper::~CAgvMoveHelper()
{
    // TODO Auto-generated destructor stub
}

int CAgvMoveHelper::Start()
{
    isRunning = true;
    return 0;
}

int CAgvMoveHelper::Stop()
{
    isRunning = false;
    return 0;
}


