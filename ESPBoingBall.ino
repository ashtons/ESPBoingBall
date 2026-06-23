#include <TFT_eSPI.h>
#include <math.h>

const int TFT_BACKLIGHT_PIN = 4;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frame = TFT_eSprite(&tft);

const uint16_t COLOR_BG = TFT_BLACK;
const uint16_t COLOR_RED = TFT_RED;
const uint16_t COLOR_WHITE = TFT_WHITE;
const uint16_t COLOR_SHADOW = tft.color565(28, 28, 34);

const int BALL_RADIUS = 24;
const int BALL_DIAMETER = BALL_RADIUS * 2;
const int BALL_SEGMENTS = 10;
const float PI_F = 3.14159265f;

float ballX;
float ballY;
float velocityX = 2.4f;
float velocityY = 1.9f;
float spin = 1.0f;

int screenWidth;
int screenHeight;
unsigned long lastFrameMs = 0;

void drawBoingBall(TFT_eSprite &target, int centerX, int centerY, float rotation)
{
    const float lightX = -0.45f;
    const float lightY = -0.60f;
    const float lightZ = 0.66f;

    for (int y = -BALL_RADIUS; y <= BALL_RADIUS; y += 2)
    {
        for (int x = -BALL_RADIUS; x <= BALL_RADIUS; x += 2)
        {
            float nx = (float)x / BALL_RADIUS;
            float ny = (float)y / BALL_RADIUS;
            float r2 = nx * nx + ny * ny;

            if (r2 > 1.0f)
            {
                continue;
            }

            float nz = sqrtf(1.0f - r2);
            float longitude = atan2f(nx, nz) + rotation;
            float latitude = asinf(ny);

            int longitudeBand = (int)floorf((longitude + PI_F) / (2.0f * PI_F / BALL_SEGMENTS));
            int latitudeBand = (int)floorf((latitude + PI_F * 0.5f) / (PI_F / BALL_SEGMENTS));
            bool redPatch = ((longitudeBand + latitudeBand) & 1) == 0;

            float light = nx * lightX + ny * lightY + nz * lightZ;
            uint8_t shade = (uint8_t)constrain(150 + light * 105, 58, 255);

            uint16_t color;
            if (redPatch)
            {
                color = tft.color565(shade, shade / 6, shade / 7);
            }
            else
            {
                color = tft.color565(shade, shade, shade);
            }

            target.fillRect(centerX + x, centerY + y, 2, 2, color);
        }
    }

    target.drawCircle(centerX, centerY, BALL_RADIUS, TFT_DARKGREY);
}

void drawShadow(TFT_eSprite &target, int centerX, int centerY)
{
    int floorY = screenHeight - 16;
    int distanceFromFloor = max(0, floorY - (centerY + BALL_RADIUS));
    int shadowWidth = map(distanceFromFloor, 0, screenHeight, BALL_RADIUS + 22, BALL_RADIUS / 2);
    int shadowHeight = max(4, shadowWidth / 5);

    target.fillEllipse(centerX, floorY, shadowWidth, shadowHeight, COLOR_SHADOW);
}

void setup()
{
    // pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    // digitalWrite(TFT_BACKLIGHT_PIN, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COLOR_BG);

    screenWidth = tft.width();
    screenHeight = tft.height();

    frame.setColorDepth(16);
    frame.createSprite(screenWidth, screenHeight);

    ballX = screenWidth * 0.33f;
    ballY = screenHeight * 0.30f;
}

void loop()
{
    unsigned long now = millis();
    if (now - lastFrameMs < 16)
    {
        return;
    }
    lastFrameMs = now;

    ballX += velocityX;
    ballY += velocityY;
    velocityY += 0.075f;

    if (ballX <= BALL_RADIUS || ballX >= screenWidth - BALL_RADIUS)
    {
        ballX = constrain(ballX, BALL_RADIUS, screenWidth - BALL_RADIUS);
        velocityX = -velocityX;
    }

    if (ballY <= BALL_RADIUS || ballY >= screenHeight - BALL_RADIUS - 16)
    {
        ballY = constrain(ballY, BALL_RADIUS, screenHeight - BALL_RADIUS - 16);
        velocityY = -velocityY * 1.0f;
    }

    spin += velocityX * 0.025f;

    frame.fillSprite(COLOR_BG);
    drawShadow(frame, (int)ballX, (int)ballY);
    drawBoingBall(frame, (int)ballX, (int)ballY, spin);
    frame.pushSprite(0, 0);
}
