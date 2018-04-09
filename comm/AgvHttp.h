#ifndef AGVHTTP_H
#define AGVHTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl/curl.h"

#include <iostream>
using namespace std;


class CAgvHttp
{
    public:
        CAgvHttp();
        ~CAgvHttp();

    public:
        static int PostData(const char *url,const char *content,int timeout,string *output);
        static int Download(const char *url,const char *outFileName);
};


#endif // AGVHTTP_H


