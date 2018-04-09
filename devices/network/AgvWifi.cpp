/*
 * AgvWifi.cpp
 *
 *  Created on: 2016-8-31
 *      Author: zxq
 */

#include "AgvWifi.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <string.h>

#include "../../utils/AgvUtils.h"



CAgvWifi::CAgvWifi()
{
	currentMode = -1;
	status = DevOff;
	haveUsbWifi = false;
	devName = "wlan0";
}


CAgvWifi::~CAgvWifi()
{
//	PowerOff();
}

bool CAgvWifi::checkUsbDeviceIsExist(const char *pidvid)
{
    if (pidvid == NULL || strlen(pidvid) == 0)
    {
        return false;
    }
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd) - 1, "lsusb | grep %s", pidvid);

    int ret = system(cmd);
    if (ret == 0)
    {
        return true;
    }
    return false;
}

/***
 *
 *
 * return:
 *        0: no exist
 *        1: exist
 *
 */

bool  CAgvWifi::CheckUsbWifi()
{

    const char *usbWifi[] =
    {
            "0bda:8179",
            "0bda:818b",
    };

    for (unsigned int i = 0; i < sizeof(usbWifi) / sizeof(char *); i++)
    {
        if (checkUsbDeviceIsExist(usbWifi[i]))
        {
            return true;
        }
    }
    printf("Cound'nt found a usb wifi device!\n");
    return false;
}



int CAgvWifi::PowerOn(int mode, string ifname)
{
    devName = ifname;
    haveUsbWifi = CheckUsbWifi();
	if (0 == ChangeMode(mode))
	{
		status = DevReady;
		return 0;
	}

	return -1;
}

int CAgvWifi::ChangeMode(int mode)
{

    if(mode == currentMode)
    {
        return 0;
    }

    if(StandMode == mode)
    {
        if (CheckConnectStatus())
        {
            currentMode = mode;
            return 0;
        }
        int ret = -1;
        if (CAgvUtils::GetPid("wpa_supplicant") <= 0)
        {
            if (haveUsbWifi)
            {
                ret = system("wpa_supplicant -B -D wext -i wlan0 -c ./tlwifi/wpa_supplicant.conf -d");
            }
            else
            {
                ret = system("wpa_supplicant -B -D wext -i wlan0 -c ./wifi/wpa_supplicant.conf -d");
            }
        }
        else
        {
            ret = 0;
        }
        if(0 != ret)
        {
            perror("start wifi sta mode error");
            return -1;
        }
        else
        {
            printf("start wifi sta mode success\n");
        }

        currentMode = mode;

        return 0;
    }
    else if(ApMode == mode)
    {

        if (CAgvUtils::GetPid("udhcpd") > 0)
        {
            CAgvUtils::KillProcess("udhcpd", true);
        }
        if (CAgvUtils::GetPid("wpa_supplicant") > 0)
        {
            CAgvUtils::KillProcess("wpa_supplicant", true);
        }

        int ret = -1;
        if (haveUsbWifi)
        {
            ret = system("cd /opt/tlwifi && ./turn_on_the_ap.sh");
        }
        else
        {
            ret = system("cd /opt/wifi && ./ifup_ap_wifi.sh");
        }
        if(0 != ret)
        {
            perror("ifup_ap_wifi");
            return -1;
        }
        else
        {
            printf("Enter AP Mode\n");
            currentMode = mode;
            return  0;
        }
//        sleep(2);
    }

    return 1;

}

int CAgvWifi::GetAPList(vector<string> &aplist)
{
    char cmd[256]= {0};
    sprintf(cmd, "iwlilst %s scan", devName.c_str());
    FILE *pfp = popen(cmd, "r");
    if (pfp == NULL)
    {
        return -1;
    }
    int i = 0, len = 0;
    char buf[1024] = {0}, *pstr = NULL;
    char ssid[64],bsid[64],level[16], str[128];
    while (fgets(buf, sizeof(buf) - 1, pfp))
    {
        printf("%s", buf);

        len = strlen(buf);
        if (buf[len - 1] == '\n')
        {
            buf[len - 1] = 0;
        }

        if ((pstr = strstr(buf,"Address: ")) != NULL)
        {
            pstr += 9;
            strncpy(bsid, pstr, sizeof(bsid) - 1);
        }
        else if ((pstr = strstr(buf, "ESSID:")) != NULL)
        {
            pstr += 6 + 1;
            buf[len - 1 - 1] = 0;
            strncpy(ssid, pstr, sizeof(ssid) - 1);
        }
        else if ((pstr = strstr(buf, "Signal level=")) != NULL)
        {
            pstr += 13;
            strncpy(level, pstr, sizeof(level) - 1);

            snprintf(str, sizeof(str) - 1, "%s %s %s", ssid, bsid, level);
            aplist[i] = str;
            i++;
        }
    }

    pclose(pfp);
	return -1;
}

int CAgvWifi::CheckApIsExist(const char* apName)
{
    if (apName == NULL || strlen(apName) == 0)
    {
        return -1;
    }

    char cmdBuf[128];

    snprintf(cmdBuf, sizeof(cmdBuf) - 1, "iwlist %s scan | grep -w %s | wc -l", devName.c_str(), apName);

    printf("cmd: %s\n", cmdBuf);

    FILE* pfp = popen(cmdBuf, "r");
    if (pfp == NULL)
    	return -1;

    int  existApNum = 0;
    char apBuf[10];

    if(fgets(apBuf, 10, pfp) != NULL)
    {
        existApNum = atoi(apBuf);
        printf("exist AP num: %d\n", existApNum);
        pclose(pfp);
    }
    else
    {
        pclose(pfp);
        return -1;
    }

    return existApNum;


}

int CAgvWifi::SetApConnection(const char *apName, const char *pwd)
{
    char fname[256] = {0}, buf[256] = {0};
    snprintf(fname, sizeof(fname) - 1, "./%s/wpa_supplicant.conf", haveUsbWifi ? "tlwifi" : "wifi");
    FILE *fp = fopen(fname, "w+");
    if (fp == NULL)
    {
        perror("Open wpa_supplicant.conf failed");
        return -1;
    }
    snprintf(buf, sizeof(buf) - 1,    "ctrl_interface=/var/run/wpa_supplicant\n"
                                      "network={\n"
                                            "ssid=\"%s\"\n"
                                            "psk=\"%s\"\n"
                                      "}", apName, pwd);
    fputs(buf, fp);

    fclose(fp);

    return 0;
}

int CAgvWifi::ConnectToAp(int timeout)
{
    if(StandMode != currentMode)
    {
        printf("Wifi Mode Error!\n");
        return -1;
    }

    if (CheckConnectStatus())
    {
        return 0;
    }


    status = Connecting;

    printf("Start router Sta\n");

    vector<string> aplist;
    //Figure:iwlist
    if(GetAPList(aplist) < 0)
    {
        printf("Get Ap list failed!\n");
        status = DevReady;
        return -1;
    }

    if (CAgvUtils::GetPid("udhcpd") > 0)
    {
        CAgvUtils::KillProcess("udhcpd", true);
    }

    int ret = -1;

    if (CAgvUtils::GetPid("wpa_supplicant") <= 0)
    {
        if (haveUsbWifi)
        {
            ret = system("wpa_supplicant -B -D wext -i wlan0 -c ./tlwifi/wpa_supplicant.conf -d");
        }
        else
        {
            ret = system("wpa_supplicant -B -D wext -i wlan0 -c ./wifi/wpa_supplicant.conf -d");
        }
    }
    else
    {
        ret = 0;
    }

    if (ret != 0)
    {
        perror("wpa_supplicant");
        status = DevReady;
        return -2;
    }

    if (timeout > 0)
    {
        int retry = timeout;
        while(!CheckConnectStatus() && retry--)
        {
            sleep(1);
        }
    }
    if (CheckConnectStatus())
    {
        return 0;
    }
    return 1;

    return 1;

}


int CAgvWifi::SetIpAddress(const char* ip, bool dhcp)
{
    if (dhcp)
    {
        if (currentMode == StandMode)
        {
            return CAgvUtils::SetIpAddress(devName.c_str(), ip);
        }
        return -1;
    }

    if (ip == NULL || strlen(ip) < 7)
        return -1;

    if (CAgvUtils::GetPid("udhcpd") > 0)
    {
        CAgvUtils::KillProcess("udhcpd", true);
    }

    return CAgvUtils::StartDhcp(devName.c_str(), 30);

}

bool CAgvWifi::isConnected()
{
	if (status == Connected)
	{
	    return true;
	}
	return false;
}


bool CAgvWifi::CheckConnectStatus()
{
    bool ret = false;

    char buf[16] = {0};
    char path[256] = {0};
    snprintf(path, sizeof(path) - 1, "/sys/class/net/%s/operstate", devName.c_str());
    int fd = open(path, O_RDONLY);
    if (fd > 0)
    {
        if (read(fd, buf, sizeof(buf) - 1) > 0)
        {
            if (strstr(buf, "up"))
            {
                ret = true;
                status = Connected;
            }
        }
        close(fd);
        if (!ret && status == Connected)
        {
            status = DevReady;
        }
    }
    else
    {
        status = DevOff;
    }
    return ret;
}

int CAgvWifi::Disconnect()
{
	if (status != DevReady && status != DevOff)
	{
		CAgvUtils::KillProcess("wpa_supplicant", true);
		CAgvUtils::KillProcess("udhcpc", true);

		status = DevReady;
	}
    return 0;
}


int CAgvWifi::PowerOff()
{
	if (status != DevOff)
	{
		Disconnect();
		status = DevOff;
	}
	return 0;
}
