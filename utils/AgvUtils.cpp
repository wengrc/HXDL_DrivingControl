/*
 * AgvUtils.cpp
 *
 *  Created on: 2016-9-1
 *      Author: zxq
 */

#include "AgvUtils.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>
#include <iostream>

CAgvUtils::CAgvUtils() {

}

CAgvUtils::~CAgvUtils() {

}

int CAgvUtils::Buffer2Bin(const unsigned char *data, int bytes)
{
    if (data == NULL || bytes == 0)
    {
        return -1;
    }
    int ret = 0;
    for (int i = 0; i < bytes; i++)
    {
        ret |= data[i] << (8 * (bytes - i - 1));
    }

    return ret;
}

int CAgvUtils::Bin2Buffer(int data, int bytes, unsigned char *buf)
{
    if (buf == NULL || bytes == 0)
        return -1;
    for (int i = 0; i < bytes; i++)
    {
        buf[i] = (data >> (8 * (bytes - i - 1))) & 0xFF;
    }
    return bytes;
}


int CAgvUtils::GetPid(const char* process)
{
	if (process == NULL || strlen(process) == 0)
		return -1;

	char buf[64] = {0};
	snprintf(buf, sizeof(buf) - 1, "pidof %s", process);
	FILE *fp = popen(buf, "r");
	if (fp)
	{
		int pid = 0;
		if (fgets(buf, sizeof(buf) - 1, fp) != NULL)
			pid = atoi(buf);
		pclose(fp);
		return pid;
	}

	return -1;
}

int CAgvUtils::KillProcess(const char* process, bool wait)
{
	if (process == NULL || strlen(process) == 0)
		return -1;

	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd) - 1, "killall %s", process);
	int ret = system(cmd);
	if (ret == 0 && wait)
	{
		while(GetPid(process) > 0)
			usleep(100 * 1000);
	}
	return ret;
}

int CAgvUtils::CheckIpAddress(const char *ip,bool *isExsiting)
{
	if (ip == NULL || isExsiting == NULL)
		return -1;

    *isExsiting = false;

    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void * tmpAddrPtr = NULL;

    if(getifaddrs(&ifAddrStruct) != 0)
    {
        perror("getifaddrs");
        return -1;
    }
    else
    {
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
        {

            if (ifa->ifa_addr == NULL)
                continue;

            if(ifa->ifa_addr->sa_family == AF_INET)
            {

                tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                memset(addressBuffer,0,sizeof addressBuffer);

                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

                if(0 == strncmp(addressBuffer,ip,strlen(ip)))
                {
                    *isExsiting = true;
                    break;
                }
            }
        }
    }

    if (ifAddrStruct != NULL)
    {
        freeifaddrs(ifAddrStruct);
    }

    return 0;
}

int CAgvUtils::GetIpAddress(const char* ifname, char* ip, unsigned int size)
{
    struct ifaddrs *ifap0, *ifap;
    //int ret;
    char buf[NI_MAXHOST];
    struct sockaddr_in *addr4;

    if (NULL == ifname || ip == NULL)
    {
        return -1;
    }

    if (getifaddrs(&ifap0))
    {
        printf("getifaddrs failed! %s\n", strerror(errno));
        return -1;
    }

    for (ifap = ifap0; ifap != NULL; ifap = ifap->ifa_next)
    {
        if (strcmp(ifname, ifap->ifa_name) != 0)
            continue;
        if (ifap->ifa_addr == NULL)
            continue;
        if ((ifap->ifa_flags & IFF_UP) == 0)
            continue;

        if (AF_INET == ifap->ifa_addr->sa_family)
        {
            addr4 = (struct sockaddr_in *) ifap->ifa_addr;
            if (NULL != inet_ntop(ifap->ifa_addr->sa_family, (void *) &(addr4->sin_addr), buf, NI_MAXHOST))
            {
                if (size <= strlen(buf))
                {
                    printf("IPV4 inet_ntop size error!\n");
                    break;
                }
                strcpy(ip, buf);
                freeifaddrs(ifap0);
                return 0;
            }
            else
            {
                printf("IPV4 inet_ntop failed! %s\n", strerror(errno));
                break;
            }
        }
    }
    freeifaddrs(ifap0);

    return 1;
}

int CAgvUtils::CheckNetStatus(const char* ifname)
{
    int ret = -1;
    char cmd[64], buf[256];
    sprintf(cmd, "ifconfig %s", ifname);
    FILE *fifret = popen(cmd, "r");
    if (fifret)
    {
        while(fgets(buf, sizeof(buf) - 1, fifret))
        {
            if (strstr(buf, "Device not found"))
            {
                ret = -1;
                break;
            }
            if (strstr(buf, "RUNNING"))
            {
                ret = 0;
                break;
            }
            else if (strstr(buf, "UP"))
            {
                ret = 1;
                break;
            }

        }
        pclose(fifret);
    }
    return ret;
}


int CAgvUtils::SetIpAddress(const char *ifname, const char *ip)
{
    if (ifname == NULL || ip == NULL)
    {
        return -1;
    }
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd) - 1, "ifconfig %s %s up", ifname, ip);
    return system(cmd);
}


int CAgvUtils::StartDhcp(const char *ifname, int timeout)
{
    if (ifname == NULL)
    {
        return -1;
    }
    if (GetPid("udhcpc") > 0)
    {
        KillProcess("udhcpc", true);
    }

    char cmd[256] = {0};
    if (timeout > 0)
    {
        snprintf(cmd, sizeof(cmd) - 1, "udhcpc -i %s -t %d -n", ifname, timeout);
    }
    else if (timeout == 0)
    {
        snprintf(cmd, sizeof(cmd) - 1, "udhcpc -i %s -b ", ifname);
    }
    else if (timeout < 0)
    {
        snprintf(cmd, sizeof(cmd) - 1, "udhcpc -i %s ", ifname);
    }

    return system(cmd);
}
