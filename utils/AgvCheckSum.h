/**
 * Copyright(c)2006,ZHIJIU All Right Reserved
 * File Name:AgvCheckSum.cpp
 * File description:the class of checksum
 * Current Version:1.0.0.1
 * Author:Figu Lin
 * Date:4/26/2016
 */

#ifndef AGVCHECKSUM_H
#define AGVCHECKSUM_H

#include <iostream>
using namespace std;

class CAgvCheckSum
{

    public:
        CAgvCheckSum();
        ~CAgvCheckSum();

    public:
        static unsigned short GetCrc16(const unsigned char *data, unsigned short len, unsigned short pwd);
        static int GetSha1(const char* file, string* sha1Code);
};

#endif // AGVCHECKSUM_H
