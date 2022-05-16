#ifndef GyroLogic_h
#define GyroLogic_h

class GyroLogic
{
public:
    void integrateOnce(double initialValue, double integrand[], double timeStep, int n, double *returnArray);
    
};

#endif
