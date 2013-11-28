#ifndef FIRE_H
#define FIRE_H

#include "math.h"

class Fire{
private:
    Fire();
    virtual ~Fire() = 0;

public:
    struct FireObject{
        Point3<float> pivotPoint;
        Point3<float> axisX;
        Point3<float> axisY;
        Point3<float> axisZ;
        Point3<float> massCenter;
    };

    struct FireScanProgram{
        unsigned int pressureTable[10];
        unsigned int prNumber;
        Point2<unsigned int> point1;
        Point2<unsigned int> point2;
        unsigned int step;
        unsigned int nasadokPosition;
    };
};

#endif

