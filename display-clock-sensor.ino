//I2C device found at address 0x3C --> OLED-Display
//I2C device found at address 0x57 --> EEPROM on RTC
//I2C device found at address 0x68 --> RTC3231
//I2C device found at address 0x76 --> BME280
  
#include <Arduino.h>
#include <U8g2lib.h>
#include <Seeed_BME280.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define SERIAL_BAUD 57600

/* Constructor */
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R1, /* reset=*/ U8X8_PIN_NONE);
BME280 bme;
RtcDS3231<TwoWire> Rtc(Wire);

void setup() {
  Serial.begin(SERIAL_BAUD);
  while(!Serial) {} // Wait

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);
  
  Wire.begin();
  
  u8g2.begin();
  u8g2.setFlipMode(0);
  u8g2.enableUTF8Print();  
  
  if(!bme.init()) {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) 
  {
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) 
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
      Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

void loop() {
  if (!Rtc.IsDateTimeValid()) 
  {
      // Common Cuases:
      //    1) the battery on the device is low or even missing and the power line was disconnected
      Serial.println("RTC lost confidence in the DateTime!");
  }
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);
  delay(60000);
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt) {
    float pressure;
    char datestring[11];
    char timestring[9];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u.%02u.%04u"),
            dt.Day(),
            dt.Month(),
            dt.Year());
            
    snprintf_P(timestring, 
            countof(timestring),
            PSTR("%02u:%02u"),
            dt.Hour(),
            dt.Minute());
            
    //Serial.print(datestring);
    RtcTemperature temp = Rtc.GetTemperature();
    u8g2.firstPage();
    do {
      //u8g2.setFontMode(1);
      u8g2.setFont(u8g2_font_6x13_mf);
      u8g2.setCursor(0,15);
      u8g2.print(datestring);
      u8g2.setCursor(0,30);
      u8g2.print(timestring);
      u8g2.setCursor(0,45);
      u8g2.print(bme.getTemperature()); //+temp.AsFloatDegC()
      u8g2.setCursor(0,60);
      u8g2.print(bme.getHumidity());
      u8g2.setCursor(0,75);
      u8g2.print(pressure = bme.getPressure()/100);
      //u8g2.print(" / ");
      //u8g2.print(bme.calcAltitude(pressure));
      //u8g2.sendBuffer();
    } while ( u8g2.nextPage() );
}
