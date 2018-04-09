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
        unsigned char   status;         //运行状态
        unsigned char   powerLevel;     //电量,实际电量值,取整数
        unsigned char   speed;          //速度,实际速度值,取整数
        unsigned int    xPos;           //当前所在X坐标,单位mm
        unsigned int    yPos;           //当前所在Y坐标,单位mm
        unsigned short  carAngle;       //当前车身角度
        unsigned short  lastPoint;      //最后一次读到的站点序号
        unsigned char   reserverd[3];   //保留
};

class ReportActionStatus
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  taskId;         //任务编号
        unsigned char   status;         //任务状态
                                        //0x01：叉起
                                        //0x02：放下
        unsigned char   reserverd[3];   //保留

};

class RequestPath
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  lastPoint;      //上个站点序号
        unsigned int    xPos;           //当前所在X坐标
        unsigned int    yPos;           //当前所在Y坐标
};

class AckToServer
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  cmd;            //命令
        unsigned short  result;         //执行结果 0x0000成功；0x0001失败；其他数值则为错误码。
        unsigned short  dataLength;     //附加数据长度
        unsigned char   *data;          //附加数据
};

class ReportTaskStatus
{
    public:
        int packData(unsigned char *data, int maxlen);
    public:
        unsigned short  id;             //AGV ID
        unsigned short  taskId;         //任务ID
        unsigned char   taskStatus;     //任务执行状态 0x01：叉起 ；0x02：放下
        unsigned char   reserved[3];
};

class StationInfo
{
    public:
        unsigned short  id;             //站点ID
        unsigned int    xPos;           //站点的X坐标（单位mm）
        unsigned int    yPos;           //站点的Y坐标（单位mm）
        double          pathAngle;      //行走路径角度
        unsigned short  turnRadius;     //转弯半径，单位mm，直线时为0
        unsigned char   mode;           //运行模式：
                                        //      0x01 前进
                                        //      0x02 后退
                                        //      0x03 左转
                                        //      0x04 右转
        unsigned char   action;         //到站动作：
                                        //      0x00 保持不变
                                        //      0x01 停止
                                        //      0x02 减速
                                        //      0x03 恢复速度
                                        //      0x04 叉臂升降
                                        //      0x05 开启充电
        unsigned short  liftHeight;     //升降器高度，单位mm，为0xFFFF时忽略
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
        unsigned short  id;             //站点ID
        unsigned int    xPos;           //站点的X坐标（单位mm）
        unsigned int    yPos;           //站点的Y坐标（单位mm）
        unsigned char   direction;      //前进方向 0x00 前进    0x01 后退
        unsigned short  turnRadius;     //转弯半径，单位mm，直线时为0
        unsigned short  turnAngle;      //转弯角度, 0-360°
        unsigned char   turnMode;       //转弯方式， 0x00 不转；0x01 左转；0x02 右转
        unsigned char   action;         //到站动作：
                                        //      0x00不停；
                                        //      0x01停站。
                                        //      0x02减速到最小速度。
                                        //      0x03加速到正常速度；
                                        //      0x04 停止后叉货
                                        //      0x05 停止后卸货
                                        //      0x06 停止后开启充电
        unsigned short  liftHeight;     //升降器高度，单位mm，为0xFFFF时忽略
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
        unsigned char   action;         //动作: 0x00运行；0x01停止。
        unsigned char   reserved[3];
};


class SetSpeed
{
    public:
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned short  speed;          //速度值*100，取整数
        unsigned char   reserved[2];
};


class ServerAck
{
    public:
        int unPackData(const unsigned char *data, int len);
    public:
        unsigned short  cmd;            //命令
        unsigned short  result;         //执行结果 0x0000成功；0x0001失败；其他数值则为错误码。
        unsigned short  dataLength;     //附加数据长度
        unsigned char   *data;          //附加数据
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
            ServerSendPath      = 0x0201,   //服务器向AGV发送路径序列
            ServerSendAction    = 0x0202,   //服务器指令AGV执行某个动作
            ServerSetSpeed      = 0x0203,   //服务器设置AGV的运行速度
            ServerResponseAck   = 0x0204,   //服务器对来自AGV的指令执行情况的回复
            ServerChargerTurnOn = 0x0205,
            ServerChargerTurnOff= 0x0206,
            ServerSynchronizeTime = 0x0207,
            ServerSetBarrierDistance = 0x0208,
            ServerSetAgvSleep   = 0x0209,
            ServerSetAgvWakeUp  = 0x0210,
            ServerRunTest       = 0xFFFF,   //行走测试命令

            AgvReportStatus     = 0x0101,   //AGV向服务器报告AGV当前的状态
            AgvRequestStation   = 0x0102,   //AGV请求去往某个卡点
            AgvHeartBeat        = 0x0103,   //AGV心跳包
            AgvCmdAck           = 0x0104,   //AGV对来自服务器的指令执行情况的回复
            AgvReportTaskStatus = 0x0105,   //AGV报告任务状态
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
        unsigned char   head;       //帧头:0xFF，标识帧的开始；
        unsigned short  length;     //长度：表示从[校验码]开始到[数据]结束的字节数（即N+6）；
        unsigned short  checkSum;   //校验码算法：CRC16，自[协议标识]到[数据]的校验值；
        unsigned short  type;       //协议标识：默认为0x0001，用于标识不同类型的协议；
        unsigned short  cmd;        //命令
        unsigned char   *data;      //附带数据内容
        unsigned short  dataLength; //附带数据长度
        unsigned char   tail;       //帧尾：0xFF，标识帧的结束；
    private:
        unsigned char buffer[2048];
};

#endif /* AGVSCHPROTOCOL_H_ */
