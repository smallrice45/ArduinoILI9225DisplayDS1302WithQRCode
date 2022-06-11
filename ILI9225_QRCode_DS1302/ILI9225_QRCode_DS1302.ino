// inClude RTC
#include <Wire.h>

#include <ThreeWire.h>
#include <RtcDS1302.h>
// InClude TFT
#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include "qrcode.h"

// Init DS1302
// DS1302接線指示: 可依需求修改
// DS1302 CLK/SCLK --> 10
// DS1302 DAT/IO --> 9
// DS1302 RST/CE --> 8
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND
ThreeWire myWire(9, 10, 8); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

#define countof(a) (sizeof(a) / sizeof(a[0]))

// Init TFT
#define TFT_RST A4
#define TFT_RS A3
#define TFT_CS A5  // SS
#define TFT_SDI A2 // MOSI
#define TFT_CLK A1 // SCK
#define TFT_LED A0  // 0 if wired to +5V directly

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED);

// Variables and constants
uint16_t x, y;
boolean flag = false;
uint16_t topBorder = 33;
uint16_t leftBorder = 13;
uint16_t codeSize = 5;

QRCode qrcode;

String currentTimeStr;
// Setup
void setup()
{
  InitTFT();
  InitRTC();
  DisplayCode("Init");
  currentTimeStr = getDateTime();
}

void loop()
{  
  // 判斷目前的時間是否更新，更新的話就刷新螢幕的時間QRCode
  Serial.print("Current RTC Time:");
  Serial.println(getDateTime());
  if (currentTimeStr != getTime()){
    currentTimeStr = getTime();
    tft.clear();
    UpdateDisplayRTC();
  }  
  delay(5000); // 5秒更新一次
}
void InitTFT(){
  #if defined(ESP32)
    hspi.begin();
    tft.begin(hspi);
  #else
    tft.begin();
  #endif
}
void InitRTC(){
  Serial.begin(9600);

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  //__DATE__，__TIME__，是程式碼編譯時的日期和時間
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println(getDateTime());
  Serial.println();

  //判斷DS1302是否接好
  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  //判斷DS1302上紀綠的時間和編譯時的時間，哪個比較新
  //如果編譯時間比較新，就進行設定，把DS1302上的時間改成新的時間
  //now：DS1302上紀綠的時間，compiled：編譯時的時間
  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    //編譯時間比較新，把DS1302上的時間改成編譯的時間
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
    //Rtc.SetDateTime(compiled);
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
}
//顯示完整年月日時間的副程式  
String getDateTime()
{
  RtcDateTime now = Rtc.GetDateTime();
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             now.Month(),
             now.Day(),
             now.Year(),
             now.Hour(),
             now.Minute(),
             now.Second() );
  if (!now.IsValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
  }
  return datestring;
}
//顯示時間的副程式  
String getTime()
{
  RtcDateTime now = Rtc.GetDateTime();
  char datestring[6];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u"),
             now.Hour(),
             now.Minute() );
  if (!now.IsValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
  }
  return datestring;
}

void UpdateDisplayRTC(){
  char cacheTime[10];
  getTime().toCharArray(cacheTime, 10);
  Serial.print("Current Display:");
  Serial.println(cacheTime);
  DisplayCode(cacheTime);
}


void UpdateRTC(){
  RtcDateTime now = Rtc.GetDateTime();
  Serial.println(getDateTime());
  Serial.println();
}

void DisplayCode(const char *codeData)
{
    tft.setBacklight(HIGH);
    uint8_t qrcodeData[qrcode_getBufferSize(3)];

    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, codeData);
    tft.fillRectangle(0, 0, 180, 220, COLOR_WHITE);
    for (uint8_t y = 0; y < qrcode.size; y++)
    {
        for (uint8_t x = 0; x < qrcode.size; x++)
        {
            tft.fillRectangle(leftBorder + x * codeSize, topBorder + y * codeSize, leftBorder + x * codeSize + codeSize, topBorder + y * codeSize + codeSize, qrcode_getModule(&qrcode, x, y) ? COLOR_BLACK : COLOR_WHITE);
        }
    }
}
