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
 * AGV底盘控制类,实现对底盘的基本控制
 */
class CAgvBaseControler
{
    public:
        /**
         * 底盘驱动控制类型
         */
        enum BaseControlType
        {
            SteeringDrive = 0,  //!< 舵轮控制
            DiffDrive,          //!< 两轮差速控制
        };

        CAgvBaseControler(int type);

        virtual ~CAgvBaseControler();

        /**
         * 初始化
         * @return
         */
        virtual int Init() = 0;

        /**
         * 前进
         * @param speed 前进速度，单位：mm/s
         * @return
         */
        virtual int RunForward(int speed) = 0;

        /**
         * 后退
         * @param speed 后退的速度，单位：mm/s
         * @return
         */
        virtual int RollBack(int speed) = 0;

        /**
         * 左转
         * @param speed 转动的速度，单位：mm/s
         * @return
         */
        virtual int TurnLeft(int speed) = 0;

        /**
         * 右转
         * @param speed 转动的速度，单位：mm/s
         * @return
         */
        virtual int TurnRight(int speed) = 0;

        /**
         * 往左旋转
         * @param speed 转动的速度，单位：mm/s
         * @return
         */
        virtual int RotateLeft(int speed) = 0;

        /**
         * 往右旋转
         * @param speed 转动的速度，单位：mm/s
         * @return
         */
        virtual int RotateRight(int speed) = 0;

        /**
         * 设置底盘速度
         * @param linearSpeed 线速度，单位：mm/s
         * @param angularSpeed 角速度，单位：rad/s
         * @return
         */
        virtual int SetCarSpeed(int linearSpeed, float angularSpeed) = 0;

        /**
         * 停车
         * @param enbrake 是否开启抱闸
         * @return
         */
        virtual int StopCar(bool enbrake) = 0;


        /**
         * 设置使能
         * @param enable 是否开启
         * @return
         */
        virtual int SetEnable(bool enable) = 0;

        /**
         * 查询错误
         * @return
         */
        virtual int CheckError(void) = 0;

        /**
         * 查询当前的线速度
         * @return
         */
        virtual int CheckSpeed(void) = 0;

        /**
         * 设置加速度
         * @param accSpeed 加速度值
         * @return
         */
        virtual int SetAccSpeed(int accSpeed) = 0;

        /**
         * 设置减速度
         * @param decSpeed 减速度值
         * @return
         */
        virtual int SetDecSpeed(int decSpeed) = 0;

        /**
         * 设置刹车是否打开
         * @param enable 是否打开, true：打开(电机停转）；false：关闭（电机可以转动）
         * @return
         */
        virtual int SetBreak(bool enable) = 0;

        virtual int GetCAN_ID(void) = 0;


   protected:
        CAgvControler           *controler;
        CAgvSetting             * setting ;

    public:
        int     driveType;          //!< 驱动类型
        int     wheelRadius;        //!< 轮子的半径
        int     wheelBase;          //!< 底盘的轴距
        int     wheelSeperation;    //!< 驱动轮距
        float     gearRatio;          //!< 减速比
        int     targetLineSpeed;    //!< 设置的目标线速度
        float   targetAngulaSpeed;  //!< 设置的目标角速度
        int     currentLineSpeed;   //!< 当前的线速度
        float   currentAngularSpeed; //!< 当前的角速度
        int     currentAccSpeed;    //!< 当前的加速度
        int     currentDecSpeed;    //!< 当前的减速度
        int     currentDirection;   //!< 当前的方向，0：停止，1：前进，2：后退
        int     errorNum;           //!< 错误号


};

#endif /* AGVBASECONTROLER_H_ */
