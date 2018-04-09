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
 * AGV�������ӿڻ���
 */
class CAgvSensor
{
    public:
        /**
         * ���캯��
         * @param name ����������
         */
        CAgvSensor(const char *name) {sensorName = name;}

        /**
         * ����
         */
        virtual ~CAgvSensor(){}

        /**
         * ��������ʼ��
         * @return 0������
         */
        virtual int Init() = 0;

        /**
         * �������ݵ�������
         * @param buf ����ָ��
         * @param len ���ݳ���
         * @return
         *  -   <0: ����ʧ��
         *  -   >0: ʵ�ʷ��͵����ݳ���
         */
        virtual int SendData(const unsigned char *buf, unsigned int len) = 0;

        /**
         * �������յ�������
         * @param buf ���յ�������ָ��
         * @param len ���յ������ݳ���
         * @return
         *    - 0�� �����ɹ�
         *    - <0: ����ʧ��
         */
        virtual int ParseData(const unsigned char *buf, unsigned int len) = 0;

        /**
         * �رմ������豸
         * @return
         *      - 0���ɹ�
         *      - <0: ʧ��
         */
        virtual int Close() = 0;

        /**
         * ��ȡ����������
         * @return ����������
         */
        inline const char *Name() {return sensorName.c_str();}
    protected:
        string      sensorName;
};


/**
 * ʹ��CAN���ߵĴ��������� @relatedalso CAagSensor
 */
class CAgvCanSensor : public CAgvSensor
{
    public:
        /**
         * ���캯��
         * @param rid  ���������ϴ�ͨ��CANID
         * @param wid  �·�����������ͨ��CANID
         * @param moduleid ������������ģ����
         * @param name ����������
         */
        CAgvCanSensor(int rid, int wid, int moduleid, const char *name);

        /**
         * ��������
         */
        virtual ~CAgvCanSensor();


        /**
         * ��ʼ��CAN���������麯���������������д
         * @param op CAN�����ӿڶ���ָ�룬���ݵ��շ�����ͨ���������Ľӿ���ʵ��
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        virtual int Init(CAgvCanOperator *op);

        /**
         * �ر�CAN�����������������д
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        virtual int Close();



        /**
         * ʵ�ָ���ĳ�ʼ������
         * @return
         */
        int Init() {return 0;}

        /**
         * ʵ�ָ���ķ������ݽӿ�
         * @param buf Ҫ���͵�����ָ��
         * @param len Ҫ���������ݳ���
         * @return
         *  -   <0: ����ʧ��
         *  -   >0: ʵ�ʷ��͵����ݳ���
         */
        int SendData(const unsigned char *buf, unsigned int len);

        /**
         * ʵ�ָ���Ľ������ݺ���
         * @param buf  Ҫ����������ָ��
         * @param len  Ҫ���������ݳ���
         * @return
         *    - 0�� �����ɹ�
         *    - <0: ����ʧ��
         */
        int ParseData(const unsigned char *buf, unsigned int len);


        /**
         * ����CAN�����ӿ�ָ��.@note �ò��������ڷ��ͺͽ�������֮ǰ���У�
         * @param op CAN��д��������ָ��
         */
        inline void SetCanOperator(CAgvCanOperator *op) {canOperator = op;}

        /**
         * ���ڰ�CANID������ĶԱȲ�����
         *
         * @param t Ҫ�ԱȵĶ���
         * @return �Ƿ����С
         */
        bool operator<(const CAgvCanSensor &t) const
        {
            if(this->rCanId < t.rCanId)
                return true;
            return false;
        }

    protected:
        /**
         * ����CAN�������豸�ϴ������ݣ�������Ҫȥʵ�ָ��麯��
         * @param buf Ҫ����������ָ��
         * @param len Ҫ���������ݳ���
         * @return
         *  -   <0:  ����ʧ��
         *  -   >0:  ���������Ч���ݵĳ���
         */
        virtual int ParseCanData(const unsigned char *buf, unsigned int len);


    private:
        /**
         * ����������������
         * @param canid Ҫ���͵�CANͨ��ID
         * @param buf   Ҫ���͵�����ָ��
         * @param len   Ҫ���͵����ݳ���
         * @return
         *  -   <0: ����ʧ��
         *  -   >0: ʵ�ʷ��͵����ݳ���
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
 * �����״ﴫ��������150msһ�ε�Ƶ���ϴ���ǰ������ͽǶ���Ϣ��
 */
class CAgvLidarSensor : public CAgvCanSensor
{
    public:
        CAgvLidarSensor();
        ~CAgvLidarSensor();

        static CAgvLidarSensor *Instance();

        /**
         * �����״��Ƿ��Ѿ���
         * @return �Ƿ�
         */
        bool IsDataReady();

        /**
         * �������Ƿ��й���
         * @return �Ƿ�
         */
        bool IsHaveError();

        /**
         * �������Ĵ�����Ϣ
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
         * ����������ģʽ
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
        int         xPos;   //!< X����,��λmm
        int         yPos;   //!< Y���꣬��λmm
        float       angle;  //!< ����ڳ�����ĽǶȣ�2λС��
        int         mode;   //!< ����ģʽ
        int         errorCode;  //!< ������
    private:
        CAgvSetting             *setting;
        int                      NavigatorOffset;
        int                      LIDAR_LEFT_RIGHT_OFFSET_MM;
        float                    ANGLE_OFFSET;
        static CAgvLidarSensor  *instance;
};


/**
 * �����Ǵ��������ܹ��ϴ����᷽��ļ��ٶȺͽǶ�
 */
class CAgvGyroscopeSensor : public CAgvCanSensor
{
    public:
        CAgvGyroscopeSensor();
        ~CAgvGyroscopeSensor();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int         xAcceleration;  //!< X�᷽����ٶ�,��λ����/��
        int         yAcceleration;  //!< Y�᷽����ٶ�,��λ����/��
        int         zAcceleration;  //!< Z�᷽����ٶ�,��λ����/��
        float       courseAngle;    //!< �����,2λС��
        float       pitchAngle;     //!< ������,2λС��
        float       heelAngle;      //!< ����,2λС��
};

/**
 * �г������������ڲ���AGV�����ߣ��ܹ�ʵʱ�ϴ���ǰ�ķ����ٶȣ����ֽǶȵ�״̬
 */
class CAgvControler : public CAgvCanSensor
{
    public:
        CAgvControler();
        ~CAgvControler();
        static CAgvControler *Instance();
        /**
         * ����/�ر�ʹ��
         * @param enable ����/�ر�
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetEnable(bool enable);

        /**
         * �������߷���
         * @param direc 0��ǰ��;1������
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetDirection(int direc);

        /**
         * ���ö��ֵĽǶ�
         * @param angle ��ΧΪ��90�㣬2λС������
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetAngle(float angle);

        /**
         * ���������ٶȣ�
         * @param speed ��λΪ mm/sec
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetSpeed(int speed);

        /**
         * ����/�ر�ɲ��
         * @param enable ����/�ر�
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetBreak(bool enable);

        /**
         * �������߾���
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int ResetDistance();


        /**
         * �������Ƿ��д���
         * @return �Ƿ�
         */
        inline bool IsHaveError() {return (runErrorCode || turnErrorCode);}

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int             direction;      //!< �н�����0��ֹͣ��1��ǰ����2������
        int             speed;          //!< �н��ٶȣ���λ����/��
        float           angle;          //!< �н��ĽǶ�
        unsigned int    distance;       //!< �н��ľ��룬��λ����
        unsigned char   runErrorCode;   //!< ���ߴ�����
        unsigned char   turnErrorCode;  //!< ת�������

    private:
        static CAgvControler *instance;
};

/**
 * ��������ʵ�ֲ泵����AGV�ϵ��������Ŀ��ƣ�ͬʱ���ϴ�������λ���Լ������޿��ص�״̬
 */
class CAgvLifter : public CAgvCanSensor
{
    public:
        CAgvLifter();
        ~CAgvLifter();

        /**
         * ��������λ��״̬
         */
        enum LiftStatus
        {
            UnKnown = -1,   //!< ��ʼδ֪
            Moving,         //!< ������
            AtBottom,       //!< ����ײ�
            AtTop,          //!< �����
        };

        int SetTargetHeight(int height);

        /**
         *����������
         * @param speed     �����ٶȣ���λΪmm/sec
         * @param height    �����߶ȣ���λΪmm
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetLiftUp(int speed, int height);

        /**
         *����������
         * @param speed     �½��ٶȣ���λΪmm/sec
         * @param height    �½��߶ȣ���λΪmm
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetLiftDown(int speed, int height);

        /**
         * �������Ƿ񵽴�ָ���߶�
         * @param height ָ���߶ȣ���λmm
         * @return �Ƿ�
         */
        bool LiftIsArrived(int height);

        int  GetLiftStatus();


        /**
         *ֹͣ����
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int StopLift();


        int InitLift();//2018-03-26
        /**
         *�������Ƿ񵽴ﶥ��
         * @return ��/��
         */
        bool LiftIsAtTop();

        /**
         *�������Ƿ񵽴�ײ�
         * @return ��/��
         */
        bool LiftIsAtBottom();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int         liftStatus;         //!< ����״̬
        int         currentHeight;      //!< ��ǰ�ĸ߶�
        int         targetHeight;       //!< ָ���������߶�
};


/**
 * �ϰ���⴫�������ܹ��ϴ�Զ/��/��/�������ϰ���״̬���б���״̬���ᷢ����Ӧ����Ϣ
 */
class CAgvBarrierSensor : public CAgvCanSensor
{

    public:
        /**
         * ����������
         */
        enum SensorType
        {
            IrSensor = 1,   //!< ����
            Laser,          //!< ����
            Ultrasonic,     //!< ������
            Touch,          //!< ����/�ӽ�����
            CrashBarrier,   //!< ��ײ��
            SensorTypeMax,
        };
        /**
         * ������״̬
         */
        enum BarrierStatus
        {
            None = 0,       //!< ���ϰ�
            Far,            //!< Զ�����ϰ�
            Middle,         //!< �о����ϰ�
            Near,           //!< �������ϰ�
            Touched,        //!< ������
        };

        /**
         * ������λ��
         */
        enum BarrierPos
        {
            Unkown = 0,     //!< δ֪
            Front  = 1,     //!< ǰ��
            Back   = 2,     //!< ����
            Left   = 4,     //!< ���
            Right  = 8,     //!< �Ҳ�
        };

        CAgvBarrierSensor();
        ~CAgvBarrierSensor();

        /**
         * ���ô�������λ��
         * @param type  ����������
         * @param id    ���������
         * @param pos   ������λ��
         */
        void SetSensorPos(int type, int id, int pos);

        /**
         * ��ȡ��������λ��
         * @param type  ����������
         * @param id    ���������
         * @return ���ش�����λ��
         */
        int GetSensorPos(int type, int id);

        /**
         * �����ϰ����������ϰ�����
         * @note Ŀǰֻ�г����������⴫����֧��
         *
         * @param type      ����������
         * @param id        ���������
         * @param status    Ҫ���õ��ϰ�״̬
         * @param distance  Ҫ���õĶ�Ӧ���ϰ�����
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetBarrierDistance(int type, int id, int status,int distance);

        /**
         * ��λ���������ϰ�״̬
         * @note Ŀǰ��Է�ײ��������֮����Ҫ������������λ״̬
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int ResetBarrierError();

        /**
         * ��ȡ���д������������ص��ϰ�״̬
         * @param side ������λ�ã� Ϊ0ʱ��ʾ���з���
         * @return �ϰ�״ֵ̬
         */
        int GetBarrierError(int side);

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        map<int, int> barrierStatus;        //!< �����������ĵ�״̬
        map<int, int> barrierPosMap;        //!< ������������λ���б�
};


/**
 * �����⴫����
 */
class CAgvCargoSensor : public CAgvCanSensor
{
    public:
        CAgvCargoSensor();
        ~CAgvCargoSensor();

        /**
         * �Ƿ��л���
         * @return true or false
         */
        bool IsHaveCargo();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     isHaveCargo;
};

/**
 * RGB��ɫ�ƴ�����
 */
class CAgvRgbLight : public CAgvCanSensor
{
    public:
        /**
         * ��һ��
         */
        enum LightSide
        {
            Left = 1,       //!< ���
            Right = 2,      //!< �Ҳ�
            BothSide = 3    //!< ����
        };

        /**
         * ���Ƶ�ģʽ
         */
        enum LightMode
        {
            Off = 0,        //!< ����
            On,             //!< ����
            FlashNormal,    //!< ��˸
            FlashBusy,      //!< ����
        };

        CAgvRgbLight();
        ~CAgvRgbLight();

        /**
         * ���ƵƵ�״̬
          *
         * @param side  ��һ��
         * @param red       ��Ƶ�ģʽ
         * @param green     �̵Ƶ�ģʽ
         * @param blue      ���Ƶ�ģʽ
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetLight(int side, int red, int green, int blue);
};

/**
 * �������������ϴ���������״̬������״̬�仯ʱ��������Ϣ�ķ�ʽ����
 * @note ������״̬500ms�ϴ�һ�Σ��б仯ʱ�����ϴ�
 */
class CAgvButtonSensor : public CAgvCanSensor
{
    public:
        /**
         * ����
         */
        enum Key
        {
            UrgentStopKey = 0,//!< ������ͣ��ť
            StartOrStopKey,   //!< ����/ֹͣ��ť
            AutoOrManualKey,  //!< �Զ�/�ֶ�ģʽ��ť
            LiftUpOrDownKey,  //!< ������ť
            KeyMax            //!< ��ť����
        };
        /**
         * ����״̬
         */
        enum KeyStatus
        {
            Off = 0,        //!< ��
            On,             //!< ��
        };

        CAgvButtonSensor();
        ~CAgvButtonSensor();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        unsigned char   keyStatus[KeyMax];   //!< ������ť��״̬
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
 * �Ŵ����������ϴ������ŵ�Ԫ�ĴŸ�ӦЧ��
 */
class CAgvMagSensor : public CAgvCanSensor
{
    public:
        /**
         *�Ŵ���������ģʽ
         */
        enum Mode
        {
            NormalBitMode = 0,  //!< ����16λ����ģʽ
            DoubleWidthMode,    //!< ����32λ����ģʽ
            DetailDataMode      //!< Pos+Valueģʽ
        };

        CAgvMagSensor(int id, int mode = NormalBitMode);
        ~CAgvMagSensor();

        /**
         * ���ôŴ������Ĵ�ͨ����Сֵ
         * @param value ��ͨ��ֵ��0-255
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
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
 * �Ŷ����������ϴ������Ŷ���ļн�
 */
class CAgvMagNailSensor : public CAgvCanSensor
{
    public:

        CAgvMagNailSensor(int id);
        ~CAgvMagNailSensor();

        /**
         * �ֶ��������ȡ��һ��Ŷ��ĽǶȣ��������ڱ�����
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int GetLastNailAngle();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     id;             //!< 0��ǰ�棬1������
        float   nailAngle;      //!< �����Ŷ��ļн�
};

/**
 * RFID���������ϴ�������RFID����
 */
class CAgvRfidReader : public CAgvCanSensor
{
    public:
        CAgvRfidReader();
        ~CAgvRfidReader();

    protected:
        int ParseCanData(const unsigned char *buf, unsigned int len);

    public:
        int     cardType;       //!< վ�㿨����
        int     cardNumber;     //!< ����
};

/**
 * ����������
 */
class CAgvVoicePlayer : public CAgvCanSensor
{
    public:

        /**
         * ����id
         */
        enum Voices
        {
            MP3_MUTE = 0,      //!< ����
            MP3_WELCOM = 1,    //!< ��ӭʹ��
            MP3_RUN = 2,       //!< ��ʼ����
            MP3_NORMAL = 3,    //!< ����վ��
            MP3_BATTERYLOW = 4,//!< ��ص�����
            MP3_NEAR = 5,      //!< �������ϰ�
            MP3_FAR = 6,       //!< Զ�����ϰ�
            MP3_MID = 7,       //!< �о����ϰ�
            MP3_URGENT = 8,    //!< ������ͣ
            MP3_OUTPATH = 9,   //!< �ѹ�
            MP3_UPGRADE = 11,  //!<AGV����
            MP3_NETWORK = 12,  //!< �������
            MP3_WIFI_DEV_FAULT = 13, //!<����ģ�����
            MP3_DRIVER_FAULT = 14,   //!<����ģ�����
            MP3_TURN = 15,      //!< ת��
            MP3_INSERT = 16,    //!< ���ڲ��
            MP3_CONTROLLER_FAULT = 17, //!<���������ϣ������������ϣ�
            MP3_NAVIGATOR_FAULT  = 18, //!<���������ϣ������������ϣ�
            MP3_EXAMINE = 19,   //!< �Լ����
            MP3_BARRIER = 20,    //!<��ײ���ϰ�����AGV������ײ������ֹͣ��
            MP3_BOTTOM = 21,    //!< ���������ײ�
            MP3_TOP = 22,       //!< ������������
            MP3_BACKING_AGV = 23, //!<��������������ע�⣩
            MP3_TOUCH = 24,
            MP3_MANUALCHAGERON = 25,
            MP3_MANUALCHAGEROFF = 26,
            MP3_PATHMAGFAULT = 27,
        };

        CAgvVoicePlayer();
        ~CAgvVoicePlayer();

        /**
         * ��������
         * @param volume ����ֵ 0 - 100
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int SetVolume(int volume);

        /**
         * ��������
         * @param id  �������
         * @param times  ���Ŵ���
         * @return
         *  -   0�� �ɹ�
         *  -   <0: ʧ��
         */
        int PlayVoice(int id, int times);
};

/**
 * ���״̬���������ϴ���ص�״̬
 */
class CAgvPowerSensor : public CAgvCanSensor
{
    public:
        enum BatteryType
        {
            None = 0,           //!< δ����
            Lead,               //!< Ǧ����
            Lithium,            //!< ﮵��
        };

        enum PowerLevel
        {
            Full,               //!< ������
            Hight,              //!< ��������
            Middle,             //!< ������
            Low,                //!< �͵���
            Empty,              //!< ��������
        };

        enum switchStuta
        {
            Off = 0,        //!< ��
            On,             //!< ��
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
        int     batteryType;    //!< �������
        int     minVoltage;     //!< ��͵�ѹ
        int     maxVoltage;     //!< ��ߵ�ѹ

        int     voltage;        //!< ��ǰ��ѹ
        int     capacity;       //!< ����
        int     electricity;    //!< ��ǰelectricity
        int     level;          //!< �ȼ�
};


#endif /* AGVSENSOR_H_ */
