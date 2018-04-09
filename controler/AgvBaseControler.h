/*
 * AgvBaseControler.h
 *
 *  Created on: 2017-6-19
 *      Author: zxq
 */

#ifndef AGVBASECONTROLER_H_
#define AGVBASECONTROLER_H_

#include "../devices/canbus/AgvSensor.h"

/**
 * AGV���̿�����,ʵ�ֶԵ��̵Ļ�������
 */
class CAgvBaseControler
{
    public:
        /**
         * ����������������
         */
        enum BaseControlType
        {
            SteeringDrive = 0,  //!< ���ֿ���
            DiffDrive,          //!< ���ֲ��ٿ���
        };

        CAgvBaseControler(int type);

        virtual ~CAgvBaseControler();

        /**
         * ��ʼ��
         * @return
         */
        virtual int Init() = 0;

        /**
         * ǰ��
         * @param speed ǰ���ٶȣ���λ��mm/s
         * @return
         */
        virtual int RunForward(int speed) = 0;

        /**
         * ����
         * @param speed ���˵��ٶȣ���λ��mm/s
         * @return
         */
        virtual int RollBack(int speed) = 0;

        /**
         * ��ת
         * @param speed ת�����ٶȣ���λ��mm/s
         * @return
         */
        virtual int TurnLeft(int speed) = 0;

        /**
         * ��ת
         * @param speed ת�����ٶȣ���λ��mm/s
         * @return
         */
        virtual int TurnRight(int speed) = 0;

        /**
         * ������ת
         * @param speed ת�����ٶȣ���λ��mm/s
         * @return
         */
        virtual int RotateLeft(int speed) = 0;

        /**
         * ������ת
         * @param speed ת�����ٶȣ���λ��mm/s
         * @return
         */
        virtual int RotateRight(int speed) = 0;

        /**
         * ���õ����ٶ�
         * @param linearSpeed ���ٶȣ���λ��mm/s
         * @param angularSpeed ���ٶȣ���λ��rad/s
         * @return
         */
        virtual int SetCarSpeed(int linearSpeed, float angularSpeed) = 0;

        /**
         * ͣ��
         * @param enbrake �Ƿ�����բ
         * @return
         */
        virtual int StopCar(bool enbrake) = 0;


        /**
         * ����ʹ��
         * @param enable �Ƿ���
         * @return
         */
        virtual int SetEnable(bool enable) = 0;

        /**
         * ��ѯ����
         * @return
         */
        virtual int CheckError(void) = 0;

        /**
         * ��ѯ��ǰ�����ٶ�
         * @return
         */
        virtual int CheckSpeed(void) = 0;

        /**
         * ���ü��ٶ�
         * @param accSpeed ���ٶ�ֵ
         * @return
         */
        virtual int SetAccSpeed(int accSpeed) = 0;

        /**
         * ���ü��ٶ�
         * @param decSpeed ���ٶ�ֵ
         * @return
         */
        virtual int SetDecSpeed(int decSpeed) = 0;

        /**
         * ����ɲ���Ƿ��
         * @param enable �Ƿ��, true����(���ͣת����false���رգ��������ת����
         * @return
         */
        virtual int SetBreak(bool enable) = 0;

        virtual int GetCAN_ID(void) = 0;


   protected:
        CAgvControler           *controler;
        CAgvSetting             * setting ;

    public:
        int     driveType;          //!< ��������
        int     wheelRadius;        //!< ���ӵİ뾶
        int     wheelBase;          //!< ���̵����
        int     wheelSeperation;    //!< �����־�
        float     gearRatio;          //!< ���ٱ�
        int     targetLineSpeed;    //!< ���õ�Ŀ�����ٶ�
        float   targetAngulaSpeed;  //!< ���õ�Ŀ����ٶ�
        int     currentLineSpeed;   //!< ��ǰ�����ٶ�
        float   currentAngularSpeed; //!< ��ǰ�Ľ��ٶ�
        int     currentAccSpeed;    //!< ��ǰ�ļ��ٶ�
        int     currentDecSpeed;    //!< ��ǰ�ļ��ٶ�
        int     currentDirection;   //!< ��ǰ�ķ���0��ֹͣ��1��ǰ����2������
        int     errorNum;           //!< �����


};

#endif /* AGVBASECONTROLER_H_ */
