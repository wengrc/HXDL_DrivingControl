/*
 * AgvEthernet.cpp
 *
 *  Created on: 2017-3-22
 *      Author: zxq
 */

#include <stdio.h>
#include <stdlib.h>

#include "AgvEthernet.h"


#include "../../utils/AgvUtils.h"

CAgvEthernet::CAgvEthernet()
{
    devName = "eth0";
}

CAgvEthernet::~CAgvEthernet()
{
//    PowerOff();
}

int CAgvEthernet::PowerOn(const char *ifname)
{
    devName = ifname;

    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd) - 1, "ifconfig %s up", ifname);
    int ret = system(cmd);
    if (ret == 0)
    {
        printf("Ethernet %s is up now!\n", ifname);
    }
    else
    {
        printf("Ethernet %s setting up failed!\n", ifname);
    }
    return ret;
}

int CAgvEthernet::PowerOff()
{
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd) - 1, "ifconfig %s down", devName.c_str());
    int ret = system(cmd);
    if (ret == 0)
    {
        printf("Turn ethernet %s down!\n", devName.c_str());
    }
    else
    {
        printf("Turn ethernet %s down failed!\n", devName.c_str());
    }
    return ret;
}

int CAgvEthernet::SetIpAddress(const string& ip, bool dhcp)
{
    int ret = -1;

    if (dhcp)
    {
        ret = CAgvUtils::StartDhcp(devName.c_str(), 30);

        if (ret == 0)
        {
            printf("Get dhcp ip address ok!\n");
        }
        else
        {
            printf("Get dhcp ip address failed!\n");
        }
        return ret;
    }
    else
    {
        ret = CAgvUtils::SetIpAddress(devName.c_str(), ip.c_str());
        if (ret == 0)
        {
            printf("Setting %s to IP:%s OK!\n", devName.c_str(), ip.c_str());
        }
        else
        {
            printf("Setting %s to IP:%s failed!\n", devName.c_str(), ip.c_str());
        }
    }
    return ret;
}

string CAgvEthernet::GetCurrentIpAddress()
{
    char ip[64] = {0};

    int ret = CAgvUtils::GetIpAddress(devName.c_str(), ip, sizeof(ip) - 1);
    if (ret == 0)
    {
        return ip;
    }
    return "";
}

int CAgvEthernet::GetCurrentStatus()
{
    int status = CAgvUtils::CheckNetStatus(devName.c_str());
    if (status == 0)
    {
        return Connected;
    }
    else if (status == 1)
    {
        return DevReady;
    }
    return DevOff;
}



