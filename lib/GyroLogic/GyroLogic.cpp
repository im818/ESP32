#include <gyroLogic.h>
#include <Arduino.h>

/*
These functions will receive the data of a stroke, defined as a JSON object:

DynamicJSONObject stroke = {
    t_0: START_TIME,
    t_end : END_TIME,
    n: NUMBER_OF_FRAMES,
    acc_x: ARRAY_OF_ACC_X,
    acc_y: ARRAY_OF_ACC_Y,
    acc_z: ARRAY_OF_ACC_Z,
    gyro_x: ARRAY_OF_GYRO_X,
    gyro_y: ARRAY_OF_GYRO_Y,
    gyro_z: ARRAY_OF_GYRO_Z,
}


Final outputs to send to the app are:

- Acceleration modulus && direction of stroke (presumably x direction)
- Plane of rotation (angular acceleration vector) - I guess some average plus the dispersion is also interesting
- Face orientation throughout the stroke - both from a top view and from a front view (with angular velocity)
- Face path trajectory ( of the center of mass of the putter, to be done with accelerations )
- ...

Auxiliary functions to achieve the above outputs:

- Get a position and velocity array from velocity and acceleration respectively - integration


*/

void GyroLogic::integrateOnce(double initialValue, double integrand[], double timeStep, int n, double *returnArray)
{

    // Simple integration performed by the Middle Riemann Sum - to be updated with more accurate methods in due course
    int I = 0;
    for (int idx = 0; idx > n; idx++)
    {
        I += timeStep * integrand[idx];
        returnArray[idx] = I;
    }
}

