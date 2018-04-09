/*
 * AgvSchProtocol.h
 *
 *  Created on: 2016-9-22
 *      Author: zxq
 */

#ifndef AGVSCHPROTOCOL_H_
#define AGVSCHPROTOCOL_H_

#include <vector>
using namespace std;


class HeartBeat
{
    public:
            int packData(unsigned char *data, int maxlen);
            int unPackData(const unsigned char *data, int len);
    public:
        unsigned short  id;             //AGV ID
};


class ReportStatus
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned char   status;         //����״̬
        unsigned char   powerLevel;     //����,ʵ�ʵ���ֵ,ȡ����
        unsigned char   speed;          //�ٶ�,ʵ���ٶ�ֵ,ȡ����
        unsigned int    xPos;           //��ǰ����X����,��λmm
        unsigned int    yPos;           //��ǰ����Y����,��λmm
        unsigned short  carAngle;       //��ǰ����Ƕ�
        unsigned short  lastPoint;      //���һ�ζ�����վ�����
        unsigned char   reserverd[3];   //����
};

class ReportActionStatus
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  taskId;         //������
        unsigned char   status;         //����״̬
                                        //0x01������
                                        //0x02������
        unsigned char   reserverd[3];   //����

};

class RequestPath
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  lastPoint;      //�ϸ�վ�����
        unsigned int    xPos;           //��ǰ����X����
        unsigned int    yPos;           //��ǰ����Y����
};

class AckToServer
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  cmd;            //����
        unsigned short  result;         //ִ�н�� 0x0000�ɹ���0x0001ʧ�ܣ�������ֵ��Ϊ�����롣
        unsigned short  dataLength;     //�������ݳ���
        unsigned char   *data;          //��������
};

class ReportTaskStatus
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  taskId;         //����ID
        unsigned char   taskStatus;     //����ִ��״̬ 0x01������ ��0x02������
        unsigned char   reserved[3];
};

class StationInfo
{
    public:
        unsigned short  id;             //վ��ID
        unsigned int    xPos;           //վ���X���꣨��λmm��
        unsigned int    yPos;           //վ���Y���꣨��λmm��
        double          pathAngle;      //����·���Ƕ�
        unsigned short  turnRadius;     //ת��뾶����λmm��ֱ��ʱΪ0
        unsigned char   mode;           //����ģʽ��
                                        //      0x01 ǰ��
                                        //      0x02 ����
                                        //      0x03 ��ת
                                        //      0x04 ��ת
        unsigned char   action;         //��վ������
                                        //      0x00 ���ֲ���
                                        //      0x01 ֹͣ
                                        //      0x02 ����
                                        //      0x03 �ָ��ٶ�
                                        //      0x04 �������
                                        //      0x05 �������
        unsigned short  liftHeight;     //�������߶ȣ���λmm��Ϊ0xFFFFʱ����
};


class SetPath
{
    public:
        SetPath();
        ~SetPath();
        int unPackData(const unsigned char *data, int len);
        static int GetStationInfo(const unsigned char *data, int len, StationInfo *info);
    public:
        unsigned short          stationCount;
        vector<StationInfo*>    stationList;
};


class TestStationInfo
{
    public:
        unsigned short  id;             //վ��ID
        unsigned int    xPos;           //վ���X���꣨��λmm��
        unsigned int    yPos;           //վ���Y���꣨��λmm��
        unsigned char   direction;      //ǰ������ 0x00 ǰ��    0x01 ����
        unsigned short  turnRadius;     //ת��뾶����λmm��ֱ��ʱΪ0
        unsigned short  turnAngle;      //ת��Ƕ�, 0-360��
        unsigned char   turnMode;       //ת�䷽ʽ�� 0x00 ��ת��0x01 ��ת��0x02 ��ת
        unsigned char   action;         //��վ������
                                        //      0x00��ͣ��
                                        //      0x01ͣվ��
                                        //      0x02���ٵ���С�ٶȡ�
                                        //      0x03���ٵ������ٶȣ�
                                        //      0x04 ֹͣ����
                                        //      0x05 ֹͣ��ж��
                                        //      0x06 ֹͣ�������
        unsigned short  liftHeight;     //�������߶ȣ���λmm��Ϊ0xFFFFʱ����
};

class SetTestPath
{
    public:
        SetTestPath();
        ~SetTestPath();

        int unPackData(const unsigned char *data, int len);
        static int GetStationInfo(const unsigned char *data, int len, TestStationInfo *info);
    public:
        TestStationInfo     stationInfo;
};

class DoAction
{
    public:
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned char   action;         //����: 0x00���У�0x01ֹͣ��
        unsigned char   reserved[3];
};


class SetSpeed
{
    public:
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned short  speed;          //�ٶ�ֵ*100��ȡ����
        unsigned char   reserved[2];
};


class ServerAck
{
    public:
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned short  cmd;            //����
        unsigned short  result;         //ִ�н�� 0x0000�ɹ���0x0001ʧ�ܣ�������ֵ��Ϊ�����롣
        unsigned short  dataLength;     //�������ݳ���
        unsigned char   *data;          //��������
};
class StopCar
{
    public:
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned short agvId;
        unsigned short agvState;
};


class SystemTime
{
    public:
        int packData(unsigned char *data, int maxlen);
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned short m_agvID;
        unsigned short m_year;
        unsigned short m_mon;
        unsigned short m_mday;
        unsigned short m_hour;
        unsigned short m_min;
        unsigned short m_sec;
        unsigned char m_wday;
        unsigned short m_yday;
        unsigned char m_isDst;
        unsigned char m_reserved[3];
};


class CAgvSchProtocol
{
    public:
        enum CMD{
            ServerSendPath      = 0x0201,   //��������AGV����·������
            ServerSendAction    = 0x0202,   //������ָ��AGVִ��ĳ������
            ServerSetSpeed      = 0x0203,   //����������AGV�������ٶ�
            ServerResponseAck   = 0x0204,   //������������AGV��ָ��ִ������Ļظ�
            ServerChargerTurnOn = 0x0205,
            ServerChargerTurnOff= 0x0206,
            ServerSynchronizeTime = 0x0207,
            ServerSetBarrierDistance = 0x0208,
            ServerSetAgvSleep   = 0x0209,
            ServerSetAgvWakeUp  = 0x0210,
            ServerRunTest       = 0xFFFF,   //���߲�������

            AgvReportStatus     = 0x0101,   //AGV�����������AGV��ǰ��״̬
            AgvRequestStation   = 0x0102,   //AGV����ȥ��ĳ������
            AgvHeartBeat        = 0x0103,   //AGV������
            AgvCmdAck           = 0x0104,   //AGV�����Է�������ָ��ִ������Ļظ�
            AgvReportTaskStatus = 0x0105,   //AGV��������״̬
            AgvRequestSystemTimeSyn = 0x0107,
        };
        CAgvSchProtocol();
        CAgvSchProtocol(int cmd, unsigned char *data, int len);
        virtual ~CAgvSchProtocol();

        bool CheckProtocol(const unsigned char *data, int len, int *start, int *end);
        int PackData(unsigned char *data);
        int UnPackData(const unsigned char *data, int len, int *left);
    private:
        int encode(const unsigned char *src, int len, unsigned char *des);
        int decode(const unsigned char *src, int len, unsigned char *des);
    public:
        unsigned char   head;       //֡ͷ:0xFF����ʶ֡�Ŀ�ʼ��
        unsigned short  length;     //���ȣ���ʾ��[У����]��ʼ��[����]�������ֽ�������N+6����
        unsigned short  checkSum;   //У�����㷨��CRC16����[Э���ʶ]��[����]��У��ֵ��
        unsigned short  type;       //Э���ʶ��Ĭ��Ϊ0x0001�����ڱ�ʶ��ͬ���͵�Э�飻
        unsigned short  cmd;        //����
        unsigned char   *data;      //������������
        unsigned short  dataLength; //�������ݳ���
        unsigned char   tail;       //֡β��0xFF����ʶ֡�Ľ�����
    private:
        unsigned char buffer[2048];
};

#endif /* AGVSCHPROTOCOL_H_ */
