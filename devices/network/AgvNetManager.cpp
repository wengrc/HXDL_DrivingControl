/*
 * AgvNetManager.cpp
 *
 *  Created on: 2017-3-22
 *      Author: zxq
 */

#include "AgvNetManager.h"
#include "../../AgvPublic.h"


#define LOGTAG  "NetManager"


CAgvNetManager *CAgvNetManager::instance = NULL;

CAgvNetManager& CAgvNetManager::Instance()
{
    if (instance == NULL)
    {
        instance = new CAgvNetManager();
    }
    return *instance;
}

CAgvNetManager::CAgvNetManager():CAgvThread("NetManager")
{
    isWifiEnable     = false;
    isEthernetEnable = false;
    isModemEnable    = false;
    isUseDhcp        = false;

    isConnected = false;
    currentType = None;
    currentIp = "";

    loopEnable = false;
    taskCmd = 0;

    ethernet = NULL;
    wifi = NULL;
}

CAgvNetManager::~CAgvNetManager()
{
    CloseNetowrk(false);
    if (ethernet != NULL)
    {
        delete ethernet;
        ethernet = NULL;
    }
    if (wifi != NULL)
    {
        delete wifi;
        wifi = NULL;
    }
}

int CAgvNetManager::Init()
{
    if (ethernet == NULL)
    {
        ethernet = new CAgvEthernet();
    }
    if (wifi == NULL)
    {
        wifi = new CAgvWifi();
    }

    isWifiEnable = CAgvSetting::Instance().WifiEnable;
    LogInfo(LOGTAG, "WifiEnable = %d", isWifiEnable);

    isEthernetEnable = CAgvSetting::Instance().EthernetEnable;
    LogInfo(LOGTAG, "EthernetEnable = %d", isEthernetEnable);

    isUseDhcp = CAgvSetting::Instance().UseDhcp;
    LogInfo(LOGTAG, "UseDhcp = %d", isUseDhcp);

    localIp = CAgvSetting::Instance().LocalIp;
    LogInfo(LOGTAG, "Load Local IP: %s", localIp.c_str());

    currentType = None;
    currentIp = "";
    isConnected = false;

    return 0;
}

int CAgvNetManager::SetupNetwork()
{
    taskCmd = 1;
    loopEnable = true;
    if (!IsRunning())
    {
        Start(false);
    }
    return 0;
}

int CAgvNetManager::CloseNetowrk(bool closeDev)
{
    if (loopEnable)
    {
        if (closeDev)
        {
            taskCmd = 2;
            while(taskCmd != 0)
            {
                usleep(50 * 1000);
            }
        }
        loopEnable = false;
        if (IsRunning())
        {
            Stop();
        }
    }
    return 0;
}

bool CAgvNetManager::CheckNetStatus()
{
    if (isEthernetEnable)
    {
        int status = ethernet->GetCurrentStatus();
        if (status == CAgvEthernet::Connected)
        {
            isConnected = true;
            currentType = Ethernet;
        }
        else
        {
            isConnected = false;
        }
    }

    if (isWifiEnable)
    {
        isConnected = wifi->CheckConnectStatus();
        if (isConnected)
        {
            currentType = Wifi;
        }
    }

    if (isConnected)
    {
        char ip[32] = {0};
        int ret = -1;
        if (currentType == Wifi)
        {
            ret = CAgvUtils::GetIpAddress(wifi->DevName().c_str(), ip, sizeof(ip) - 1);
        }
        else if (currentType == Ethernet)
        {
            ret = CAgvUtils::GetIpAddress(ethernet->DevName().c_str(), ip, sizeof(ip) - 1);
        }
        if (ret == 0)
        {
            currentIp = ip;
        }
        else
        {
            currentIp = "";
        }
    }
    else
    {
        currentIp = "";
    }

    return isConnected;
}

void CAgvNetManager::Run()
{
    int  timerCount = 0;
    bool lastNetworkStatus = false;
    bool isNetworkOk = false;

    while(loopEnable)
    {
        switch (taskCmd)
        {
            case 0: //¿ÕÏÐ
                if (timerCount++ > 10)
                {
                    timerCount = 0;

                    isNetworkOk = CheckNetStatus();
                    if (isNetworkOk != lastNetworkStatus)
                    {
                        if (isNetworkOk)
                        {
                            if (isWifiEnable)
                            {
                                SendEvent(evNetworkOK, 0, NULL);
                                LogInfo(LOGTAG, "Wifi is connected!");
                            }
                            if (isEthernetEnable)
                            {
                                SendEvent(evNetworkOK, 1, NULL);
                                LogInfo(LOGTAG, "Ethernet is connected!");
                            }
                        }
                        else
                        {
                            if (isWifiEnable)
                            {
                                SendEvent(evNetworkError, 0, NULL);
                                LogError(LOGTAG, "Wifi is disconnected!");
                            }
                            if (isEthernetEnable)
                            {
                                SendEvent(evNetworkError, 1, NULL);
                                LogError(LOGTAG, "Ethernet is disconnected!");
                            }
                        }

                        lastNetworkStatus = isNetworkOk;
                    }
                }
                else
                {
                    usleep(100 * 1000);
                }
                break;

            case 1: //´ò¿ªÍøÂç
            {
                bool isNetworkReady = false;

                //³õÊ¼»¯ÍøÂç
                if (isEthernetEnable)
                {
                    if (ethernet->PowerOn("eth0") != 0)
                    {
                        LogError(LOGTAG, "Ehternet setup failed!");
                        isEthernetEnable = false;
                        isWifiEnable = true;
                    }
                    else
                    {
                        int ret = -1;
                        if (isUseDhcp)
                        {
                            ret = ethernet->SetIpAddress("", true);
                            if (ret == 0)
                            {
                                LogInfo(LOGTAG, "Ethernet get dhcp ok!");
                            }
                            else
                            {
                                LogError(LOGTAG, "Ethernet get dhcp failed!");
                            }
                        }

                        if (!isUseDhcp || ret)
                        {
                            bool ipexist = false;

                            if (CAgvUtils::CheckIpAddress(localIp.c_str(), &ipexist))
                            {
                                LogWarn(LOGTAG, "Get ethernet ip failed!");
                            }

                            if (!ipexist)
                            {

                                ret = ethernet->SetIpAddress(localIp, false);
                                if (ret == 0)
                                {
                                    LogInfo(LOGTAG, "Seting ethernet ip to %s ok!", localIp.c_str());
                                }
                                else
                                {
                                    LogError(LOGTAG, "Seting ethernet ip to %s failed!", localIp.c_str());
                                }
                            }
                            else
                            {
                                LogInfo(LOGTAG, "Now the ip address is %s\n", localIp.c_str());
                                isNetworkReady = true;
                            }
                        }
                    }
                }

                if (isWifiEnable)
                {
                    int ret = wifi->PowerOn(CAgvWifi::StandMode);
                    ret = wifi->ConnectToAp(10);
                    if (wifi->CheckConnectStatus())
                    {
                        if (wifi->SetIpAddress(localIp.c_str(), isUseDhcp))
                        {
                            LogError(LOGTAG, "Connect to AP failed!");
                        }
                        else
                        {
                            LogInfo(LOGTAG, "Connect to AP OK!");
                            isNetworkReady = true;
                        }
                    }
                    else
                    {
                        LogError(LOGTAG, "Connect to AP failed!");
                    }

                }
                taskCmd = 0;
                break;
            }

            case 2: //¹Ø±ÕÍøÂç
            {
                if (isEthernetEnable)
                {
                    ethernet->PowerOff();
                }
                if (isWifiEnable)
                {
                    wifi->PowerOff();
                }

                taskCmd = 0;
                break;
            }
            default:
                break;
        }
    }
}






