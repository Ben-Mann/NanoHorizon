/***************************************************
  This is a example sketch demonstrating graphic drawing
  capabilities of the SSD1351 library for the 1.5"
  and 1.27" 16-bit Color OLEDs with SSD1351 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1431
  ------> http://www.adafruit.com/products/1673

  If you're using a 1.27" OLED, change SCREEN_HEIGHT to 96 instead of 128.

  These displays use SPI to communicate, 4 or 5 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  The Adafruit GFX Graphics core library is also required
  https://github.com/adafruit/Adafruit-GFX-Library
  Be sure to install it!
 ****************************************************/

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

// You can use any (4 or) 5 pins
#define SCLK_PIN 2
#define MOSI_PIN 3
#define DC_PIN   4
#define CS_PIN   5
#define RST_PIN  6

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#define RUN_TEST_PATTERNS 0

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>


// Option 1: use any pins but a little slower
//Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);

// Option 2: must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

Adafruit_MPU6050 mpu;
char hasMpu = 0;

float p = 3.1415926;

void testdrawtext(char *text, uint16_t color) {
    tft.setCursor(0,0);
    tft.setTextColor(color);
    tft.print(text);
}

char line = 0;
void printLn(char *text) {
    tft.setCursor(0, line);
    tft.print(text);
    line += 12;
}

void setup(void) {
    Serial.begin(9600);
    Serial.print("hello!");
    pinMode(10, OUTPUT);
    tft.begin();

    Serial.println("init");

    // You can optionally rotate the display by running the line below.
    // Note that a value of 0 means no rotation, 1 means 90 clockwise,
    // 2 means 180 degrees clockwise, and 3 means 270 degrees clockwise.
    //tft.setRotation(1);
    // NOTE: The test pattern at the start will NOT be rotated!  The code
    // for rendering the test pattern talks directly to the display and
    // ignores any rotation.

    uint16_t time = millis();
    tft.fillRect(0, 0, 128, 128, BLACK);
    time = millis() - time;

    Serial.println(time, DEC);

    tft.setTextColor(GREEN);
    printLn("Artificial Horizon v0.9");

    if (mpu.begin()) {
        hasMpu = 1;
        mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
        mpu.setGyroRange(MPU6050_RANGE_250_DEG);
        mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        printLn("MPU initialised.");
    } else {
        testdrawtext("Cannot initialise MPU", RED);
    }
    delay(500);
     tft.fillScreen(BLACK);
}

float v[7] = { 0, 0, 0, 0, 0, 0, 0 };
//char first = 1;

void printValue(float value, int index) {
    int y = index * 12;
//    if (!first) {
//        tft.setTextColor(BLACK);
//        tft.setCursor(0, y);
//        tft.print(v[index]);
//    }
    tft.setTextColor(index < 3 ? WHITE : index < 6 ? YELLOW : CYAN);
    tft.setCursor(0, y);
    tft.print(value);
    v[index] = value;
}

void testMpu6050() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

//    printValue(a.acceleration.x, 0);
//    printValue(a.acceleration.y, 1);
//    printValue(a.acceleration.z, 2);
//
//    printValue(g.gyro.x, 3);
//    printValue(g.gyro.y, 4);
//    printValue(g.gyro.z, 5);
//
//    printValue(temp.temperature, 6);
//    first = 0;
}

// We want to display numbers, and we want to do that very fast.
// Rather than building a RAM buffer - we don't have much RAM -
// we can hardcode our digit renderers. And rather than overlaying
// it, and double-rendering and introducing flicker, it's far quicker
// to do all rendering in one pass. Without memory for a frame buffer,
// we therefore need to do this in our hardcoded write function.
// Either that, or we use 10% of the chip's memory as a row or column
// buffer, and write only to that.

const char charNeg = 10;
const char charDot = 11;
const char charSpace = 15;

char characters[6*7] = {
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
        charSpace, charSpace,charSpace,charSpace,charSpace,charSpace,
};

void printToBuffer(char row, char colourIndex, float value) {
    char colourBits = colourIndex << 4;
    char offset = row * 6;
    int16_t ivalue = value * 100; // work with ints
    characters[offset + 0] = colourBits | (ivalue < 0 ? charNeg : charSpace);
    if (ivalue < 0) {
        ivalue = -ivalue;
    }
    if (ivalue > 9999) {
        ivalue = ivalue % 10000;
    }
    characters[offset + 5] = colourBits | (ivalue % 10);
    ivalue /= 10;
    characters[offset + 4] = colourBits | (ivalue % 10);
    ivalue /= 10;
    characters[offset + 3] = charDot;
    characters[offset + 2] = colourBits | (ivalue == 0 ? charSpace : ivalue % 10);
    ivalue /= 10;
    characters[offset + 1] = colourBits | (ivalue == 0 ? charSpace : ivalue % 10);
}

// 16bit colours are 5 bits red, 6 bits green, 5 bits blue
// 116, 203, 255
const uint16_t blue = 0x733F; // 01110 011001 11111
// 179, 121, 41
const uint16_t brown = 0x63C5; // 01100 011110 00101

uint16_t colourBuffer[16] = {};

//  xx   x    xx  xxx  x x  xxxx  xxx xxxx  xx   xx
// x  x xx   x  x    x x x  x    x       x x  x x  x
// x  x  x     x    x  xxxx xxx  xxx    x   xx   xxx xxxx
// x  x  x    x      x   x     x x  x  x   x  x    x
//  xx  xxx  xxxx xxx    x  xxx   xx  x     xx    x        xx
// 01100111001111011100001
// E11E02F00029520115A074F
uint8_t vfont[] = {
        0x0E, 0x11, 0x11, 0x0E, // 0
        0x12, 0x1F, 0x10, 0x00, // 1
        0x12, 0x19, 0x15, 0x12, // 2
        0x11, 0x11, 0x15, 0x0A, // 3
        0x07, 0x04, 0x1F, 0x04, // 4
        0x17, 0x15, 0x15, 0x09, // 5
        0x0E, 0x15, 0x15, 0x09, // 6
        0x11, 0x09, 0x05, 0x03, // 7
        0x0A, 0x15, 0x15, 0x0A, // 8
        0x02, 0x05, 0x15, 0x0E, // 9
        0x04, 0x04, 0x04, 0x04, // -
        0x00, 0x10, 0x10, 0x00, // .
};
uint16_t colours[] = { WHITE, CYAN, YELLOW, RED };
void blitCharacterVSlice(char packedCharacter, char xSlice, char bufferOffset) {
    char character = packedCharacter & 0x0F;
    if (character == charSpace || xSlice == 4)
        return;
    uint16_t c = colours[packedCharacter >> 4];
    char slice = character * 4 + xSlice;
    uint16_t *p = &colourBuffer[bufferOffset];
    uint8_t fontslice = vfont[slice];
    for (char i = 0; i < 5; i++) {
        if (fontslice & 0x01) {
            p[0] = c;
        }
        if (fontslice & 0x02) {
            p[1] = c;
        }
        if (fontslice & 0x04) {
            p[2] = c;
        }
        if (fontslice & 0x08) {
            p[3] = c;
        }
        if (fontslice & 0x10) {
            p[4] = c;
        }
    }
}

//               56          60          64                70    72
char target[] = { 0, 0, 0, 0, 0, 0, 1, 2, 3, 2, 1, 0, 0, 0, 0, 0, 0 };

#define AVR_WRITESPI(x) for (SPDR = (x); (!(SPSR & _BV(SPIF)));)

void _writePixels(uint16_t *colors, uint32_t len) {
    uint8_t *buf = (uint8_t *) colors;
    uint8_t *end = buf + (len << 1);
    while (buf < end) {
        AVR_WRITESPI(buf[1]);
        AVR_WRITESPI(*buf);
        buf += 2;
    }
}

// This is relatively fast.
void drawHorizonVWritePixels(int16_t x, int16_t yintercept) {
    bool hasCharsX = x < 6 * 5;
    char charCol = x / 5;
    char xBit = x % 5;
    for (int band = 0; band < 8; band++) {
        int ystart = band * 16;
        int yend = ystart + 16;
        tft.setAddrWindow(x, ystart, 1, 16);
        int y = 0;
        while (y < yintercept && y < 16) {
            colourBuffer[y++] = blue;
        }
        while (y >= yintercept && y < 16) {
            colourBuffer[y++] = brown;
        }
        yintercept -= 16;

        if (hasCharsX && ystart < 7 * 8) {
            char charRow = ystart / 8;
            char charIndex = charCol + charRow * 6;
            char character = characters[charIndex];
            blitCharacterVSlice(character, xBit, 0);
            if (charRow < 6) {
                character = characters[charIndex + 6];
                blitCharacterVSlice(character, xBit, 8);
            }
        }

        // Overlay --v--
        // xxxxxx     xxxxxx
        //       x   x
        //        x x
        //         x
        if (band == 4 && x >= 56 && x <= 72) {
            colourBuffer[target[x - 56]] = YELLOW;
        }

        // tft.writePixels(colourBuffer, 16, true, false);
        _writePixels(colourBuffer, 16);
    }
}

int16_t test = 0;
uint16_t lastFrame = millis();

typedef struct {
    float x, y, z;
} Vector;

#define vecLengthSq(v) ((v).x*(v).x + (v).y*(v).y + (v).z*(v).z)
#define vecLength(v) (sqrtf(vecLengthSq(v)))

//float vecLength(Vector v) {
//    return sqrtf(vecLengthSq(v));
//}

void vecNormalise(Vector &v) {
    float length = vecLength(v);
    v.x /= length;
    v.y /= length;
    v.z /= length;
}

Vector vecCrossProduct(Vector &v1, Vector &v2) {
    Vector result = {};
    result.x = v1.y*v2.z - v1.z*v2.y;
    result.y = v1.z*v2.x - v1.x*v2.z;
    result.z = v1.x*v2.y - v1.y*v2.x;
    return result;
}

#define dotProduct(v1, v2) ((v1).x*(v2).x + (v1).y*(v2).y + (v1).z*(v2).z)

Vector vecForward = { .x = -1, .y = 0, .z = 0 };

//float dotProduct(Vector v1, Vector v2) {
//    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
//}

// See https://create.arduino.cc/projecthub/MinukaThesathYapa/arduino-mpu6050-accelerometer-f92d8b
// For alternate MPU6050 code.

void drawHorizon() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    // printToBuffer(3, 1, g.gyro.pitch);
    printToBuffer(0, 0, a.acceleration.x);
    printToBuffer(1, 0, a.acceleration.y);
    printToBuffer(2, 0, a.acceleration.z);
    printToBuffer(6, 2, temp.temperature);

    uint16_t now = millis();
    float fps = 1000.0f / (now - lastFrame); // (now - lastFrame) / 100.0f; //
    printToBuffer(5, 1, fps);
    lastFrame = now;

    // Transaction
    tft.startWrite();

    // Build a vector to "down", and normalise it, ie 0,0,1
    // given our forward vector is -1,0,0 (depends on sensor orientation),
    // cross product of forward and down should correlate with horizon slope?
    // and angle between forward and down gives horizon offset?
    // dot product of forward and down gives us the cos of the angle between them
    Vector accel = { .x = a.acceleration.x, .y = a.acceleration.y, .z = a.acceleration.z };
    vecNormalise(accel);
    float cosPitch = dotProduct(accel, vecForward);
    Vector vecSide = vecCrossProduct(accel, vecForward);
    float m = vecSide.z / vecSide.y;

    printToBuffer(3, 1, m);
    printToBuffer(4, 1, cosPitch);

    // Find the equation of a line which expresses the horizon.
    // Using slope-intercept y = mx + c. Note that we will need
    // special handling to cope with 90 degrees, since m will be INF
    // float m = 0;
    // float c = 0 + test % 128 - 64;
    float c = -70 * cosPitch;
    int16_t y = 0;
    test ++;



    // Draw the horizon - brown below the line, blue above.
    for (int16_t x = 0; x < 128; x++) {
        y = m * (x - 64) + c + 64;
        drawHorizonVWritePixels(x, y);
    }

    tft.endWrite();
}

void loop() {
    if (hasMpu == 1) {
        drawHorizon();
        //testMpu6050();
        //delay(100);
    }
}