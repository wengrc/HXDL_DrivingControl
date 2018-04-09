/*
 * AgvCarRunControl.h
 *
 *  Created on: 2017-5-4
 *      Author: zxq
 */

#ifndef AGVCARRUNCONTROL_H_
#define AGVCARRUNCONTROL_H_


#include "../AgvPublic.h"
#include "AgvBaseControler.h"
#include <string>

using namespace std;
typedef struct StRunContext
{
    int     station;
    int     mode;
    int     xPos;
    int     yPos;
    float   angle;
    int     radius;
    int     speed;
    int     action;
    bool    needStop;
    int     liftHeight;
}RunContext;

/**
 * AGV �ƶ�������
 */
class CAgvMoveHelper
{
    public:
        /**
         * ���캯��
         * @param controler  ���̿��������ָ��
         */
        CAgvMoveHelper(const char *name, CAgvBaseControler *controler);

        /**
         * ����
         */
        virtual ~CAgvMoveHelper();

        /**
         * ��ʼ����
         * @return
         */
        int Start();

        /**
         * ֹͣ����
         * @return
         */
        int Stop();

        /**
         * �ڿ�������ѭ�����õĺ�����Լ10ms����һ��
         * @return
         */
        virtual int LoopRunning() = 0;

        /**
         * �����ƶ���Ŀ�ĵ�״̬�Ͷ�����Ϣ
         * @param station       վ���
         * @param mode          �н���ģʽ
         * @param speed         �н����ٶ�
         * @param xpos          ��վ֮���X����
         * @param ypos          ��վ֮���Y����
         * @param angle         ��վ�ĳ���Ƕ�
         * @param radius        ת��İ뾶
         * @param action        ��վ֮��Ķ���
         * @param liftheight    ��֮��Ĳ�۵ĸ߶�
         * @return
         */
        virtual int SetTarget(int station, int mode, int speed, int xpos, int ypos, float angle, int radius, int action, int liftheight) = 0;

        /**
         *���Ŀ�ĵ���Ϣ
         * @return
         */
        virtual int ClearTarget() = 0;

        virtual int GetRunningMode() = 0;

        virtual int SetBarrierStatus(int status) = 0;

        /**
         *�������е�״̬
         * @param run �Ƿ�������
         */
        inline void SetRunning(bool run) {isRunning = run;}

        /**
         *����Ƿ���������
         * @return
         */
        bool IsRunning()    {return isRunning;}

        inline const string &Name() {return name;}



    protected:
        string              name;
        CAgvBaseControler   *controler;
        CAgvControler       *s_Controler;
        CAgvLidarSensor     *s_Lidar;
        RunContext          startPoint;
        RunContext          targetPoint;

        bool                isRunning;
        int                 distance;
        bool                isFinished;
        bool                isStopAfterFinish;
        int                 runningMode;
        bool                needMoveLift;
        int                 liftHeight;
        int                 CarRunningMode;
};


#endif /* AGVCARRUNCONTROL_H_ */
