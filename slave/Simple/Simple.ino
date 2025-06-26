#include <SPI.h>

#include "EthercatReceiver.hh"
#include "EthercatSender.hh"

static EthercatBus Bus;


struct MasterToSlave
{
    float CurrentRef;
    bool ServoOn : 1;
} __attribute__((packed));

static EthercatReceiver<MasterToSlave> Receiver{ Bus };


struct SlaveToMaster
{
    int32_t Position;
    int32_t Velocity;
    int32_t Current;
    bool Error : 1;
} __attribute__((packed));

static EthercatSender<SlaveToMaster> Sender{ Bus };


void Assert(bool ExpectTrue)
{
    if (ExpectTrue)
        return;

    for (;;)
        digitalWrite(13, millis() % 200 > 100);
}

void setup()
{
    Assert(Bus.Init());
}

void loop()
{
    Bus.Update();
    
    const auto Data = Receiver.GetData();

    Sender.SetData({
        .Position = Data.CurrentRef,
        .Velocity = Data.CurrentRef,
        .Current = Data.CurrentRef,
        .Error = Data.ServoOn,
    });

    Serial.print(Data.CurrentRef), Serial.print('\t');
    Serial.print(Data.ServoOn), Serial.print('\n');


    delay(10);
}
