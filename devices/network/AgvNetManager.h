/*
 * AgvNetManager.h
 *
 *  Created on: 2017-3-22
 *      Author: zxq
 */

#ifndef AGVNETMANAGER_H_
#define AGVNETMANAGER_H_

#include <string>
#include "../../utils/AgvThread.h"
#include "AgvWifi.h"
#include "AgvEthernet.h"

using namespace std;

class CAgvNetManager : CAgvThread
{
    public:
        enum NetworkType{
            None        = 0,
            Ethernet    = 1,
            Wifi        = 2,
            Modem       = 4,
        };

        static CAgvNetManager &Instance();

        virtual ~CAgvNetManager();

        int Init();

        int SetupNetwork();

        int CloseNetowrk(bool closeDev);

        bool CheckNetStatus();



        inline bool IsWifiEnable()      {return isWifiEnable;}
        inline bool IsEthernetEnable()  {return isEthernetEnable;}
        inline bool IsModemEnable()     {return isModemEnable;}  
        inline int CurrentType()        {return currentType;}
        inline string &CurrentIp()      {return currentIp;}
        inline bool IsConnected()       {return isConnected;}

    private:
        CAgvNetManager();

        void Run();

    private:
        static CAgvNetManager *instance;

        bool    isWifiEnable;
        bool    isEthernetEnable;
        bool    isModemEnable;
        bool    isUseDhcp;
        int     currentType;
        string  currentIp;
        bool    isConnected;

        bool    loopEnable;
        int     taskCmd;
        string  localIp;

        CAgvEthernet    *ethernet;
        CAgvWifi        *wifi;
};

#endif /* AGVNETMANAGER_H_ */
