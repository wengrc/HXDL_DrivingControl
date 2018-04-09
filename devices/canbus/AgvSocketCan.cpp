#include "AgvSocketCan.h"

#define PF_CAN  29

CAgvSocketCan::CAgvSocketCan()
{
    fd_can = -1;
    readTimeout = 0;
    isBlock = true;
    lastFilter = NULL;
    lastFilterCount = 0;
}


CAgvSocketCan::~CAgvSocketCan()
{

}


int CAgvSocketCan::setNonBlocking(int sfd)
{
    int  flags = 1;
    int  res;


    flags = fcntl (sfd, F_GETFL, 0);

    if (flags == -1)
    {
        perror ("fcntl");

        return -1;
    }

    flags |= O_NONBLOCK;
    res    = fcntl (sfd, F_SETFL, flags);

    if (res == -1)
    {
        perror ("fcntl");

        return -1;
    }

    isBlock = false;
    return 0;
}


int CAgvSocketCan::Initial(int id, int baudrate, bool isNonBlock)
{
    char ifname[16] = {0}, cmd[256] = {0};
    struct ifreq  ifr;
    struct sockaddr_can  addr;

    snprintf(ifname, sizeof(ifname) - 1, "can%d", id);
    sprintf(cmd, "ifconfig %s down", ifname);

    if(system(cmd) < 0)
    {
        perror("Close can interface failed!");
        return -1;
    }
    sprintf(cmd, "echo %d > /sys/devices/platform/FlexCAN.%d/bitrate", baudrate, id);
    if(-1 == system(cmd))
    {
        perror("Set can bus baudrate failed!");
        return -2;
    }
    sprintf(cmd, "ifconfig %s up", ifname);
    if(-1 == system(cmd))
    {
        return -3;
    }

    fd_can = socket (PF_CAN, SOCK_RAW, CAN_RAW);

    if (fd_can < 0)
    {
        perror ("CanSocket Init error");

        return -3;
    }

    addr.can_family = PF_CAN;

    snprintf(ifr.ifr_name, 16, "can%d", id);

    if ( ioctl (fd_can, SIOCGIFINDEX, &ifr) )
    {
        perror ("CanSocket ioctl error");

        return -1;
    }

    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind ( fd_can, (struct sockaddr *)&addr, sizeof(addr) ) < 0)
    {
        perror ("bind");

        return -2;
    }

    if (isNonBlock)
    {
        if (setNonBlocking (fd_can) != 0)
        {
            printf ("CanSocket Invoke setNonBlocking failed");

            return -4;
        }
        isBlock = false;
    }

    lastFilter = NULL;
    lastFilterCount = 0;
    return 0;
}


int CAgvSocketCan::Write(uint can_id,
                         const unsigned char *pDataIn,
                         uint len)
{
    if (len > 8)
    {
        return -1;
    }

    struct can_frame  frame;
    memset ( &frame, 0, sizeof(struct can_filter) );
    frame.can_id  = can_id;
    frame.can_id &= CAN_EFF_MASK;
    frame.can_id |= CAN_EFF_FLAG; //
    frame.can_dlc = len;
    memcpy (frame.data, pDataIn, len);

    if ( -1 == write ( fd_can, &frame, sizeof(frame) ) )
    {
        perror ("CanSocket Write error");

        return -1;
    }

    return 0;
}


int CAgvSocketCan::Read(struct can_filter *pFilterIn,
                        uint filter_count,
                        uint *pCanIdOut,
                        uint *pLenOut,
                        unsigned char *pDataOut)
{
    if (pFilterIn != NULL && filter_count > 0 &&
       (pFilterIn != lastFilter || filter_count != lastFilterCount))
    {
        lastFilter = pFilterIn;
        lastFilterCount = filter_count;
        if (setsockopt ( fd_can,
                         SOL_CAN_RAW,
                         CAN_RAW_FILTER,
                         pFilterIn, filter_count * sizeof(struct can_filter) ) != 0)
        {
            perror ("CanSocket setsockopt error");
            return -1; //
        }
    }

    struct can_frame  frame;
    memset ( &frame, 0, sizeof(struct can_filter) );

    if (isBlock && readTimeout > 0)
    {
        fd_set canfd_set;
        FD_ZERO(&canfd_set);
        FD_SET(fd_can, &canfd_set);

        struct timeval tout;
        tout.tv_sec = readTimeout / 1000;
        tout.tv_usec = (readTimeout % 1000) * 1000;

        int ret = select(fd_can + 1, &canfd_set, NULL, NULL, &tout);
        if (ret < 0)
        {
            perror("CanSocket read select error");
            if (errno != EINTR)
                return -2;
            else
                return 0;
        }
        else if (ret == 0)
        {
            return 0;
        }
        if (!FD_ISSET(fd_can, &canfd_set))
        {
            return 0;
        }
    }

    int  nbytes = read ( fd_can, &frame, sizeof(struct can_frame) );

    if (nbytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return -3; //Figu:ignore this error
        }
        else
        {
            perror ("CanSocket Read error");

            return -2;
        }
    }
    else
    {
        if (frame.can_id & CAN_EFF_FLAG)
        {
            *pCanIdOut = frame.can_id & CAN_EFF_MASK;
        }
        else
        {
            *pCanIdOut = frame.can_id & CAN_SFF_MASK;
        }

        *pLenOut = frame.can_dlc;

        memcpy (pDataOut, frame.data, frame.can_dlc);

        /*
         *  if (frame.can_id & CAN_RTR_FLAG)
         *  {
         *   //Figu:remote request
         *  }*/
    }

    return 0;
}


int CAgvSocketCan::Close()
{
    if (fd_can > 0)
    {
        if ( -1 == close (fd_can) )
        {
            perror ("CanSocket Close error");

            return -1;
        }
        fd_can = 0;
        lastFilter = NULL;
        lastFilterCount = 0;
    }

    return 0;
}


