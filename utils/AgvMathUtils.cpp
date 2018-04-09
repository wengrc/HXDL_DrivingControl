 /*
 * AgvGeometryUtils.cpp
 *
 *  Created on: 2017-1-5
 *      Author: zxq
 */

#include <math.h>

#include "AgvMathUtils.h"

CAgvMathUtils::CAgvMathUtils()
{
    // TODO Auto-generated constructor stub
}

CAgvMathUtils::~CAgvMathUtils()
{
    // TODO Auto-generated destructor stub
}


/*
 * 函数名：int getDisPoint
 * 描述  ：计算两点间的距离
 * 输入  ：两点坐标
 * 输出  ：距离
 */
float CAgvMathUtils::getDisPoint ( POSITION *First, POSITION *Second )
{
  float  decX, decY, disXY;
  decX = Second->xPos - First->xPos;
  decY = Second->yPos - First->yPos;
  disXY = sqrt ( decY * decY + decX * decX );
  return disXY;
}

/*
 * 函数名：int getDisPoint
 * 描述  ：计算两点间的距离
 * 输入  ：两点坐标
 * 输出  ：距离
 */
float CAgvMathUtils::getDisPoint ( int x1, int y1, int x2, int y2 )
{
  float  decX, decY, disXY;
  decX = x2 - x1;
  decY = y2 - y1;
  disXY = sqrt ( decY * decY + decX * decX );
  return disXY;
}


/*
 * 函数名：line getSlopPoint
 * 描述  ：已知两个点计算点斜方程式
 * 输入  ：两点坐标
 * 输出  ：K , B 
 */
static float minFlop = 0.01;
static float maxFlop = 1000;
#define NONE   0
LINE CAgvMathUtils::getSlopPoint ( POSITION *First, POSITION *Second )
{
    float  decX;
    float  decY;
	LINE   line;
	decY = Second->yPos - First->yPos;
	decX = Second->xPos - First->xPos;
	if ( decX == 0 && decY != 0 ) 
	{
		if ( decY > 0 )
		{
            line.K = maxFlop;
		}
		else
		{
			line.K = -maxFlop;
		}
	} 
	else if ( decY == 0 )
	{
		line.K = NONE;
	}
    else
	{
		line.K = decY / decX;
		if (fabs ( line.K ) <= minFlop)
		{
			line.K = NONE;
		}
		else if ( line.K >= maxFlop )
		{
			line.K = maxFlop;
		}
		else if ( line.K <= -maxFlop )
		{
			line.K = -maxFlop;
		}
	}
	
	if ( fabs ( line.K  ) <= maxFlop ) 
	{
		line.B = Second->yPos - line.K * Second->xPos;
	} 
	else
	{
		line.B = NONE;
	}
	
	return line;
}


/*
 * 函数名：double getAnglePoint
 * 描述  ：计算两点间的所成直线与x轴的夹角
 * 输入  ：两点坐标
 * 输出  ：夹角0~359
 */
float CAgvMathUtils::getAnglePoint ( POSITION *First, POSITION *Second )
{
    float  	decX;
    float  	decY;
    float		angle;

    decX = Second->xPos - First->xPos;
    decY = Second->yPos - First->yPos;
    angle = atan2 ( decY,decX );
    angle = angle * 180 / PI ;
    if(angle < 0)
    {
        angle = angle + 360 ;
    }

    return angle;
}
/*
 * 函数名：double getAnglePoint
 * 描述  ：计算两点间的所成直线与x轴的夹角
 * 输入  ：两点坐标
 * 输出  ：夹角0~359
 */
float CAgvMathUtils::getAnglePoint ( int x1, int y1, int x2, int y2)
{
    float  decX;
    float  decY;
    float  angle;

    decX = x2 - x1;
    decY = y2 - y1;
    angle = atan2 ( decY,decX );
    angle = angle * 180 / PI ;
    if(angle < 0)
    {
        angle = angle + 360 ;
    }
    else if(angle > 360)
    {
        angle = angle - 360 ;
    }

    return angle;
}


/*
 * 函数名：LINE getAnglePoint
 * 描述  ：过一点 做垂线
 * 输入  ：当前坐标两点坐标
 * 输出  ：垂线参数
 */

LINE CAgvMathUtils::getVertical ( POSITION *Curr_Point, POSITION *First, POSITION *Second )
{
    LINE lineXY;
	LINE lineVer;
	lineXY = getSlopPoint ( First, Second );

	if ( lineXY.K == NONE ) 
	{
		lineVer.K = maxFlop;
	} 
	else if ( fabs( lineXY.K ) == maxFlop ) 
	{
		lineVer.K = NONE;
	} 
	else 
	{
		lineVer.K = ( -1 / lineXY.K );
	}
	
	if ( fabs ( lineVer.K ) <= maxFlop ) 
	{
		lineVer.B = Curr_Point->yPos - lineVer.K * Curr_Point->xPos;
	} 
	else
	{
		lineVer.B = NONE;
	}
	
	return lineVer;
}


/*
 * 函数名：POSITION getInterPoint
 * 描述  ：过一点做垂线 与两点连线的交点
 * 输入  ：当前坐标两点坐标
 * 输出  ：交点
 */
POSITION CAgvMathUtils::getInterPoint( POSITION *Curr_Point, POSITION *First, POSITION *Second )
{
    LINE lineXY;
	LINE lineVer;
	POSITION interPoint;
	
	lineXY = getSlopPoint ( First, Second );
	lineVer = getVertical ( Curr_Point , First, Second );

	if ( lineXY.K == NONE ) 
	{
		interPoint.yPos = First->yPos;
		interPoint.xPos = Curr_Point->xPos;
	} 
    else if ( fabs( lineXY.K ) == maxFlop )
	{
		interPoint.xPos  = First->xPos;
		interPoint.yPos = Curr_Point->yPos;
	} 
	else 
	{
        interPoint.xPos = ( lineVer.B - lineXY.B ) / ( lineXY.K - lineVer.K );
        interPoint.yPos = ( lineVer.B - lineXY.B ) / ( lineXY.K - lineVer.K ) * lineXY.K + lineXY.B;
	}
	
	interPoint.angle = getAnglePoint ( Curr_Point , &interPoint );
	
	return interPoint;
}



/*
 * 函数名：float getDisPointToLine
 * 描述  ：求当前坐标到两个之间所构成直线距离和车身相对要纠偏的方向
 * 输入  ：当前坐标和构成直线的两点坐标
 * 输出  ：有方向的距离
 */
float CAgvMathUtils::getDisPointToLine ( POSITION *Curr_Point, POSITION *First, POSITION *Second )
{
//    int  decX;
//    int  decY;
    float  decXY;
    float  decAngle;
    float  conAngle;
    POSITION     interPoint;

    interPoint = getInterPoint ( Curr_Point, First, Second );


    decXY = getDisPoint( Curr_Point,&interPoint );
    conAngle = getAnglePoint ( First , Curr_Point );
    decAngle = getAnglePoint ( First , Second );


    {
        static int timeCount = 0;
        //timeCount ++;
        if(timeCount > 0)
        {
            timeCount = 0;
            printf("interPoint:(%d,%d) now(%d,%d)\n",interPoint.xPos,interPoint.yPos,Curr_Point->xPos,Curr_Point->yPos);
            printf("fandSAngle:%f   decXY = %f\n",decAngle , decXY);
            printf("conAngle:%f  decAngle:%f\n",conAngle, conAngle - decAngle);
        }
    }
//    printf("fandSAngle:%f   decXY = %d\n",decAngle , decXY);
    decAngle = conAngle - decAngle;

//    printf("conAngle:%f  decAngle:%f\n",conAngle, decAngle);
    if(decAngle <= -180)
    {
        decAngle = decAngle + 360;
    }
    else if(decAngle > 180)
    {
        decAngle = decAngle - 360;
    }

    if( decAngle >= 0)
    {
        return ( decXY );
    }
    else
    {
        return ( -decXY );
    }
}



/*
 * 函数名：float getDisPointToLine
 * 描述  ：标准坐标系转化成极坐标系
 * 输入  ：标准坐标
 * 输出  ：极坐标
 */
POLAR CAgvMathUtils::rectToPolar ( POSITION *rectPosition )
{
    POLAR polarPosition;
    float adjustAngel;
    polarPosition.distance = sqrt( rectPosition->xPos*rectPosition->xPos + rectPosition->yPos*rectPosition->yPos )/1000;
    polarPosition.beta = atan2 ( rectPosition->yPos,rectPosition->xPos );
    polarPosition.beta = -polarPosition.beta * 180 / PI ;

    if( rectPosition->angle > 180 )
    {
        adjustAngel = rectPosition->angle - 180;
    }
    else
    {
        adjustAngel = rectPosition->angle + 180;
    }
    polarPosition.alpha = - polarPosition.beta - adjustAngel;
    if(polarPosition.alpha > 180)
    {
        polarPosition.alpha = polarPosition.alpha - 360;
    }
    else if(polarPosition.alpha < -180)
    {
        polarPosition.alpha = polarPosition.alpha + 360;
    }
    return polarPosition;
}


float CAgvMathUtils::changeAngleScope ( float needChangeAngle , float scope)
{
    if(scope > ANGLE_360_DEGREE)
    {
        return needChangeAngle;
    }

    if(needChangeAngle > scope)
    {
        while(needChangeAngle > scope)
        {
            needChangeAngle = needChangeAngle - ANGLE_360_DEGREE;
        }
    }
    else if(needChangeAngle < (scope - ANGLE_360_DEGREE))
    {
        while(needChangeAngle < (scope - ANGLE_360_DEGREE))
        {
            needChangeAngle = needChangeAngle + ANGLE_360_DEGREE;
        }
    }
    return needChangeAngle;
}

float CAgvMathUtils::rangeRestriction(float value,float minValue,float maxValue)
{
    if(maxValue < minValue)
    {
        return value;
    }
    if(value > maxValue)
    {
        value = maxValue;
    }
    else if(value < minValue)
    {
        value = minValue;
    }
    return value;
}

int CAgvMathUtils::rangeRestriction(int value,int minValue,int maxValue)
{
    if(maxValue < minValue)
    {
        return value;
    }
    if(value > maxValue)
    {
        value = maxValue;
    }
    else if(value < minValue)
    {
        value = minValue;
    }
    return value;
}



LINE1 CAgvMathUtils::getLineFromAngleAndPoint(float angle, POINT point)
{
   float k = 0.0;
   LINE1 line;
   angle = angle * PI / 180;
   k = tan(angle);
   if(k >= 1000.0 || k < -1000.0)
   {
       line.L1 = 1;
       line.L2 = 0;
       line.L3 = -point.x;
   }
   else
   {
       line.L1 = k;
       line.L2 = -1;
       line.L3 = point.y - k * point.x ;
   }

   return line;
}

LINE1 CAgvMathUtils::getLineFromPoint(POINT startPoint, POINT endPoint)
{
   LINE1 line;

   if(startPoint.x == endPoint.x && startPoint.y == endPoint.y)
   {
       printf("can not get line from one point!!!\nPlease check your input points!!!\n");
       line.L1 = 0;
       line.L2 = 0;
       line.L3 = 0;
   }
   else if(startPoint.x == endPoint.x)
   {
       line.L1 = 1;
       line.L2 = 0;
       line.L3 = -startPoint.x;
   }
   else if(startPoint.y == endPoint.y)
   {
       line.L1 = 0;
       line.L2 = -1;
       line.L3 = startPoint.y;
   }
   else
   {
       line.L1 = (startPoint.y - endPoint.y)/(startPoint.x - endPoint.x);
       line.L2 = -1;
       line.L3 = -line.L1 * startPoint.x + startPoint.y;
   }

   return line;
}
float CAgvMathUtils::getLineAngleFromPoint(POINT startPoint, POINT endPoint)
{
   float angle = atan2((endPoint.y - startPoint.y),(endPoint.x - startPoint.x));
   angle = angle * 180 / PI;

   return angle;
}

float CAgvMathUtils::getDistanceFromPointToLine(POINT p1, LINE1 line)
{
   float distance = line.L1 * p1.x + line.L2 * p1.y + line.L3;
   distance = distance / sqrt(line.L1 * line.L1 + line.L2 * line.L2);

   return distance;
}

float CAgvMathUtils::getDistanceBetweenPoses(POSE startPose, POSE endPose)
{
   float decX = startPose.xPos - endPose.xPos;
   float decY = startPose.yPos - endPose.yPos;
   float distance = sqrt(decX * decX + decY * decY);

   return distance;
}

float CAgvMathUtils::getDistanceBetweenPoints(POINT startPoint, POINT endPoint)
{
   float decX = startPoint.x - endPoint.x;
   float decY = startPoint.y - endPoint.y;
   float distance = sqrt(decX * decX + decY * decY);

   return distance;
}
float CAgvMathUtils::getDistanceBetweenPose(POSE startPose, POSE endPose)
{
   float decX = startPose.xPos - endPose.xPos;
   float decY = startPose.yPos - endPose.yPos;
   float distance = sqrt(decX * decX + decY * decY);

   return distance;
}

float CAgvMathUtils::getDistanceFromPointToArc(POINT p1, ARC arc)
{
   float distance = getDistanceBetweenPoints(p1, arc.center) - arc.radius;

   return distance;
}

float CAgvMathUtils::getArcAngle(float lineAngle)
{
   float betaAngle;
   float gammaAngle;
   if(lineAngle < -90 || lineAngle > 90)
   {
       printf("input line angle error!!!\n");
   }
   if(lineAngle >= 0)
   {
       betaAngle = 90 - lineAngle;
   }
   else if(lineAngle < 0)
   {
       betaAngle = 90 + lineAngle;
   }

   gammaAngle = 180 - 2 * betaAngle;

   return gammaAngle;
}

float CAgvMathUtils::getChordLength(POINT startPoint, POINT endPoint)
{
   float distance = getDistanceBetweenPoints(startPoint, endPoint);

   return distance;
}

float CAgvMathUtils::getArcRadius(float arcLength, float arcAngle)
{
   //float angleDegree = arcAngle * 180 / PI;
   float radius = arcLength / sin(arcAngle / 2);

   return radius;
}

void CAgvMathUtils::limitAngle(float* angle, float min, float max)
{
   if(*angle < min)
   {
       *angle = min;
   }
   else if(*angle > max)
   {
       *angle = max;
   }
}

LINE1 CAgvMathUtils::getVerticalLine(LINE1 line1, POINT p1)
{
   LINE1 verticalLine;
   if(0 == line1.L1 && 0 == line1.L2)
   {
       printf("input line format error!!!\n");
       verticalLine.L1 = 0;
       verticalLine.L2 = 0;
       verticalLine.L3 = 0;
   }
   else if(0 == line1.L1)
   {
       verticalLine.L1 = -1;
       verticalLine.L2 = 0;
       verticalLine.L3 = p1.x;
   }
   else if(0 == line1.L2)
   {
       verticalLine.L1 = 0;
       verticalLine.L2 = -1;
       verticalLine.L3 = p1.y;
   }
   else
   {
       verticalLine.L1 = -1 / line1.L1;
       verticalLine.L2 = line1.L2;
       verticalLine.L3 = -(verticalLine.L1 * p1.x + verticalLine.L2 * p1.y);
   }

   return verticalLine;
}


POINT CAgvMathUtils::getIntersection(LINE1 line1, LINE1 line2)
{
   POINT intersection;
   intersection.x = 0.0;
   intersection.y = 0.0;

   if(0.0 == line1.L1 && 0.0 == line1.L2)
   {
       printf("input line1 format error!!!\n");
   }
   else if(0.0 == line2.L1 && 0.0 == line2.L2)
   {
       printf("input line2 format error!!!\n");
   }//parallel
   else if(line1.L1 * line2.L2 == line1.L2 * line2.L1)
   {
       printf("parallel lines have no intersection.");
   }
   else
   {
       intersection.x = (-line1.L3 * line2.L2 + line1.L2 * line2.L3) / (line1.L1 * line2.L2 - line1.L2 * line2.L1);
       if(line1.L2 == 0.0)
       {
           intersection.y = -(line2.L1 * intersection.x + line2.L3) / line2.L2;
       }
       else
       {
           intersection.y = -(line1.L1 * intersection.x + line1.L3) / line1.L2;
       }

   }

   return intersection;
}

//transform the angle from [minAngleLimit, maxAngleLimit) to (-transformedAngleLimit, transformedAngleLimit]
float CAgvMathUtils::transformAngle(float angle, float minAngleLimit, float maxAngleLimit,float transformedAngleLimit)
{
   if(angle >= maxAngleLimit || angle < minAngleLimit)
   {
       printf("angle format error: %f!!!\n", angle);
       angle = 0.0;
   }
   else if(angle > transformedAngleLimit)
   {
       angle = angle - transformedAngleLimit * 2;
   }
   else if(angle <= -transformedAngleLimit)
   {
       angle = angle + transformedAngleLimit * 2;
   }

   return angle;
}


float CAgvMathUtils::getAngleDiff(float currentAngle, float targetAngle)
{
   float decAngle = 0;

   if(currentAngle > ANGLE_180_DEGREE || currentAngle <= -ANGLE_180_DEGREE)
   {
       printf("current angle format error!!!\n");
   }
   if(targetAngle > ANGLE_180_DEGREE || targetAngle <= -ANGLE_180_DEGREE)
   {
       printf("target angle format error!!!\n");
   }

   decAngle = currentAngle - targetAngle;

   if(decAngle > ANGLE_180_DEGREE)
   {
       decAngle = decAngle - ANGLE_360_DEGREE;
   }
   else if(decAngle <= -ANGLE_180_DEGREE)
   {
       decAngle = ANGLE_360_DEGREE + decAngle;
   }

   return decAngle;
}

POSE CAgvMathUtils::transformCoordinate(POSE poseReference, POSE poseToTransform)
{
   float angle = poseReference.angle * PI / 180.0;
   POSE poseTransformed;
   float decX = poseToTransform.xPos - poseReference.xPos;
   float decY = poseToTransform.yPos - poseReference.yPos;

   poseTransformed.angle = poseToTransform.angle - poseReference.angle;

   if(poseTransformed.angle < 0.0)
   {
       poseTransformed.angle = poseTransformed.angle + ANGLE_360_DEGREE;
   }

   poseTransformed.xPos = decX * cos(angle) + decY * sin(angle);
   poseTransformed.yPos = decY * cos(angle) - decX * sin(angle);

   return poseTransformed;
}

POSE CAgvMathUtils::reTransformCoordinate(POSE poseReference, POSE poseTransformed)
{
   float angle = poseReference.angle * PI / 180;
   POSE poseToTransform;
   float decX = cos(angle) * poseTransformed.xPos - sin(angle) * poseTransformed.yPos;
   float decY = sin(angle) * poseTransformed.xPos + cos(angle) * poseTransformed.yPos;

   poseToTransform.angle = poseTransformed.angle + poseReference.angle;

   if(poseToTransform.angle >= ANGLE_360_DEGREE)
   {
       poseToTransform.angle = poseToTransform.angle - ANGLE_360_DEGREE;
   }

   poseToTransform.xPos = poseReference.xPos + decX;
   poseToTransform.yPos = poseReference.yPos + decY;

   return poseToTransform;
}

POSE CAgvMathUtils::transformPose(POSE basepose, int offset)
{
    POSE newpose;

    newpose.xPos = basepose.xPos + offset * cos(PI * basepose.angle / ANGLE_180_DEGREE);
    newpose.yPos = basepose.yPos + offset * sin(PI * basepose.angle / ANGLE_180_DEGREE);
    newpose.angle = basepose.angle;

   return newpose;
}

POINT CAgvMathUtils::getPointFromPose(POSE pose1)
{
   POINT point1;
   point1.x = (float)pose1.xPos;
   point1.y = (float)pose1.yPos;

   return point1;
}

POSE CAgvMathUtils::getPoseFromPoint(POINT p1, float angle)
{
   POSE pose1;

   pose1.xPos = p1.x;
   pose1.yPos = p1.y;
   pose1.angle = angle;

   return pose1;
}

float CAgvMathUtils::getRotateAngle(POSE currentpose, POINT center, DIRECTION direction)
{
    POINT currentPoint;
    float rotateAngle;
    float angleDiff;

    currentPoint.x = currentpose.xPos;
    currentPoint.y = currentpose.yPos;

    if(center.y >= 0.0)
    {
        rotateAngle = getLineAngleFromPoint(currentPoint, center);
        rotateAngle = rotateAngle - 90.0;
        //printf("rotateAngle: %f\n", rotateAngle);
        if(forward == direction)
        {
            rotateAngle = rotateAngle + ANGLE_360_DEGREE;
        }
        else
        {
            if(rotateAngle < 0.0)
            {
                rotateAngle = ANGLE_360_DEGREE + rotateAngle;
            }
            else
            {

            }
        }

        //printf("rotateAngle: %f\n", rotateAngle);

        angleDiff = currentpose.angle - rotateAngle ;
        angleDiff = transformAngle(angleDiff, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

        printf("angle diff: %f\n", angleDiff);
    }
    else
    {
        rotateAngle = getLineAngleFromPoint(currentPoint, center);
        rotateAngle = rotateAngle + 90.0;
        if(backward == direction)
        {
            if(rotateAngle < 0.0)
            {
                rotateAngle = rotateAngle + ANGLE_360_DEGREE;
            }
            else
            {

            }

        }
        else
        {

        }

        //printf("rotateAngle: %f\n", rotateAngle);

        angleDiff = currentpose.angle - rotateAngle;
        angleDiff = transformAngle(angleDiff,  -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

        //printf("angle diff: %f\n", angleDiff);
    }

    return angleDiff;
}



float CAgvMathUtils::getRotateAngle(POSE currentpose, POINT center, DIRECTION direction, PATHDIRECTION pathDirection)
{
   POINT currentPoint;
   float rotateAngle;
   float angleDiff;

   currentPoint.x = currentpose.xPos;
   currentPoint.y = currentpose.yPos;

//   if((center.y >= 0.0 && clockwise == pathDirection)
//           || (center.y < 0 && clockwise == pathDirection))
   if(anticlockwise == pathDirection)
   {
       rotateAngle = getLineAngleFromPoint(currentPoint, center);
       rotateAngle = rotateAngle - ANGLE_90_DEGREE;

       if(rotateAngle < 0.0)
       {
           rotateAngle = rotateAngle + ANGLE_360_DEGREE;
       }

       //printf("rotateAngle: %f\n", rotateAngle);

       angleDiff = currentpose.angle - rotateAngle ;
       angleDiff = transformAngle(angleDiff, -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

       //printf("angle diff: %f\n", angleDiff);
   }
   else
   {
       rotateAngle = getLineAngleFromPoint(currentPoint, center);
       rotateAngle = rotateAngle + ANGLE_90_DEGREE;

       if(rotateAngle < 0.0)
       {
           rotateAngle = rotateAngle + ANGLE_360_DEGREE;
       }
       else if(rotateAngle >= ANGLE_360_DEGREE)
       {
           rotateAngle = rotateAngle - ANGLE_360_DEGREE;
       }

       //printf("rotateAngle: %f\n", rotateAngle);

       angleDiff = currentpose.angle - rotateAngle;
       angleDiff = transformAngle(angleDiff,  -ANGLE_360_DEGREE, ANGLE_360_DEGREE, ANGLE_180_DEGREE);

       //printf("angle diff: %f\n", angleDiff);
   }

   return angleDiff;
}

CIRCLE CAgvMathUtils::getCircleCenter(POSE startpose, POSE targetpose )
{
   POINT startPoint;
   POINT targetPoint;
   POINT midPoint;

   LINE1 l1;//VerticalLine
   LINE1 l2;//VerticalLine
   LINE1 l3;//target point tangent line
   LINE1 l4;//start point - target point  line

   POINT circleCenter;
   float radius;
   CIRCLE circle;

   startPoint.x = startpose.xPos;
   startPoint.y = startpose.yPos;
   targetPoint.x = targetpose.xPos;
   targetPoint.y = targetpose.yPos;

   midPoint.x = (startPoint.x + targetPoint.x) / 2;
   midPoint.y = (startPoint.y + targetPoint.y) / 2;

   printf("(%f, %f)\n", midPoint.x, midPoint.y);

   l4 = getLineFromPoint(startPoint, targetPoint);
   l3 = getLineFromAngleAndPoint(targetpose.angle, targetPoint);

   printf("l4:%f %f %f\n", l4.L1,l4.L2,l4.L3);
   printf("l3:%f %f %f\n", l3.L1,l3.L2,l3.L3);
   l2 = getVerticalLine(l4, midPoint);

   l1 = getVerticalLine(l3, targetPoint);

   printf("l2:%f %f %f\n", l2.L1,l2.L2,l2.L3);
   printf("l1:%f %f %f\n", l1.L1,l1.L2,l1.L3);
   circleCenter = getIntersection(l1, l2);

   radius = getDistanceBetweenPoints(circleCenter, targetPoint);

   printf("start point:(%f, %f)\n", startPoint.x, startPoint.y);
   printf("center:(%f, %f) R: %f\n", circleCenter.x, circleCenter.y, radius);

   circle.center = circleCenter;
   circle.radius = radius;

   return circle;
}


void CAgvMathUtils::limitThrehold(int* variant, int min, int max)
{
   if(*variant < min)
   {
       *variant = min;
   }
   else if(*variant > max)
   {
       *variant = max;
   }
}

void CAgvMathUtils::limitThrehold(float* variant, float min, float max)
{
   if(*variant < min)
   {
       *variant = min;
   }
   else if(*variant > max)
   {
       *variant = max;
   }
}


POSE CAgvMathUtils::getPoseFrom_x_y_alpha(int x, int y, float angle)
{
   POSE pose;
   pose.xPos = x;
   pose.yPos = y;
   pose.angle = angle;

   return pose;
}


POINT CAgvMathUtils::getPointFrom_x_y(float x, float y)
{
   POINT point;
   point.x = x;
   point.y = y;

   return point;
}

CIRCLE CAgvMathUtils::getCircleFromCenterAndRadius(float x, float y, float radius)
{
   CIRCLE circle;

   circle.center.x = x;
   circle.center.y = y;
   circle.radius = radius;

   return circle;
}

