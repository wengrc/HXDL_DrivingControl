/*
 * AgvMainLoop.h
 *
 *  Created on: 2016-10-15
 *      Author: zxq
 */

#ifndef AGVMAINLOOP_H_
#define AGVMAINLOOP_H_

#include "../AgvPublic.h"


class CAgvMainLoop : public CAgvThread
{
    public:
        CAgvMainLoop();
        virtual ~CAgvMainLoop();

        int StartLoop();

        void Run();

        int ExecLoop();

        void ExitLoop();

    private:


        bool loopEnable;
};

#endif /* AGVMAINLOOP_H_ */
