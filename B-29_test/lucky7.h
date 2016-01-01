#ifndef LUCKY7_H
#define LUCKY7_H
#include "Arduino.h"
#include "Serial.h"

#define OFF 0
#define ON  255

// PWM outputs
#define O1  11
#define O2  10
#define O3  9
#define O5  6
#define O6  5
#define O7  3

#define O4  7      
#define O8  8
#define O13 13

#define IR  2

#define AVECNT 10

// ((5/1024)/10000)*43000 ideal
// 12.16/553
#define BVSCALE 0.02198915009


#define LUCKY7_TIMEOUTMOTORUPDOWN    45000  // 45 sec in milliseconds
#define LUCKY7_TIMEMOTORDELAY         2000  // 2 sec
#define LUCKY7_TIME30SEC             30000  // 30 sec
#define LUCKY7_TIME5MIN             300000  // 300 sec = 5 min
#define LUCKY7_TIME2HOUR           7200000U // 2 hours
#define LUCKY7_TIME4HOUR          14400000U // 4 hours
#define LUCKY7_TIME12HOUR         43200000U // 12 hours

class Lucky7;

class Light
{
private:
  FRIEND_TEST(OnOffLight, Constructor);
  FRIEND_TEST(OnOffLight, On);
  FRIEND_TEST(OnOffLight, Off);
  FRIEND_TEST(OnOffLight, FunctionCallOperatorGetValue);
  FRIEND_TEST(OnOffLight, FunctionCallOperatorSetValue);
  FRIEND_TEST(OnOffLight, Resume);
  FRIEND_TEST(OnOffLight, Update);

  // Do not implement to make sure are never called
  Light(Light & other); 
  Light & operator=(const Light &rhs);

protected:
  uint8_t lightLevel;
  bool paused;

public:
  Light();
  virtual ~Light();
  
  bool getPaused() {return paused;};
  
  void on() {lightLevel = ON; paused = true;};
  void off() {lightLevel = OFF; paused = true;};
  void resume() {paused = false;};
  virtual void update() = 0;

  uint8_t & operator()(void) {return lightLevel;};
  //  Light & operator=(const uint8_t value) {lightLevel = value; return *this;};


};

class OnOffLight : public Light
{
public:
  OnOffLight() : Light() {;};
  void update() {;};

};

class BlinkingLight : public Light
{
private:
  FRIEND_TEST(BlinkingLight, Constructor);
  FRIEND_TEST(BlinkingLight, Update);
  FRIEND_TEST(FastBlinkingLight, Constructor);
  FRIEND_TEST(SlowBlinkingLight, Constructor);
  
  BlinkingLight(); // Do not implement

protected:
  uint32_t onLength;
  uint32_t offLength;
  uint32_t changeTime;
  uint8_t  maxLightLevel;
  
public:
  BlinkingLight(uint32_t onLengthValue,
                uint32_t offLengthValue,
                uint8_t  maxLightLevelValue);
  void update();
};

class FastBlinkingLight : public BlinkingLight
{
private:
  FastBlinkingLight(); // Do not implement
  
public:
  FastBlinkingLight(uint8_t maxLightLevelValue);
};

class SlowBlinkingLight : public BlinkingLight
{
private:
  SlowBlinkingLight(); // Do not implement
  
public:
  SlowBlinkingLight(uint8_t maxLightLevelValue);
};

class DecayLight: public Light
{

// see F16 Light Dimming Plot.ods
// lightLevel = a+b*exp(d*t/e)

}

class TimeOfDay
{
public:

 enum DayPart {
   EVENING    = 'E', // Light level below threshhold, EVENING timer started, nightStart set.
   NIGHT      = 'N', // Light level below threshhold and EVENING timed out
   PREDAWN    = 'P', // Starts at (nightStart + lengthOfNight - predawnLength)
   MORNING    = 'M', // Light level above threshhold, MORNING timer started, dayStart set
   DAY        = 'D', // Light level above threshhold and MORNING timed out
  } ;
  
  void setup(uint16_t initialValueMin, uint16_t initialValueMax,
        uint8_t nightDayThresholdPercentageValue);

  DayPart updateAverage(const uint16_t lightLevel);
  DayPart getDayPart();
  uint16_t getNightDayThreshold();

  uint16_t getPhotocellAvgValueMin() {return photocellAvgValueMin;};
  uint16_t getPhotocellAvgValueMax() {return photocellAvgValueMax;};
  uint16_t getPhotocellAvgValueCurrent() {return photocellAvgValueCurrent;};

  // Primarily used to get access to private variables for testing


private:
  FRIEND_TEST(TimeOfDayTest, setup);
  FRIEND_TEST(TimeOfDayTest, Constructor);
  FRIEND_TEST(TimeOfDayTest, getNightDayThreshold);
  FRIEND_TEST(TimeOfDayTest, UpdatePhotocellAvgValues);
  FRIEND_TEST(TimeOfDayTest, UpdateTimeOfDay);

  uint16_t photocellAvgValueCurrent;
  uint16_t photocellAvgValueMin;
  uint16_t photocellAvgValueMax;
  uint8_t  nightDayThresholdPercentage;
  uint32_t lengthOfNight;
  uint32_t nightStart;
  uint32_t dayStart;
  uint32_t update30secTimeout;
  uint32_t update5minTimeout;
  uint32_t eveningLength;
  uint32_t morningLength;
  uint32_t predawnLength;
  

#define PHOTOCELLVALUESSIZE LUCKY7_TIME5MIN/LUCKY7_TIME30SEC
  uint16_t photocellValues[PHOTOCELLVALUESSIZE];
  uint8_t  photocellValuesIndex;

  void updatePhotocellAvgValues(uint16_t photocellAvgValue);
  void updateTimeOfDay();
  DayPart currentDayPart;
  
};

class UpDownMotor
{
private:
  FRIEND_TEST(UpDownMotorTest, Setup);
  FRIEND_TEST(UpDownMotorTest, MotorUpStop);
  FRIEND_TEST(UpDownMotorTest, MotorDownStop);
  FRIEND_TEST(UpDownMotorTest, MotorUpStart);
  FRIEND_TEST(UpDownMotorTest, MotorDownStart);
  FRIEND_TEST(UpDownMotorTest, MotorUpUpdate);
  FRIEND_TEST(UpDownMotorTest, MotorDownUpdate);
  FRIEND_TEST(UpDownMotorTest, MotorUpdate);

  uint8_t * p_outputUp;
  uint8_t * p_outputDown;

  bool inMotorUpMode;
  bool inMotorDownMode;

  uint32_t motorUpStartTime;
  uint32_t motorDownStartTime;
  
  void motorUpUpdate();
  void motorDownUpdate();
  void motorUpStop();
  void motorDownStop();


public:
  void setup(uint8_t * p_oUp, uint8_t * p_oDown);

  void motorUpStart();
  void motorDownStart();

  uint8_t getMotorUpPower()   {return *p_outputUp;};
  uint8_t getMotorDownPower() {return *p_outputDown;};

  void motorUpdate();

  void motorStop() {motorUpStop(); motorDownStop();};
};

class  IRrecvMock;
class  decode_results;


class Lucky7
{
private:
  FRIEND_TEST(Lucky7Test, Setup);
  FRIEND_TEST(Lucky7Test, SaveOutputState);
  FRIEND_TEST(Lucky7Test, SetOutputStateFromSaved);
  FRIEND_TEST(Lucky7Test, Loop);
  FRIEND_TEST(Lucky7Test, Photocell1and2andBatteryVoltage);
  FRIEND_TEST(Lucky7Test, OutputMoveTo);

  uint32_t irTimeout;
  uint16_t pc1[AVECNT], pc2[AVECNT], bc[AVECNT];
  uint8_t aveptr;
  void outputMoveTo(const uint8_t outputPin, uint8_t & currentValue,
                    uint8_t targetValue, const uint16_t stepDelay);

  uint8_t o1Saved,o2Saved,o3Saved,o4Saved,o5Saved,o6Saved,o7Saved;

public:

  enum BoardLightMode {
    LIGHT_ON,
    LIGHT_OFF,
    LIGHT_FAST_BLINK,
    LIGHT_SLOW_BLINK,
  };

  void boardLight(BoardLightMode mode, void (lightOn)(), void (lightOff)());

  uint8_t o1,o2,o3,o4,o5,o6,o7,o8,o13;


  void setup();

  uint32_t loop();
  uint32_t irLoop();

  void saveOutputState();
  void setOutputStateFromSaved();

  void o1On()             {o1 = ON;}
  void o1Set(uint8_t v)   {o1 = v;}
  void o1Off()            {o1 = 0;}
  void o1MoveTo(uint8_t v, const uint16_t stepdelay);
  void o1Toggle()         {o1 = o1?OFF:ON;};       
  
  void o2On()             {o2 = ON;}
  void o2Set(uint8_t v)   {o2 = v;}
  void o2Off()            {o2 = 0;}
  void o2MoveTo(uint8_t v, const uint16_t stepdelay);
  void o2Toggle()         {o2 = o2?OFF:ON;};       

  void o3On()             {o3 = ON;}
  void o3Set(uint8_t v)   {o3 = v;}
  void o3Off()            {o3 = 0;}
  void o3MoveTo(uint8_t v, const uint16_t stepdelay);
  void o3Toggle()         {o3 = o3?OFF:ON;};       

  void o4On()             {o4 = ON;}
  void o4Set(uint8_t v)   {o4 = v;}
  void o4Off()            {o4 = 0;}
  void o4MoveTo(uint8_t v, const uint16_t stepdelay);
  void o4Toggle()         {o4 = o4?OFF:ON;};       

  void o5On()             {o5 = ON;}
  void o5Set(uint8_t v)   {o5 = v;}
  void o5Off()            {o5 = 0;}
  void o5MoveTo(uint8_t v, const uint16_t stepdelay);
  void o5Toggle()         {o5 = o5?OFF:ON;};       

  void o6On()             {o6 = ON;}
  void o6Set(uint8_t v)   {o6 = v;}
  void o6Off()            {o6 = 0;}
  void o6MoveTo(uint8_t v, const uint16_t stepdelay);
  void o6Toggle()         {o6 = o6?OFF:ON;};       

  void o7On()             {o7 = ON;}
  void o7Set(uint8_t v)   {o7 = v;}
  void o7Off()            {o7 = 0;}
  void o7MoveTo(uint8_t v, const uint16_t stepdelay);
  void o7Toggle()         {o7 = o7?OFF:ON;};       

  void o8On()             {o8 = ON;}
  void o8Set(uint8_t v)   {o8 = v;}
  void o8Off()            {o8 = 0;}
  void o8MoveTo(uint8_t v, const uint16_t stepdelay);
  void o8Toggle()         {o8 = o8?OFF:ON;};       

  void o13On()            {o13 = ON;}
  void o13Set(uint8_t v)  {o13 = v;}
  void o13Off()           {o13 = 0;}
  void o13MoveTo(uint8_t v, const uint16_t stepdelay);
  void o13Toggle()        {o13 = o13?OFF:ON;};       

  uint16_t photocell1();
  uint16_t photocell2();

  float batteryVoltage();

};
#endif
