/*
 * AgvEthernet.h
 *
 *  Created on: 2017-3-22
 *      Author: zxq
 */

#ifndef AGVETHERNET_H_
#define AGVETHERNET_H_

#include <string>

using namespace std;

class CAgvEthernet
{
    public:
        enum Status{
            DevOff,
            DevReady,
            Connected,
        };

        CAgvEthernet();

        virtual ~CAgvEthernet();

        int PowerOn(const char *ifname = "eth0");

        int PowerOff();

        int SetIpAddress(const string &ip, bool dhcp);

        string GetCurrentIpAddress();

        int GetCurrentStatus();

        inline const string &DevName() {return devName;}

    private:
        static CAgvEthernet *instance;
        string devName;
};

#endif /* AGVETHERNET_H_ */
