/*
 * AgvRunningHelperAlgoTest.h
 *
 *  Created on: 2017-06-21
 *      Author: cry
 */

#ifndef AGVMOVEHELPERFORKLIFT_H
#define AGVMOVEHELPERFORKLIFT_H

#include "AgvMoveHelper.h"
#include "AgvRoutePlanner.h"
#include "fstream"

#define MIN_ASPEED 15//original:10 //mm/s
#define MIN_DSPEED 15//original:10 //mm/s
#define INIT_SPEED 100 //mm/s
#define MAX_TARGET_SPEED 2000 //mm/s
#define MIN_TARGET_SPEED 100 //mm/sS
#define SPEED_100_MM_PER_SECOND 100
#define BASE_LINEAR_SPEED SPEED_100_MM_PER_SECOND //mm/s, 6m/min
#define TIME_10_SECOND 10
#define TIME_14_SECOND 14



#undef LIDAR_TO_HEAD_LENGTH
#define LIDAR_TO_HEAD_LENGTH 300

#undef MAX_ANGULAR_SPEED_FORWARD
#define MAX_ANGULAR_SPEED_FORWARD 0.087

#define TIME_1000ms 1000
#define TIME_150ms 150
#define SPEED_CHANGE_TIMES 7//6

class CAgvMoveHelperForklift : public CAgvMoveHelper
{
    public:
        typedef enum
        {
            highSpeedMode = 0x01,
            lowSpeedMode  = 0x02,
        }runMode;

        typedef enum
        {
            relative = 0x01,
            absolute = 0x02,
        }ROTATEMODE;

        typedef enum
        {
            left,
            right,
        }ROTATE_DIRECTION;

        typedef struct
        {
            POSE target;
            int runmode;
            int distance;
            int speed;
        } SUBPATH;

        CAgvMoveHelperForklift(CAgvBaseControler *ctrler);

        virtual ~CAgvMoveHelperForklift();

        int Start();

        int Stop();

        int GetRunningMode();

        int SetBarrierStatus(int status);

        int LoopRunning();

        int SetTarget(int station, int mode, int speed, int xpos, int ypos, float angle, int radius, int action, int liftheight);

        int ClearTarget();



    private:
        void clearSubpath(void);

        void setSubpath(int index, POSE target, int mode);

        int SplitToSubTarget(POSE currentpose, POSE targetpose, DIRECTION direction);

        int  PosePackage(POSE pose,int x, int y, float angle);

        int  AttitudeAdjustToFork(POSE forkliftCurrentPos, POSE loadCarPos);

        POSE Relative2WorldCoordinate(POSE relativePos, POSE forkliftPoseInWorld);

        float Calculate2PointCrossYaxis(POSE pointA, POSE pointB);

        void ManageWheelSpeed();

        void ChangeRotateSpeedByAngle(float angle, ROTATE_DIRECTION direction);

        POSE GetCurrentPose();

        int RotateTest(float angle);

        int ForwardTest(POSE currentpose);

        int ForwardToLine(POSE currentpose, POSE targetpose);

        int BackwardToLine(POSE currentpose, POSE targetpose);

        int ForwardToArc(POSE currentpose, POSE targetpose);

        int BackwardToArc(POSE currentpose, POSE targetpose);

        int RotateLeft(float angle);

        int RotateRight(float angle);

        int Rotate(float angle);

        int StopFork(void);

        int wheelControl(int distance, float carAngle);

        int calTargetSpeedByPathDistance(int distance);

        int calAspeed();

        int calDspeed();

        void forwardSpeedInit(int *savedTargetSpeed, runMode *mode);

        int calFinalDelayDis(int *SpeedFinalFinal, float *Kdistance);

        int getAbsSpeed(void);

    private:
        int         pathIndex;
        bool        initParaFlag;
        ROTATEMODE  rotateMode;
        bool        haveBarrier;
        bool        isChangeSpeedInit;
        bool        isGlobalTargetSave;
        float       angle;
        POSE        startPose;
        POSE        targetPose;
        int         subpathSize;
        SUBPATH     subpath[5];
        int         currentLinearSpeed;
        float       currentAngularSpeed;
        int         lastLinearSpeed;
        float       lastAngularSpeed;
        float       baseAngularSpeed;
        int         forwardHighSpeed;
        int         forwardLowSpeed;
        int         arriveSpeed;
        int         backwardSpeed;
        int         turningSpeed;
        int         turningLowSpeed;
        int         turningArriveSpeed;
        int         initSpeed;
        int         targetSpeed;
        int         aspeed;
        int         dspeed;
        bool        isCarGetStuck;
        bool        isAvoidingBarrier;
        int         BarrierStatus;

        CAgvMathUtils *agvMathUtils;
        CAgvSetting * setting;
        float       DELAY_ANGLE_OFFSET;
        int       DELAY_ARC_DISTANCE_OFFSET;
        int       DELAY_DISTANCE_OFFSET;
        int       STANDARD_RADIUS;
        int       SavedXMLSTANDARD_RADIUS;
        float       ANGLE_OFFSET;
        int       LIDAR_LEFT_RIGHT_OFFSET_MM;
        int       wheelBase;
        int         MAXSPEEDALLOWED;
        int         LIDAR_TO_TAIL_LENGTH;
        int       carID;

        ofstream  output_X;
        ofstream  output_Y;
        ofstream  output_Angle;
};


#endif // AGVRUNNINGHELPERALGOTEST_H
