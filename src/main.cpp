// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h> // Include Websocket Library
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Ticker.h>
#include "InitProtocols.h"
//#include "GyroLogic.h"

// Replace with your network credentials
const char *ssid = "ESP32";
const char *password = "12345678";

Ticker timer;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
sensors_event_t a, g, temp;

// Create a sensor object
Adafruit_MPU6050 mpu;

bool readDataFlag = false;
bool stopWebSocket = false;
void readData()
{
  readDataFlag = true;
}

void initMPU()
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

void serverOn()
{

  server.on("/live-acceleration", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncResponseStream *response = request->beginResponseStream("application/json");

      DynamicJsonDocument accel_data(1024);
      
      mpu.getEvent(&a, &g, &temp);

      // Get current acceleration values
      accX = a.acceleration.x;
      accY = a.acceleration.y;
      accZ = a.acceleration.z;

      accel_data["accX"] = String(accX);
      accel_data["accY"] = String(accY);
      accel_data["accZ"] = String(accZ);

      serializeJson(accel_data, *response);
      request->send(response); });

  server.on("/live-gyro", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncResponseStream *response = request->beginResponseStream("application/json");
      
      Serial.println("Processing stroke recording request ...");
      DynamicJsonDocument gyro_data(1024);

      mpu.getEvent(&a, &g, &temp);

      float gyroX_temp = g.gyro.x;
      float gyroY_temp = g.gyro.y;
      float gyroZ_temp = g.gyro.z;

      gyro_data["gyroX"] = String(gyroX);
      gyro_data["gyroY"] = String(gyroY);
      gyro_data["gyroZ"] = String(gyroZ);

      serializeJson(gyro_data, *response);
      request->send(response); });

  server.on("/record-stroke", HTTP_GET, [](AsyncWebServerRequest *request)
            {AsyncResponseStream *response = request->beginResponseStream("application/json");
      
      stopWebSocket = true;
      DynamicJsonDocument strokeData(1024);
      
      // create nested array within JSON object
      JsonArray accelerationX = strokeData.createNestedArray("acceleration_x");
      JsonArray accelerationY = strokeData.createNestedArray("acceleration_y");
      JsonArray accelerationZ = strokeData.createNestedArray("acceleration_z");

      // hard code how many readings to be taken for one shot
      int n = 20;

      for (int i = 0; i < n; i++)
      {

        mpu.getEvent(&a, &g, &temp);

        accelerationX.add(String(a.acceleration.x));
        accelerationY.add(String(a.acceleration.y));
        accelerationZ.add(String(a.acceleration.z));

        delay(100);
        Serial.println(i);
      };

      serializeJson(strokeData, *response);
      request->send(response); });

  //  Start server
  server.begin();
}

String updateSocket()
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

DynamicJsonDocument recordStroke()
{

  // declare JSON object
  DynamicJsonDocument strokeData(1024);

  // create nested array within JSON object
  JsonArray accelerationX = strokeData.createNestedArray("acceleration_x");
  JsonArray accelerationY = strokeData.createNestedArray("acceleration_y");
  JsonArray accelerationZ = strokeData.createNestedArray("acceleration_z");

  // hard code how many readings to be taken for one shot
  int n = 20;

  for (int i = 0; i < n; i++)
  {

    mpu.getEvent(&a, &g, &temp);

    accelerationX.add(a.acceleration.x);
    accelerationY.add(a.acceleration.y);
    accelerationZ.add(a.acceleration.z);
  }

  return strokeData;
}

DynamicJsonDocument getAccelData()
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

DynamicJsonDocument getGyroData()
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

void setup()
{
  // begin serial com for debugging
  Serial.begin(115200);

  // set up MPU
  initMPU();

  // set up soft AP
  WiFi.softAP(ssid, password);

  // publish the public IP of the AP
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  serverOn();
  timer.attach(1, readData);
}

void loop()
{
  while (!stopWebSocket)
  {
    webSocket.begin();
    webSocket.loop();
    if (readDataFlag)
    {
      String jsonString = updateSocket();
      webSocket.broadcastTXT(jsonString);
      readDataFlag = false;
    }
  }
}