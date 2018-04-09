/*
 * AgvSchProtocol.cpp
 *
 *  Created on: 2016-9-22
 *      Author: zxq
 */
#include "AgvSchProtocol.h"
#include "../AgvPublic.h"
#include "../utils/AgvCheckSum.h"

#define LOGTAG  "AgvSchProtocol"

//i += Bin2Buffer(id, sizeof(id), data + i);
#define PackSectionData(section, data, offset)      do{ offset += Bin2Buffer(section, sizeof(section), data + offset);  }while(0);

#define PackEmptySection(section, data, offset)     do{ offset += sizeof(section); }while(0);

//     speed = Buffer2Bin(data + i, sizeof(speed));   i += sizeof(speed);
#define UnPackSectionData(section, data, offset)    do{ section = Buffer2Bin(data + offset, sizeof(section)); offset += sizeof(section); }while(0);
//memcpy(reserved, data + i, sizeof(reserved));   i += sizeof(reserved);
#define UnPackEmptySection(section, data, offset)   do{ memcpy(section, data + offset, sizeof(section)); offset += sizeof(section); }while(0);

unsigned int Buffer2Bin(const unsigned char *data, int bytes)
{
    if (data == NULL || bytes == 0)
    {
        return -1;
    }
    unsigned int ret = 0;
    for (int i = 0; i < bytes; i++)
    {
        ret |= data[i] << (8 * (bytes - i - 1));
    }

    return ret;
}

int Bin2Buffer(unsigned int data, int bytes, unsigned char *buf)
{
    if (buf == NULL || bytes == 0)
        return -1;
    for (int i = 0; i < bytes; i++)
    {
        buf[i] = (data >> (8 * (bytes - i - 1))) & 0xFF;
    }
    return bytes;
}

CAgvSchProtocol::CAgvSchProtocol()
{
    head = tail = 0xFF;
    checkSum = 0;
    type = 0x0001;
    length = 0;
    cmd = 0;
    data = NULL;
    dataLength = 0;
}

CAgvSchProtocol::CAgvSchProtocol(int cmd, unsigned char *data, int len)
{
    head = 0xFF;
    tail = 0xFF;
    checkSum = 0;
    type = 0x0001;
    length = len + 6;
    this->cmd = cmd;
    this->data = data;
    dataLength = len;
}

CAgvSchProtocol::~CAgvSchProtocol()
{

}
//FF 2C 6A 00 01 00 08 00 EF 01 01 00 00 00 00 FF
bool CAgvSchProtocol::CheckProtocol(const unsigned char *buf, int len, int *start, int *end)
{
    if (buf == NULL || len < 10)
        return false;
    int hi = -1, ti = -1;
    for (int i = 0; i < len; i++)
    {
        if (hi < 0 && buf[i] == head)
        {
            hi = i;
        }
        if (hi >= 0 && ti < 0 && buf[i] == tail)
        {
            if (i - hi > 8)
            {
                ti = i;
                break;
            }
            else
            {
                hi = i;
            }
        }
    }

    if (ti - hi > 8)
    {
        if (start != NULL)
        {
            *start = hi;
        }
        if (end != NULL)
        {
            *end = ti;
        }
        return true;
    }
    return false;
}

int CAgvSchProtocol::PackData(unsigned char *pbuf)
{
    if (pbuf == NULL)
        return -1;

    unsigned char *buf = buffer;
    memset(buffer, 0, sizeof(buffer));
    checkSum = 0;
    int i = 0;
    PackSectionData(length,     buf, i);
    PackSectionData(checkSum,   buf, i);
    PackSectionData(type,       buf, i);
    PackSectionData(cmd,        buf, i);
    dataLength = length - 6;
    if (dataLength > 0 && data != NULL)
    {
        memcpy(buf + i, data, dataLength);
        i += dataLength;
    }
    checkSum = CAgvCheckSum::GetCrc16(buf + 4, i - 4, 0);
    Bin2Buffer(checkSum, sizeof(checkSum), buf + 2);

    pbuf[0] = head;
    int len = encode(buf, i, pbuf + 1);
    pbuf[len + 1] = tail;
    len += 2;

    return len;
}

int CAgvSchProtocol::UnPackData(const unsigned char *pbuf, int len, int *left)
{
    int start = -1, end = -1;
    if (!CheckProtocol(pbuf, len, &start, &end))
    {
        LogError(LOGTAG, "Protocol check failed!\n");
        return -1;
    }
    if (left != NULL)
    {
        *left = len - end - 1;
    }
#ifdef COMDEBUG
    printf("Protocol check ok! head:%d, tail:%d\n", start, end);
#endif
    int dlen = -1, i = 0;

    unsigned char *buf = buffer;
    memset(buffer, 0, sizeof(buffer));
    dlen = decode(pbuf + start + 1, end - start - 1, buf);

    UnPackSectionData(length, buf, i);
    UnPackSectionData(checkSum, buf, i);

    unsigned short crc = CAgvCheckSum::GetCrc16(buf + 4, dlen - 4, 0);
    if (checkSum != crc)
    {
        LogError(LOGTAG, "Protocol CRC check failed! calc:%04X , get:%04X!\n", crc, checkSum);
        return -2;
    }
    UnPackSectionData(type, buf, i);
    UnPackSectionData(cmd, buf, i);
    dataLength = length - 6;
    if (dataLength > 0)
    {
        this->data = &buf[i];
    }

    return dlen + 2;
}

int CAgvSchProtocol::encode(const unsigned char* src, int len, unsigned char* des)
{
    if (src == NULL || des == NULL)
        return -1;

    int i = 0, j = 0;
    for (i = 0, j = 0; i < len; i++)
    {
        if (src[i] == head)
        {
            des[j++] = 0x85;
            des[j++] = 0x86;
        }
        else if (src[i] == 0x85)
        {
            des[j++] = 0x85;
            des[j++] = 0x85;
        }
        else if (src[i] == 0x86)
        {
            des[j++] = 0x86;
            des[j++] = 0x86;
        }
        else
        {
            des[j++] = src[i];
        }
    }

    if (0)//(j != len)
    {
        printf("Befor Encode Data[%d]:", len);
        for (int i = 0; i < len; i++)
            printf("%02X ", src[i]);
        printf("\n");
        printf("After Encode Data[%d]:", j);
        for (int i = 0; i < j; i++)
            printf("%02X ", des[i]);
        printf("\n");
    }

    return j;
}

int CAgvSchProtocol::decode(const unsigned char* src, int len, unsigned char* des)
{
    if (src == NULL || des == NULL)
        return -1;

    int i = 0, j = 0;
    for (i = 0, j = 0; i < len; i++)
    {
        if (src[i] == 0x85)
        {
            if (i == (len -1))
            {
                return -1;
            }
            if (src[i + 1] == 0x85)
            {
                i++;
                des[j++] = 0x85;
            }
            else if (src[i + 1] == 0x86)
            {
                i++;
                des[j++] = 0xFF;
            }
            else
            {
                return -1;
            }
        }
        else if (src[i] == 0x86)
        {
            if (i == (len -1))
            {
                return -1;
            }
            if (src[i + 1] == 0x86)
            {
                i++;
                des[j++] = 0x86;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            des[j++] = src[i];
        }
    }

    /*
    if (j != len)
    {
        printf("Befor Decode Data[%d]:", len);
        for (int i = 0; i < len; i++)
            printf("%02X ", src[i]);
        printf("\n");
        printf("After Decode Data[%d]:", j);
        for (int i = 0; i < j; i++)
            printf("%02X ", des[i]);
        printf("\n");
    }
    */

    return j;
}

int HeartBeat::packData(unsigned char *data, int maxlen)
{
    if (maxlen < 2)
        return -1;

    int i = 0;
    PackSectionData(id, data, i);
    return i;
}

int HeartBeat::unPackData(const unsigned char *data, int len)
{
    if (len < 2)
        return -1;
    int i = 0;
    UnPackSectionData(id, data, i)

    return i;
}

int ReportStatus::packData(unsigned char *data, int maxlen)
{
    if (data == NULL || maxlen < 20)
        return -1;
    int i = 0;
    PackSectionData(id,         data, i);
    PackSectionData(status,     data, i);
    PackSectionData(powerLevel, data, i);
    PackSectionData(speed,      data, i);
    PackSectionData(xPos,       data, i);
    PackSectionData(yPos,       data, i);
    PackSectionData(carAngle,   data, i);
    PackSectionData(lastPoint,  data, i);
    PackEmptySection(reserverd, data, i);
    return i;
}

int ReportActionStatus::packData(unsigned char *data, int maxlen)
{
    if (data == NULL || maxlen < 8)
        return -1;
    int i = 0;
    PackSectionData(id,         data, i);
    PackSectionData(taskId,     data, i);
    PackSectionData(status,     data, i);
    PackEmptySection(reserverd, data, i);
    return i;
}

int RequestPath::packData(unsigned char *data, int maxlen)
{

    if (data == NULL || maxlen < 12)
        return -1;
    int i = 0;
    PackSectionData(id,         data, i);
    PackSectionData(xPos,       data, i);
    PackSectionData(yPos,       data, i);
    PackSectionData(lastPoint,  data, i);
    return i;
}

int AckToServer::packData(unsigned char *data, int maxlen)
{
    if (data == NULL || maxlen < 8)
        return -1;
    int i = 0;
    PackSectionData(id,         data, i);
    PackSectionData(cmd,        data, i);
    PackSectionData(result,     data, i);
    PackSectionData(dataLength, data, i);
    if (dataLength > 0 && this->data != NULL)
    {
        memcpy(data + i, this->data, dataLength);
        i += dataLength;
    }
    return i;
}


int ReportTaskStatus::packData(unsigned char *data, int maxlen)
{
    if (data == NULL || maxlen < 8)
        return -1;
    int i = 0;
    PackSectionData(id,         data, i);
    PackSectionData(taskId,     data, i);
    PackSectionData(taskStatus, data, i);
    PackEmptySection(reserved,  data, i);

    return i;
}

SetPath::SetPath()
{
    stationCount = 0;
    stationList.reserve(1024);
}

SetPath::~SetPath()
{
    for (unsigned int i = 0; i < stationList.size(); i++)
    {
        delete stationList[i];
    }
}

int SetPath::GetStationInfo(const unsigned char *data, int len, StationInfo *info)
{
    if (info == NULL || len < 18)
    {
        return 0;
    }

    int i = 0;
    UnPackSectionData(info->id,         data, i);
    UnPackSectionData(info->xPos,       data, i);
    UnPackSectionData(info->yPos,       data, i);
    unsigned short pathAngle;
    UnPackSectionData(pathAngle,  data, i);
    info->pathAngle = (double)pathAngle*0.01;
    UnPackSectionData(info->turnRadius, data, i);
    UnPackSectionData(info->mode,       data, i);
    UnPackSectionData(info->action,     data, i);
    UnPackSectionData(info->liftHeight, data, i);

    return i;
}

int SetPath::unPackData(const unsigned char *data, int len)
{
    if (len < 2)
        return -1;

    int i = 0;
    stationList.clear();
    stationCount = Buffer2Bin(data + i, sizeof(stationCount)); i += sizeof(stationCount);
    if (stationCount > 0)
    {
        for (int j = 0; j < stationCount; j++)
        {
            if (i == len)
            {
                printf("Warnning!!!! Path count error! %d - %d\n", j , stationCount);
                break;
            }
            StationInfo *info = new StationInfo;
            i += GetStationInfo(data + i, len - i, info);

            stationList.push_back(info);
        }
    }
    return i;
}


SetTestPath::SetTestPath()
{
}

SetTestPath::~SetTestPath()
{
}

int SetTestPath::GetStationInfo(const unsigned char *data, int len, TestStationInfo *info)
{
    if (info == NULL || len < 19)
    {
        return 0;
    }

    int i = 0;
    UnPackSectionData(info->id,         data, i);
    UnPackSectionData(info->xPos,       data, i);
    UnPackSectionData(info->yPos,       data, i);
    UnPackSectionData(info->direction,  data, i);
    UnPackSectionData(info->turnRadius, data, i);
    UnPackSectionData(info->turnAngle,  data, i);
    UnPackSectionData(info->turnMode,   data, i);
    UnPackSectionData(info->action,     data, i);
    UnPackSectionData(info->liftHeight, data, i);

    return i;
}

int SetTestPath::unPackData(const unsigned char *data, int len)
{
    if (len < 2)
    {
        return -1;
    }

    int i = 0;
    int stationCount = Buffer2Bin(data + i, sizeof(stationCount)); i += sizeof(stationCount);
    if (stationCount > 0)
    {
        i += GetStationInfo(data + i, len - i, &stationInfo);
    }
    return i;
}


int DoAction::unPackData(const unsigned char *data, int len)
{
    if (len < 4)
        return -1;
    int i = 0;
    UnPackSectionData(action,       data, i);
    UnPackEmptySection(reserved,    data, i);
    return i;
}

int SetSpeed::unPackData(const unsigned char *data, int len)
{
    if (len < 4)
        return -1;
    int i = 0;
    UnPackSectionData(speed,       data, i);
    UnPackEmptySection(reserved,   data, i);
    return i;
}

int ServerAck::unPackData(const unsigned char *data, int len)
{
    if (len < 6)
        return -1;
    int i = 0;
    UnPackSectionData(cmd,         data, i);
    UnPackSectionData(result,      data, i);
    UnPackSectionData(dataLength,  data, i);
    if (dataLength > 0)
    {
        this->data = (unsigned char *)data;
        i += dataLength;
    }
    else
    {
        this->data = NULL;
    }
    return i;
}
int StopCar::unPackData(const unsigned char *data, int len)
{
    if (len < 4)
        return -1;
    int i = 0;
    UnPackSectionData(agvId,         data, i);
    UnPackSectionData(agvState,      data, i);
    return i;
}

int SystemTime::unPackData(const unsigned char *data, int len)
{
    if (len < 16)
        ;//return -1;
    int i = 2;
    //UnPackSectionData(m_agvID, data, i)
    UnPackSectionData(m_year, data, i)
    UnPackSectionData(m_mon, data, i)
    UnPackSectionData(m_mday, data, i)
    UnPackSectionData(m_hour, data, i)
    UnPackSectionData(m_min, data, i)
    UnPackSectionData(m_sec, data, i)

    //UnPackSectionData(m_wday, data, i)
    //UnPackSectionData(m_yday, data, i)
    //UnPackSectionData(m_isDst, data, i)
    //UnPackEmptySection(m_reserved, data, i);

    return i;
}

int SystemTime::packData(unsigned char *data, int maxlen)
{
    if (data == NULL || maxlen < 8)
        return -1;
    int i = 0;
    PackSectionData(m_agvID, data, i);
    return i;
}
