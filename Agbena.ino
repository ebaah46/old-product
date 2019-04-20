
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "time.h"

const char* ntpServer = "africa.pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

String date;
// Definition of delay varialbles
static const uint32_t DELAY_1_S = 1000UL;
static const uint32_t DELAY_1_MIN = 60UL * DELAY_1_S;
static const uint32_t DELAY_1_30MIN = 30UL * DELAY_1_MIN;

WiFiMulti wifiMulti;

 HTTPClient http;

// Interval per reading Definitions for use
int INTERVAL_READING = 50;
int INTERVAL_READING_TIME_SPACING = 100;

//MQ sensor pin definitions 
#define SENSORTHRES 2500
#define   mq2_pin   39    
int mqState;

// O2 Sensor predefinitions
#define O2_preheat_time  60000
#define O2_percentage 20.80
#define O2_pin 36
float O2;
bool O2_init_flag = 0;          
float calibration_voltage; 

// Date Library predefinitions

// Variables to save date and time
String formattedDate;
String dayStamp;


char serverAddress[] = "https://old-backend.herokuapp.com/old/data";  

void setup() {

    Serial.begin(115200);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    wifiMulti.addAP("BEKs", "alelepof");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    wifiMulti.run();
    http.begin(serverAddress);
    Serial.println("Connecting to web server......."); 
//    printLocalTime();
}

void loop() {
    String oxy =  getOxyVal(O2IntervalReading());
    delay(2000);
    String mq =  getMqState(checkMQ());
 if(wifiMulti.run()==WL_CONNECTED){
    postData(oxy,mq);    
    Serial.println("making POST request.....");
    }
        
    delay(DELAY_1_30MIN);
}


// MQ function definitions

int readMQ(){
  int analogSensor = analogRead(mq2_pin);
  return analogSensor;
  }


// Checking MQ state with small logic
     int checkMQ(){ 
        if(mqIntervalReading() < SENSORTHRES){
            mqState = 0;
            return mqState;
        }
        else{
          mqState = 1;

        }
      return mqState;
     }

// O2 Sensor  Function definitions 

// *********************************** //

float O2_value(){
  unsigned int sum = 0;
  if (O2_init_flag == 0)
  {
    O2_init_flag = 1;         
    pinMode(O2_pin, INPUT);
    for (unsigned char i = 64;i > 0;i--)
    {
      sum = sum + analogRead(O2_pin);
      delay(100);
    }
    sum = sum >> 6;
    calibration_voltage = sum / O2_percentage;
    return 20.80;           
  }
  else
  {
    for (unsigned char i = 32;i > 0;i--)
    {
      sum = sum + analogRead(O2_pin);
      delay(50);
    }
    sum = sum >> 5;
    float output = sum / calibration_voltage;

    return output;
    
  }
}

String getOxyVal(float oxyVal){
  String Use = String(oxyVal);
  return Use;  
  }

String getMqState(int mqState){
  String myUse = String(mqState);
  return myUse;
  }



// interval Reading function definitions

// O2 interval reading
float O2IntervalReading(){
    float value = 0.0;
    Serial.println("Reading OXYGEN value....");
    for(int i =0;i<INTERVAL_READING;i++){
        value += O2_value();
        delay(INTERVAL_READING_TIME_SPACING);
      }
    value = value/ INTERVAL_READING;
  Serial.println(value);
return value;
  }

// MQ interval reading
long mqIntervalReading(){
  long  value = 0;
     Serial.println("Reading MQ value....");
  for(int i=0;i<INTERVAL_READING;i++){
      value += readMQ();
      delay(INTERVAL_READING_TIME_SPACING); 
    }
  value = value/INTERVAL_READING;
  Serial.println(value);
return value;
  }

// Get time function definition
String printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
   String d = String(timeinfo.tm_mday);
   String yr = String(2019);
   String mon = String(timeinfo.tm_mon+1);
   Serial.println(date = yr + "-" + mon + "-"+ d);
  return date;
}

  // Post data to server function definition
void postData(String oxyVal, String mqState){
      // Post Request details
      int statusCode  = 0;
      String response = " ";
      // Declaration of arduino json object size
      StaticJsonDocument<300> doc;
      char JsonMessageBuffer[300];  
      doc["oxyVal"] = oxyVal;
      doc["mqState"] = mqState;
      doc["now"] = printLocalTime();
      serializeJsonPretty(doc, Serial);
      serializeJsonPretty(doc,JsonMessageBuffer);
      http.addHeader("Content-Type", "application/json",false,true);
      statusCode = http.POST(JsonMessageBuffer);
      response = http.getString();
      while(statusCode != 201)
      {
          Serial.println("Data could not be posted.");
          Serial.println("Response received:");
          Serial.println(statusCode);
          statusCode = http.POST(JsonMessageBuffer);
              
      }
      Serial.println("Data successfully posted.");
      Serial.println("Response received:");
      Serial.println(response);
      http.end();
  }

      
 
  
