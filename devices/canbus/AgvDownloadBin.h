#ifndef AGVDOWNLOADBIN_H
#define AGVDOWNLOADBIN_H

#include "AgvSocketCan.h"




class CAgvDownLoadBin
{
    public:
        CAgvDownLoadBin();
        ~CAgvDownLoadBin();

    public:
        int SendFileViaCanBus(const char *pFileName,CAgvSocketCan &socket_can, int boardNo);

    private:
        int readDataToAckCheck(CAgvSocketCan &socket_can,unsigned char *pSendBuf,unsigned char *pReadBuf,int len);


};


#endif
