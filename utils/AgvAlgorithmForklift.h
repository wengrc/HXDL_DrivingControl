/*
 * AgvAlgorithm.h
 *
 *  Created on: 2017-05-25
 *      Author: crystal
*/
#ifndef AGVALGORITHM_FORKLIFT_H
#define AGVALGORITHM_FORKLIFT_H

#define MAX_DISTANCE_VERY_PRECISE  10 //mm
#define MAX_CAR_ANGLE_VERY_PRECISE 1.0 //degree
#define MAX_OUTPUT_FOR_PRECISE          8

#define MAX_DISTANCE_CRUDE  100 //mm
#define MAX_CAR_ANGLE_CRUDE 8.0 //degree

#define MAX_DISTANCE_PRECISE 30 //mm
#define MAX_ANGLE_PRECISE 5.0//2.0 //degree

#define DEAD_ZONE_DISTANCE 20 //mm
#define DEAD_ZONE_ANGLE 1 //degree

#define MAX_OUTPUT_FOR_CRUDE    5//30


class CAgvFuzzyForklift
{
    public:
    CAgvFuzzyForklift();
    ~CAgvFuzzyForklift();

    int output(int distance, float carAngle,  int type);
    int getValByIndex(int row,int column, ControlType type);
    private:

};

#define INTEGRATION_THREHOLD 4

#define KP 10
#define KI 0
#define KD 0


class CAgvAIForklift
{
    public:
    CAgvAIForklift();
    ~CAgvAIForklift();

    int output(int distance, float carAngle, int type);

    private:
};

#endif /* AGVALGORITHM_H */
