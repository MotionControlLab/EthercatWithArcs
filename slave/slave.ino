
#include "EasyCAT.h"
#include <SPI.h>
#include <Servo.h>

EasyCAT easyCAT;
Servo servo;

void setup()
{
    servo.attach(3);

    if (not easyCAT.Init())
    {
        // 初期化失敗
        for (;;)
            digitalWrite(13, millis() % 200 > 100);
    }
}

void loop()
{
    easyCAT.MainTask();

    // Master -> サーボ
    int angle;
    memcpy(&angle, easyCAT.BufferOut.Byte, sizeof angle);
    servo.write(angle);

    // つまみ -> Master
    int vol = analogRead(0);
    memcpy(easyCAT.BufferIn.Byte, &vol, sizeof vol);

    delay(10);
}
