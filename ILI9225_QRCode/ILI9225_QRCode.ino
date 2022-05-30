#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include "qrcode.h"

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

// Setup
void setup()
{
#if defined(ESP32)
    hspi.begin();
    tft.begin(hspi);
#else
    tft.begin();
#endif
    Serial.begin(9600);
    DisplayCode("MAGICWING");
}

void loop()
{

  
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
