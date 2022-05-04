// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h> // Include Websocket Library
#include <SPIFFS.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Ticker.h>

// Replace with your network credentials
const char *ssid = "ESP32";
const char *password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Create a sensor object
Adafruit_MPU6050 mpu;

Ticker timer;
sensors_event_t a, g, temp;

StaticJsonDocument<200> doc_tx;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

// Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

// Init MPU6050
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

bool readDataFlag = false;
void readData()
{
  readDataFlag = true;
}

void setup()
{
  // begin serial com for debugging
  Serial.begin(115200);

  initMPU();
  // set up soft AP
  WiFi.softAP(ssid, password);

  // publish the public IP of the AP
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/acceleration", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument readings(1024);
      mpu.getEvent(&a, &g, &temp);

      // Get current acceleration values
      accX = a.acceleration.x;
      accY = a.acceleration.y;
      accZ = a.acceleration.z;
      readings["accX"] = String(accX);
      readings["accY"] = String(accY);
      readings["accZ"] = String(accZ);

      serializeJson(readings, *response);
      request->send(response); });

  server.on("/gyro", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument readings(1024);
      mpu.getEvent(&a, &g, &temp);

      float gyroX_temp = g.gyro.x;
      if (abs(gyroX_temp) > gyroXerror)
      {
        gyroX += gyroX_temp / 50.00;
      }

      float gyroY_temp = g.gyro.y;
      if (abs(gyroY_temp) > gyroYerror)
      {
        gyroY += gyroY_temp / 70.00;
      }

      float gyroZ_temp = g.gyro.z;
      if (abs(gyroZ_temp) > gyroZerror)
      {
        gyroZ += gyroZ_temp / 90.00;
      }

      readings["gyroX"] = String(gyroX);
      readings["gyroY"] = String(gyroY);
      readings["gyroZ"] = String(gyroZ);

      serializeJson(readings, *response);
      request->send(response); });

  // Start server
  server.begin();
  webSocket.begin();
  timer.attach(1, readData);
}

void loop()
{
  webSocket.loop();
  if (readDataFlag)
  {
    mpu.getEvent(&a, &g, &temp);

    // Get current acceleration values
    accX = a.acceleration.x;
    accY = a.acceleration.y;
    accZ = a.acceleration.z;

    float gyroX_temp = g.gyro.x;
    if (abs(gyroX_temp) > gyroXerror)
    {
      gyroX += gyroX_temp / 50.00;
    }

    float gyroY_temp = g.gyro.y;
    if (abs(gyroY_temp) > gyroYerror)
    {
      gyroY += gyroY_temp / 70.00;
    }

    float gyroZ_temp = g.gyro.z;
    if (abs(gyroZ_temp) > gyroZerror)
    {
      gyroZ += gyroZ_temp / 90.00;
    }

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
    webSocket.broadcastTXT(jsonString);

    readDataFlag = false;
  }
}