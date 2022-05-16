#include "InitProtocols.h"
#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h> // Include Websocket Library
#include <SPIFFS.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;

// Create a sensor object
Adafruit_MPU6050 mpu;

// Init MPU6050
void InitProtocols::initMPU()
{
    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
        {
            delay(10);
        }
    }
    Serial.println("MPU6050 Found!");
}

// Init server
void InitProtocols::serverOn()
{

    // Create AsyncWebServer object on port 80
    AsyncWebServer server(80);

    mpu.getEvent(&a, &g, &temp);
    DynamicJsonDocument accel_data(16384);
    DynamicJsonDocument gyro_data(16384);

    gyro_data = this->get_gyro_data(mpu);
    accel_data = this->get_accel_data(mpu);

    server.on("/acceleration", HTTP_GET, [accel_data](AsyncWebServerRequest *request)
              { AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(accel_data, *response);
      request->send(response); });

    server.on("/gyro", HTTP_GET, [gyro_data](AsyncWebServerRequest *request)
              { AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(gyro_data, *response);
      request->send(response); });

    // Start server
    server.begin();
}

String InitProtocols::updateSocket()
{
    mpu.getEvent(&a, &g, &temp);
    StaticJsonDocument<200> doc_tx;

    // Get current acceleration values
    accX = a.acceleration.x;
    accY = a.acceleration.y;
    accZ = a.acceleration.z;

    float gyroX_temp = g.gyro.x;
    float gyroY_temp = g.gyro.y;
    float gyroZ_temp = g.gyro.z;

    String jsonString = "";                      // create a JSON string for sending data to the client
    JsonObject object = doc_tx.to<JsonObject>(); // create a JSON Object

    object["accX"] = String(accX);
    object["accY"] = String(accY);
    object["accZ"] = String(accZ);
    object["gyroX"] = String(gyroX);
    object["gyroY"] = String(gyroY);
    object["gyroZ"] = String(gyroZ);

    serializeJson(doc_tx, jsonString); // convert JSON object to string
    Serial.println(jsonString);        // print JSON string to console for debug purposes (you can comment this out)
    return jsonString;
}

DynamicJsonDocument InitProtocols::get_accel_data(Adafruit_MPU6050 mpu)
{
    DynamicJsonDocument readings(16384);

    // Get current acceleration values
    accX = a.acceleration.x;
    accY = a.acceleration.y;
    accZ = a.acceleration.z;
    readings["accX"] = String(accX);
    readings["accY"] = String(accY);
    readings["accZ"] = String(accZ);

    return readings;
}

DynamicJsonDocument InitProtocols::get_gyro_data(Adafruit_MPU6050 mpu)
{

    DynamicJsonDocument readings(16384);
    
    float gyroX_temp = g.gyro.x;
    float gyroY_temp = g.gyro.y;
    float gyroZ_temp = g.gyro.z;

    readings["gyroX"] = String(gyroX);
    readings["gyroY"] = String(gyroY);
    readings["gyroZ"] = String(gyroZ);

    return readings;
}