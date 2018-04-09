#ifndef AGVSOCKETCAN_H
#define AGVSOCKETCAN_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <limits.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <errno.h>


using namespace std;

class CAgvSocketCan
{
    public:
        CAgvSocketCan();
        ~CAgvSocketCan();

    public:
        int Initial(int portId, int baudrate, bool isNonBlock);
        int Write(uint can_id, const unsigned char *pDataIn,uint len);
        inline void SetReadTimeout(int ms) {readTimeout = ms;}
        int Read(struct can_filter *pFilterIn,uint filter_count,uint *pCanIdOut,uint *pLenOut,unsigned char *pDataOut);
        int Close();

    private:
        int setNonBlocking(int sfd);
        int fd_can;
        bool isBlock;
        int  readTimeout;
        struct can_filter *lastFilter;
        unsigned int    lastFilterCount;
};


#endif // AGVSOCKETCAN_H
