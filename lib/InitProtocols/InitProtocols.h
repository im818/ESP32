#ifndef InitProtocols_h
#define InitProtocols_h
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

class InitProtocols
{
public:
    void initMPU();
    void serverOn();
    String updateSocket();

private:
    DynamicJsonDocument get_accel_data(Adafruit_MPU6050 mpu);
    DynamicJsonDocument get_gyro_data(Adafruit_MPU6050 mpu);
};

#endif