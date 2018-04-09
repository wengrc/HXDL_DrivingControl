/*
 * AgvGeometryUtils.h
 *
 *  Created on: 2017-1-5
 *      Author: zxq
 */
#include <stdio.h>
#ifndef AGVGEOMETRYUTILS_H_
#define AGVGEOMETRYUTILS_H_


#ifndef PI
#define PI                    3.14159265358979f
#endif
#define mDegree               1                //毫度
#define Degree                (1000 * mDegree) //度
#define PIXEL                 10               //以10毫米为一个像素点
#define ANGLE_TO_RADIAN( X )  (PI * X / 180)

#define ANGLE_360_DEGREE 360.0
#define ANGLE_270_DEGREE 270.0
#define ANGLE_180_DEGREE 180.0
#define ANGLE_90_DEGREE 90.0
#define ANGLE_60_DEGREE 60.0
#define ANGLE_45_DEGREE 45.0
#define ANGLE_15_DEGREE 15.0

#define ANGLE_181_DEGREE 181.0
#define ANGLE_30_DEGREE 30.0
#define ANGLE_20_DEGREE 20.0
#define ANGLE_10_DEGREE 10.0
#define ANGLE_5_DEGREE 5.0
#define ANGLE_1_DEGREE 1.0
#define ANGLE_HALF_DEGREE 0.5

#define DISTANCE_1000_MM 1000
#define DISTANCE_500_MM 500
#define DISTANCE_200_MM 200
#define DISTANCE_100_MM 100
#define DISTANCE_10_MM 10



typedef struct _polar
{
    float     distance;
    float     beta;
    float     alpha;
}POLAR;

typedef struct _position
{
    int     xPos;
    int     yPos;
    float     angle;
}POSITION;

typedef POSITION POSE;

typedef struct
{
    float  K;
    float  B;
}LINE;


typedef struct _battry
{
    int     vol;
    int     cap;
    int     ele;
}BATTERYMAG;


typedef enum _Direction
{
    forward = 0x01,
    backward = 0x02,
} DIRECTION;

typedef struct _POINT
{
    float x;
    float y;
} POINT;

typedef struct
{
    POINT center;
    float radius;
}  CIRCLE;

typedef struct _LINE1
{
    float L1;
    float L2;
    float L3;
} LINE1;

typedef struct _ARC
{
    POINT center;
    float radius;
} ARC;

typedef enum _PATHDIRECTION
{
    clockwise = 0x01,
    anticlockwise = 0x02,
} PATHDIRECTION;


class CAgvMathUtils
{
    public:
        CAgvMathUtils();

        virtual ~CAgvMathUtils();

        /*
         * 函数名：float getDisPoint
         * 描述  ：计算两点间的距离
         * 输入  ：两点坐标
         * 输出  ：距离
         */
        static float getDisPoint ( int x1, int y1, int x2, int y2 );

        /*
         * 函数名：float getDisPoint
         * 描述  ：计算两点间的距离
         * 输入  ：两点坐标
         * 输出  ：距离
         */
        static float getDisPoint ( POSITION *First, POSITION *Second );

        /*
         * 函数名：line getSlopPoint
         * 描述  ：已知两个点计算一般方程式
         * 输入  ：两点坐标
         * 输出  ：A , B , C
         */
        static LINE getSlopPoint ( POSITION *First, POSITION *Second );

        /*
         * 函数名：double getAnglePoint
         * 描述  ：计算两点间的所成直线与x轴的夹角
         * 输入  ：两点坐标
         * 输出  ：夹角0~359
         */
        static float getAnglePoint ( int x1, int y1, int x2, int y2);

        /*
         * 函数名：double getAnglePoint
         * 描述  ：计算两点间的所成直线与x轴的夹角
         * 输入  ：两点坐标
         * 输出  ：夹角0~359
         */
        static float getAnglePoint ( POSITION *First, POSITION *Second );

        /*
         * 函数名：LINE getVertical
         * 描述  ：过一点 做垂线
         * 输入  ：当前坐标两点坐标
         * 输出  ：垂线参数
         */
        static LINE getVertical ( POSITION *Curr_Point, POSITION *First, POSITION *Second );

        /*
         * 函数名：POSITION getInterPoint
         * 描述  ：过一点做垂线 与两点连线的交点
         * 输入  ：当前坐标两点坐标
         * 输出  ：交点
         */
        static POSITION getInterPoint( POSITION *Curr_Point, POSITION *First, POSITION *Second );

        /*
         * 函数名：float getDisPointToLine
         * 描述  ：求当前坐标到两个之间所构成直线距离和车身相对要纠偏的方向
         * 输入  ：当前坐标和构成直线的两点坐标
         * 输出  ：有方向的距离
         */
        static float getDisPointToLine ( POSITION *Curr_Point, POSITION *First, POSITION *Second );

        /*
         * 函数名：float getDisPointToLine
         * 描述  ：标准坐标系转化成极坐标系
         * 输入  ：标准坐标
         * 输出  ：极坐标
         */
        static POLAR rectToPolar ( POSITION *rectPosition );

        static float changeAngleScope ( float needChangeAngle , float scope);

        static float rangeRestriction(float value,float minValue,float maxValue);

        static int rangeRestriction(int value,int minValue,int maxValue);

        static LINE1 getLineFromAngleAndPoint(float angle, POINT point);

        static LINE1 getLineFromPoint(POINT startPoint, POINT endPoint);

        static float getLineAngleFromPoint(POINT startPoint, POINT endPoint);

        static float getDistanceFromPointToLine(POINT p1, LINE1 line);

        static float getDistanceFromPointToArc(POINT startPoint, ARC arc);

        static float getDistanceBetweenPoses(POSE startPose, POSE endPose);

        static float getDistanceBetweenPoints(POINT startPoint, POINT endPoint);

        static float getDistanceBetweenPose(POSE startPose, POSE endPose);

        static float getArcAngle(float lineAngle);

        static float getChordLength(POINT startPoint, POINT endPoint);

        static float getArcRadius(float arcLength, float arcAngle);

        static void limitAngle(float* angle, float min, float max);
        static LINE1 getVerticalLine(LINE1 line1, POINT p1);

        static POINT getIntersection(LINE1 line1, LINE1 line2);

        static float transformAngle(float angle, float minAngleLimit, float maxAngleLimit,float transformedAngleLimit);

        static float getAngleDiff(float currentAngle, float targetAngle);

        static POSE reTransformCoordinate(POSE poseReference, POSE poseTransformed);

        static POSE transformCoordinate(POSE poseReference, POSE poseToTransform);

        static POSE transformPose(POSE basepose, int offset);

        static POINT getPointFromPose(POSE pose1);

        static POSE getPoseFromPoint(POINT p1, float angle);

        static float getRotateAngle(POSE currentpose, POINT center, DIRECTION direction);

        static float getRotateAngle(POSE currentpose, POINT center, DIRECTION direction, PATHDIRECTION pathDirection);

        static CIRCLE getCircleCenter(POSE startpose, POSE targetpose );

        static void limitThrehold(int* variant, int min, int max);

        static void limitThrehold(float* variant, float min, float max);

        static POSE getPoseFrom_x_y_alpha(int x, int y, float angle);

        static POINT getPointFrom_x_y(float x, float y);

        static CIRCLE getCircleFromCenterAndRadius(float x, float y, float radius);

};

#endif /* AGVGEOMETRYUTILS_H_ */
