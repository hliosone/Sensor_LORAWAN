#include <Preferences.h>
#include <TTN_BLE_esp32.h>
#include <heltec.h>
#include <TTN_esp32.h>
#include <TTN_CayenneLPP.h>
#include <DHT12.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>
#include <BMP180.h>
#include <BMP180I2C.h>
#include <Arduino.h>
    DHT12 dht12;
       
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60      /* Time ESP32 will go to sleep (in seconds) */
int ping=0;
const char *devEui = "002C990DF2D3F865";
const char *appEui = "70B3D57ED003EA08";
const char *appKey = "40C4412BA265CAACF671F1A071A20509";
TTN_esp32 ttn;
TTN_CayenneLPP lpp;
SSD1306Wire  aff(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_64_32);

Adafruit_CCS811 ccs;
BMP180I2C bmp180(0x77);
void setup() {
  Wire.begin(4,15);
  Serial.begin(115200);
  
  Serial.println("This message will never be printed");
 pinMode(Vext, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(Vext, LOW);
    dht12.begin();
  aff.init();
  aff.flipScreenVertically();
  aff.setFont(ArialMT_Plain_10);
  Wire.setClock(100000); // frequence compatible avec le DHT12
  ttn.begin();
  ttn.join(devEui, appEui, appKey);
  aff.clear();
  aff.drawString(0, 0, "Joining TTN...");
  aff.display();
  Serial.print("joining TTN ");
  while (!ttn.isJoined()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\njoined !");
  ttn.showStatus();
Serial.println("CCS811 test");
if(!ccs.begin()){
Serial.println("Failed to start sensor! Please check your wiring.");
while(1);
}
if (!bmp180.begin())
  {
    Serial.println("begin() failed. check your BMP180 Interface and I2C Address.");
    while (1);
  }

  //reset sensor to default parameters.
  bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);

}

void loop() {



 if (!bmp180.measurePressure())
  {
    Serial.println("");
    return;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true. 
  do
  {
    delay(100);
  } while (!bmp180.hasValue());
   float airq = ((float)ccs.geteCO2()/100);
  float pres = (bmp180.getPressure()/100);
 
  
  
  lpp.reset();
  lpp.addTemperature(1, temp);
  lpp.addRelativeHumidity(2, hum);
  lpp.addBarometricPressure(3, pres);
  lpp.addAnalogInput(4, airq);
  if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize())) {
    Serial.printf("Temp: %f Hum: %f CO2: %f Press: %f ", temp, hum, airq, (pres*100));
    Serial.println();
    aff.clear();
    aff.drawString(0, 0, "Pres:" + (String)pres + "ppm");
    aff.drawString(0, 20, "Temp:" + (String)temp + "°C");
    aff.drawString(0, 10, "Hum:" + (String)hum + "%");
   
    aff.display();}
    delay(5000);
}
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();  
  if (Serial.available() > 0) {
    int ping = Serial.read();

    switch (ping) {
       case '0':{
       float temp = dht12.readTemperature();
        Serial.println("0");
          lpp.reset();
  lpp.addTemperature(1, temp);
  if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize())) {
    Serial.printf("Temp: %f ", temp);
    Serial.println();
  }
        break;
       }
      case '1':{
       
      float hum = dht12.readHumidity();
        Serial.println("1");
        Serial.printf("Hum: %f ",hum);
        Serial.println();
        break;
      }
      case '2':
      {
        Serial.println("2");
        Serial.printf("Press: %f ",(pres*100));
        Serial.println();
        break;
      }
      case '3':{
        Serial.println("3");
    if(ccs.available()){  
  if(!ccs.readData()){
  Serial.print("CO2: ");
  Serial.print(ccs.geteCO2());
  Serial.print("ppm");
  Serial.print("\n");
 
  }
  else{
    Serial.println("ERROR!");
  while(1); 
    }
  };
        break;
      }
        }
          delay(1);        // delay in between reads for stability
  }
    }

 
