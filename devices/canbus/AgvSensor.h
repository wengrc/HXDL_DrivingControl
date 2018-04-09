/*
 * AgvSensor.h
 *
 *  Created on: 2016-11-17
 *      Author: zxq
 */

#ifndef AGVSENSOR_H_
#define AGVSENSOR_H_



#include <string.h>

#include "AgvCanOperator.h"

using namespace std;

/**
 * AGV传感器接口基类
 */
class CAgvSensor
{
    public:
        /**
         * 构造函数
         * @param name 传感器名称
         */
        CAgvSensor(const char *name) {sensorName = name;}

        /**
         * 析构
         */
        virtual ~CAgvSensor(){}

        /**
         * 传感器初始化
         * @return 0：正常
         */
        virtual int Init() = 0;

        /**
         * 发送数据到传感器
         * @param buf 数据指针
         * @param len 数据长度
         * @return
         *  -   <0: 发送失败
         *  -   >0: 实际发送的数据长度
         */
        virtual int SendData(const unsigned char *buf, unsigned int len) = 0;

        /**
         * 解析接收到的数据
         * @param buf 接收到的数据指针
         * @param len 接收到的数据长度
         * @return
         *    - 0： 解析成功
         *    - <0: 解析失败
         */
        virtual int ParseData(const unsigned char *buf, unsigned int len) = 0;

        /**
         * 关闭传感器设备
         * @return
         *      - 0：成功
         *      - <0: 失败
         */
        virtual int Close() = 0;

        /**
         * 获取传感器名称
         * @return 传感器名称
         */
        inline const char *Name() {return sensorName.c_str();}
    protected:
        string      sensorName;
};


/**
 * 使用CAN总线的传感器基类 @relatedalso CAagSensor
 */
class CAgvCanSensor : public CAgvSensor
{
    public:
        /**
         * 构造函数
         * @param rid  传感器的上传通道CANID
         * @param wid  下发给传感器的通道CANID
         * @param moduleid 传感器所处的模块编号
         * @param name 传感器名称
         */
        CAgvCanSensor(int rid, int wid, int moduleid, const char *name);

        /**
         * 析构函数
         */
        virtual ~CAgvCanSensor();


        /**
         * 初始化CAN传感器的虚函数，其子类可以重写
         * @param op CAN操作接口对象指针，数据的收发均是通过调用它的接口来实现
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        virtual int Init(CAgvCanOperator *op);

        /**
         * 关闭CAN传感器，子类可以重写
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        virtual int Close();



        /**
         * 实现父类的初始化函数
         * @return
         */
        int Init() {return 0;}

        /**
         * 实现父类的发送数据接口
         * @param buf 要发送的数据指针
         * @param len 要发送是数据长度
         * @return
         *  -   <0: 发送失败
         *  -   >0: 实际发送的数据长度
         */
        int SendData(const unsigned char *buf, unsigned int len);

        /**
         * 实现父类的解析数据函数
         * @param buf  要解析的数据指针
         * @param len  要解析的数据长度
         * @return
         *    - 0： 解析成功
         *    - <0: 解析失败
         */
        int ParseData(const unsigned char *buf, unsigned int len);


        /**
         * 设置CAN操作接口指针.@note 该操作必须在发送和接收数据之前进行！
         * @param op CAN读写操作对象指针
         */
        inline void SetCanOperator(CAgvCanOperator *op) {canOperator = op;}

        /**
         * 用于按CANID来排序的对比操作符
         *
         * @param t 要对比的对象
         * @return 是否比它小
         */
        bool operator<(const CAgvCanSensor &t) const
        {
            if(this->rCanId < t.rCanId)
                return true;
            return false;
        }

    protected:
        /**
         * 解析CAN传感器设备上传的数据，子类需要去实现该虚函数
         * @param buf 要解析的数据指针
         * @param len 要解析的数据长度
         * @return
         *  -   <0:  解析失败
         *  -   >0:  解析完后有效数据的长度
         */
        virtual int ParseCanData(const unsigned char *buf, unsigned int len);


    private:
        /**
         * 往传感器发送数据
         * @param canid 要发送的CAN通道ID
         * @param buf   要发送的数据指针
         * @param len   要发送的数据长度
         * @return
         *  -   <0: 发送失败
         *  -   >0: 实际发送的数据长度
         */
        int sendCanData(int canid, const unsigned char *buf, unsigned int len);

    public:
        unsigned int        rCanId;
        unsigned int        wCanId;
        int                 moduleId;
        int                 heartBeatCounter;
        bool                isAlive;
    protected:
        CAgvCanOperator     *canOperator;

};

/**
 * 激光雷达传感器，以150ms一次的频率上传当前的坐标和角度信息，
 */
class CAgvLidarSensor : public CAgvCanSensor
{
    public:
        CAgvLidarSensor();
        ~CAgvLidarSensor();

        static CAgvLidarSensor *Instance();

        /**
         * 激光雷达是否已就绪
         * @return 是否
         */
        bool IsDataReady();

        /**
         * 传感器是否有故障
         * @return 是否
         */
        bool IsHaveError();

        /**
         * 导航器的错误信息
         */
        enum
        {
            NoError = 0,
            WrongOperatingMode,
            AsynchronyMethodTerminated,
            InvalidData,
            NoPositionAvailable,
            Timeout,
            MethodAlreadyActive,
            GeneralError
        };

        /**
         * 导航器工作模式
         */
        enum
        {
            InitialPositioning = 0,
            ContinuousPositioning,
            VirtualPositioning,
            PositioningStopped,
            PositionInvalid,
            External
        };

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int         xPos;   //!< X坐标,单位mm
        int         yPos;   //!< Y坐标，单位mm
        float       angle;  //!< 相对于车身方向的角度，2位小数
        int         mode;   //!< 工作模式
        int         errorCode;  //!< 错误码
    private:
        CAgvSetting             *setting;
        int                      NavigatorOffset;
        int                      LIDAR_LEFT_RIGHT_OFFSET_MM;
        float                    ANGLE_OFFSET;
        static CAgvLidarSensor  *instance;
};


/**
 * 陀螺仪传感器，能够上传三轴方向的加速度和角度
 */
class CAgvGyroscopeSensor : public CAgvCanSensor
{
    public:
        CAgvGyroscopeSensor();
        ~CAgvGyroscopeSensor();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int         xAcceleration;  //!< X轴方向加速度,单位毫米/秒
        int         yAcceleration;  //!< Y轴方向加速度,单位毫米/秒
        int         zAcceleration;  //!< Z轴方向加速度,单位毫米/秒
        float       courseAngle;    //!< 航向角,2位小数
        float       pitchAngle;     //!< 俯仰角,2位小数
        float       heelAngle;      //!< 倾侧角,2位小数
};

/**
 * 行车控制器，用于操作AGV的行走，能够实时上传当前的方向，速度，舵轮角度的状态
 */
class CAgvControler : public CAgvCanSensor
{
    public:
        CAgvControler();
        ~CAgvControler();
        static CAgvControler *Instance();
        /**
         * 开启/关闭使能
         * @param enable 开启/关闭
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetEnable(bool enable);

        /**
         * 设置行走方向
         * @param direc 0：前进;1：后退
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetDirection(int direc);

        /**
         * 设置舵轮的角度
         * @param angle 范围为±90°，2位小数精度
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetAngle(float angle);

        /**
         * 设置行走速度，
         * @param speed 单位为 mm/sec
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetSpeed(int speed);

        /**
         * 开启/关闭刹车
         * @param enable 开启/关闭
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetBreak(bool enable);

        /**
         * 重置行走距离
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int ResetDistance();


        /**
         * 控制器是否有错误
         * @return 是否
         */
        inline bool IsHaveError() {return (runErrorCode || turnErrorCode);}

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int             direction;      //!< 行进方向，0：停止，1：前进，2：后退
        int             speed;          //!< 行进速度，单位毫米/秒
        float           angle;          //!< 行进的角度
        unsigned int    distance;       //!< 行进的距离，单位毫米
        unsigned char   runErrorCode;   //!< 行走错误码
        unsigned char   turnErrorCode;  //!< 转向错误码

    private:
        static CAgvControler *instance;
};

/**
 * 升降器，实现叉车或者AGV上的升降器的控制，同时会上传起降器的位置以及上下限开关的状态
 */
class CAgvLifter : public CAgvCanSensor
{
    public:
        CAgvLifter();
        ~CAgvLifter();

        /**
         * 升降器的位置状态
         */
        enum LiftStatus
        {
            UnKnown = -1,   //!< 初始未知
            Moving,         //!< 升降中
            AtBottom,       //!< 在最底部
            AtTop,          //!< 在最顶部
        };

        int SetTargetHeight(int height);

        /**
         *升起升降器
         * @param speed     上升速度，单位为mm/sec
         * @param height    上升高度，单位为mm
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetLiftUp(int speed, int height);

        /**
         *降下升降器
         * @param speed     下降速度，单位为mm/sec
         * @param height    下降高度，单位为mm
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetLiftDown(int speed, int height);

        /**
         * 升降器是否到达指定高度
         * @param height 指定高度，单位mm
         * @return 是否
         */
        bool LiftIsArrived(int height);

        int  GetLiftStatus();


        /**
         *停止升降
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int StopLift();


        int InitLift();//2018-03-26
        /**
         *升降器是否到达顶部
         * @return 是/否
         */
        bool LiftIsAtTop();

        /**
         *升降器是否到达底部
         * @return 是/否
         */
        bool LiftIsAtBottom();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int         liftStatus;         //!< 上升状态
        int         currentHeight;      //!< 当前的高度
        int         targetHeight;       //!< 指定的升降高度
};


/**
 * 障碍检测传感器，能够上传远/中/近/触碰等障碍的状态，有报警状态将会发送相应的消息
 */
class CAgvBarrierSensor : public CAgvCanSensor
{

    public:
        /**
         * 传感器类型
         */
        enum SensorType
        {
            IrSensor = 1,   //!< 红外
            Laser,          //!< 激光
            Ultrasonic,     //!< 超声波
            Touch,          //!< 触须/接近开关
            CrashBarrier,   //!< 防撞条
            SensorTypeMax,
        };
        /**
         * 传感器状态
         */
        enum BarrierStatus
        {
            None = 0,       //!< 无障碍
            Far,            //!< 远距离障碍
            Middle,         //!< 中距离障碍
            Near,           //!< 近距离障碍
            Touched,        //!< 已碰触
        };

        /**
         * 传感器位置
         */
        enum BarrierPos
        {
            Unkown = 0,     //!< 未知
            Front  = 1,     //!< 前面
            Back   = 2,     //!< 后面
            Left   = 4,     //!< 左侧
            Right  = 8,     //!< 右侧
        };

        CAgvBarrierSensor();
        ~CAgvBarrierSensor();

        /**
         * 设置传感器的位置
         * @param type  传感器类型
         * @param id    传感器编号
         * @param pos   传感器位置
         */
        void SetSensorPos(int type, int id, int pos);

        /**
         * 获取传感器的位置
         * @param type  传感器类型
         * @param id    传感器编号
         * @return 返回传感器位置
         */
        int GetSensorPos(int type, int id);

        /**
         * 设置障碍传感器的障碍距离
         * @note 目前只有超声波、激光传感器支持
         *
         * @param type      传感器类型
         * @param id        传感器编号
         * @param status    要设置的障碍状态
         * @param distance  要设置的对应的障碍距离
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetBarrierDistance(int type, int id, int status,int distance);

        /**
         * 复位传感器的障碍状态
         * @note 目前针对防撞条，触碰之后需要发送命令来复位状态
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int ResetBarrierError();

        /**
         * 获取所有传感器中最严重的障碍状态
         * @param side 传感器位置， 为0时表示所有方向
         * @return 障碍状态值
         */
        int GetBarrierError(int side);

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        map<int, int> barrierStatus;        //!< 各个传感器的的状态
        map<int, int> barrierPosMap;        //!< 各个传感器的位置列表
};


/**
 * 货物检测传感器
 */
class CAgvCargoSensor : public CAgvCanSensor
{
    public:
        CAgvCargoSensor();
        ~CAgvCargoSensor();

        /**
         * 是否有货物
         * @return true or false
         */
        bool IsHaveCargo();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     isHaveCargo;
};

/**
 * RGB三色灯传感器
 */
class CAgvRgbLight : public CAgvCanSensor
{
    public:
        /**
         * 哪一侧
         */
        enum LightSide
        {
            Left = 1,       //!< 左侧
            Right = 2,      //!< 右侧
            BothSide = 3    //!< 两边
        };

        /**
         * 亮灯的模式
         */
        enum LightMode
        {
            Off = 0,        //!< 不亮
            On,             //!< 常亮
            FlashNormal,    //!< 闪烁
            FlashBusy,      //!< 快闪
        };

        CAgvRgbLight();
        ~CAgvRgbLight();

        /**
         * 控制灯的状态
          *
         * @param side  哪一侧
         * @param red       红灯的模式
         * @param green     绿灯的模式
         * @param blue      蓝灯的模式
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetLight(int side, int red, int green, int blue);
};

/**
 * 按键传感器，上传各按键的状态，按键状态变化时将会以消息的方式发送
 * @note 各按键状态500ms上传一次，有变化时立即上传
 */
class CAgvButtonSensor : public CAgvCanSensor
{
    public:
        /**
         * 按键
         */
        enum Key
        {
            UrgentStopKey = 0,//!< 紧急暂停按钮
            StartOrStopKey,   //!< 运行/停止按钮
            AutoOrManualKey,  //!< 自动/手动模式按钮
            LiftUpOrDownKey,  //!< 升降按钮
            KeyMax            //!< 按钮数量
        };
        /**
         * 按键状态
         */
        enum KeyStatus
        {
            Off = 0,        //!< 关
            On,             //!< 开
        };

        CAgvButtonSensor();
        ~CAgvButtonSensor();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        unsigned char   keyStatus[KeyMax];   //!< 各个按钮的状态
};


class CAgvHMISensor : public CAgvCanSensor
{
    public:
        int ShowBatteryCapacity(int Capacity);
        int ShowNetState(int State);
        int ShowCarState(int State);
        int WriteHMIPara();

        CAgvHMISensor();
        ~CAgvHMISensor();
    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);
    private:
        CAgvSetting             *setting;
};


/**
 * 磁传感器，会上传各个磁单元的磁感应效果
 */
class CAgvMagSensor : public CAgvCanSensor
{
    public:
        /**
         *磁传感器数据模式
         */
        enum Mode
        {
            NormalBitMode = 0,  //!< 常规16位数据模式
            DoubleWidthMode,    //!< 倍宽32位数据模式
            DetailDataMode      //!< Pos+Value模式
        };

        CAgvMagSensor(int id, int mode = NormalBitMode);
        ~CAgvMagSensor();

        /**
         * 设置磁传感器的磁通量最小值
         * @param value 磁通量值，0-255
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetMagShreshold(int value);

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     id;
        int     mode;
        int     magValue;
        float   middlePoint;
        unsigned char magValues[32];
        unsigned char magFullValues[32];
};

/**
 * 磁钉传感器，上传两个磁钉间的夹角
 */
class CAgvMagNailSensor : public CAgvCanSensor
{
    public:

        CAgvMagNailSensor(int id);
        ~CAgvMagNailSensor();

        /**
         * 手动发命令获取上一组磁钉的角度，结果存放在变量中
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int GetLastNailAngle();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     id;             //!< 0：前面，1：后面
        float   nailAngle;      //!< 两个磁钉的夹角
};

/**
 * RFID读卡器，上传读到的RFID卡号
 */
class CAgvRfidReader : public CAgvCanSensor
{
    public:
        CAgvRfidReader();
        ~CAgvRfidReader();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     cardType;       //!< 站点卡类型
        int     cardNumber;     //!< 卡号
};

/**
 * 语音播放器
 */
class CAgvVoicePlayer : public CAgvCanSensor
{
    public:

        /**
         * 语音id
         */
        enum Voices
        {
            MP3_MUTE = 0,      //!< 静音
            MP3_WELCOM = 1,    //!< 欢迎使用
            MP3_RUN = 2,       //!< 开始运行
            MP3_NORMAL = 3,    //!< 到达站点
            MP3_BATTERYLOW = 4,//!< 电池电量低
            MP3_NEAR = 5,      //!< 近距离障碍
            MP3_FAR = 6,       //!< 远距离障碍
            MP3_MID = 7,       //!< 中距离障碍
            MP3_URGENT = 8,    //!< 紧急暂停
            MP3_OUTPATH = 9,   //!< 脱轨
            MP3_UPGRADE = 11,  //!<AGV升级
            MP3_NETWORK = 12,  //!< 网络错误
            MP3_WIFI_DEV_FAULT = 13, //!<无线模块故障
            MP3_DRIVER_FAULT = 14,   //!<驱动模块故障
            MP3_TURN = 15,      //!< 转弯
            MP3_INSERT = 16,    //!< 正在叉货
            MP3_CONTROLLER_FAULT = 17, //!<控制器故障：（控制器故障）
            MP3_NAVIGATOR_FAULT  = 18, //!<导航器故障：（导航器故障）
            MP3_EXAMINE = 19,   //!< 自检错误
            MP3_BARRIER = 20,    //!<碰撞条障碍：（AGV发生碰撞，紧急停止）
            MP3_BOTTOM = 21,    //!< 升降器到底部
            MP3_TOP = 22,       //!< 升降器到顶部
            MP3_BACKING_AGV = 23, //!<倒车：（倒车请注意）
            MP3_TOUCH = 24,
            MP3_MANUALCHAGERON = 25,
            MP3_MANUALCHAGEROFF = 26,
            MP3_PATHMAGFAULT = 27,
        };

        CAgvVoicePlayer();
        ~CAgvVoicePlayer();

        /**
         * 设置音量
         * @param volume 音量值 0 - 100
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int SetVolume(int volume);

        /**
         * 播放语音
         * @param id  语音编号
         * @param times  播放次数
         * @return
         *  -   0： 成功
         *  -   <0: 失败
         */
        int PlayVoice(int id, int times);
};

/**
 * 电池状态传感器，上传电池的状态
 */
class CAgvPowerSensor : public CAgvCanSensor
{
    public:
        enum BatteryType
        {
            None = 0,           //!< 未定义
            Lead,               //!< 铅酸电池
            Lithium,            //!< 锂电池
        };

        enum PowerLevel
        {
            Full,               //!< 满电量
            Hight,              //!< 电量充足
            Middle,             //!< 电量中
            Low,                //!< 低电量
            Empty,              //!< 电量过低
        };

        enum switchStuta
        {
            Off = 0,        //!< 关
            On,             //!< 开
        };

        CAgvPowerSensor(int type);

        ~CAgvPowerSensor();

        void SetBatteryType(int type);

        int SetChargeSwitch(bool enable);

        int SetAgvSleepSwitch(bool enable);

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    private:
        int calcCapacity(float volt);

    public:
        int     batteryType;    //!< 电池类型
        int     minVoltage;     //!< 最低电压
        int     maxVoltage;     //!< 最高电压

        int     voltage;        //!< 当前电压
        int     capacity;       //!< 电量
        int     electricity;    //!< 当前electricity
        int     level;          //!< 等级
};


#endif /* AGVSENSOR_H_ */
