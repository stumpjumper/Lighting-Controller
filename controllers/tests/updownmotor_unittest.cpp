#include "updownmotor_unittest.h"

using ::testing::StrEq;
using ::testing::Matcher;

void UpDownMotorTest::SetUp()
{
  uint8_t p_oUpArrayTmp  [4] = {ON, ON , OFF, OFF};
  uint8_t p_oDownArrayTmp[4] = {ON, OFF, ON , OFF};

  for (uint8_t i = 0; i < 4; i++) {
    p_oUpArray  [i] = p_oUpArrayTmp  [i];
    p_oDownArray[i] = p_oDownArrayTmp[i];
  }

  p_oUp   = NULL;
  p_oDown = NULL;

  on  = ON ;
  off = OFF;
  p_on   = &on;
  p_off  = &off;
}

TEST_F(UpDownMotorTest, Setup) {
  for (uint8_t i = 0; i < 4; i++) {
    
    uint8_t oUp  ;
    uint8_t oDown;
    UpDownMotor udm = UpDownMotor();
    udm.setup(oUp, oDown);
    EXPECT_EQ(OFF, *udm.p_outputUp  );
    EXPECT_EQ(OFF, *udm.p_outputDown);
    oUp   = p_oUpArray[i];
    oDown = p_oDownArray[i];

    EXPECT_EQ(&oUp          , udm.p_outputUp        );
    EXPECT_EQ(&oDown        , udm.p_outputDown      );
    EXPECT_EQ(p_oUpArray[i]  , *udm.p_outputUp       ) << "oUp = " << int(oUp) << ",p_oUpArray[" << int(i) << "] = " << int(p_oUpArray[i]);
    EXPECT_EQ(p_oDownArray[i], *udm.p_outputDown     ) << "oDown = " << int(oDown) << ", p_oDownArray[" << int(i) << "] = " << int(p_oDownArray[i]);
    EXPECT_FALSE(udm.inMotorUpMode     );
    EXPECT_FALSE(udm.inMotorDownMode   );
    EXPECT_EQ(0              , udm.motorUpStartTime  );
    EXPECT_EQ(0              , udm.motorDownStartTime);
  }
}

TEST_F(UpDownMotorTest, GetInMotorUpDownMode) {
    uint8_t oUp  ;
    uint8_t oDown;
    UpDownMotor udm = UpDownMotor();
    udm.setup(oUp, oDown);

    udm.inMotorUpMode = true;
    udm.inMotorDownMode = true;
    EXPECT_TRUE(udm.getInMotorUpMode()   );
    EXPECT_TRUE(udm.getInMotorDownMode() );

    udm.inMotorUpMode = false;
    udm.inMotorDownMode = false;
    EXPECT_FALSE(udm.getInMotorUpMode()   );
    EXPECT_FALSE(udm.getInMotorDownMode() );

}

TEST_F(UpDownMotorTest, MotorUpStop) {
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t oUp  ;
    uint8_t oDown;
    UpDownMotor udm = UpDownMotor();
    udm.setup(oUp, oDown);
    EXPECT_EQ(OFF, *udm.p_outputUp  );
    EXPECT_EQ(OFF, *udm.p_outputDown);
    oUp   = p_oUpArray[i];
    oDown = p_oDownArray[i];

    udm.motorUpStop();
    EXPECT_EQ(OFF  , *udm.p_outputUp);
    EXPECT_FALSE(udm.inMotorUpMode );
    EXPECT_FALSE(udm.inMotorDownMode );
  }
}

TEST_F(UpDownMotorTest, MotorDownStop) {
  for (uint8_t i = 0; i < 4; i++) {
    
    uint8_t oUp  ;
    uint8_t oDown;
    UpDownMotor udm = UpDownMotor();
    udm.setup(oUp, oDown);
    EXPECT_EQ(OFF, *udm.p_outputUp  );
    EXPECT_EQ(OFF, *udm.p_outputDown);
    oUp   = p_oUpArray[i];
    oDown = p_oDownArray[i];

    udm.motorDownStop();
    EXPECT_EQ(OFF  , *udm.p_outputDown);
    EXPECT_FALSE(udm.inMotorUpMode );
    EXPECT_FALSE(udm.inMotorDownMode );
  }
}

TEST_F(UpDownMotorTest, MotorUpStart) {
  ArduinoMock * arduinoMock = arduinoMockInstance();

  EXPECT_CALL(*arduinoMock, millis())
    .Times(1);

  uint32_t millisSet = 9283*60*60;
  arduinoMock->setMillisRaw(millisSet);

  bool inMotorUpModeArray[] = {false, true};

  uint8_t oUp  ;
  uint8_t oDown;
  UpDownMotor udm = UpDownMotor();
  udm.setup(oUp, oDown);
  EXPECT_EQ(OFF, *udm.p_outputUp  );
  EXPECT_EQ(OFF, *udm.p_outputDown);
  oUp   = ON;
  oDown = OFF;
  
  for (uint8_t i = 0; i < 2; i++) {
    udm.inMotorUpMode = inMotorUpModeArray[i];

    udm.motorUpStart();
    EXPECT_EQ(millisSet, udm.motorUpStartTime);
    EXPECT_TRUE(udm.inMotorUpMode);
  }

  releaseArduinoMock();
}

TEST_F(UpDownMotorTest, MotorDownStart) {
  ArduinoMock * arduinoMock = arduinoMockInstance();

  EXPECT_CALL(*arduinoMock, millis())
    .Times(1);

  uint32_t millisSet = 9283*60*60;
  arduinoMock->setMillisRaw(millisSet);

  bool inMotorDownModeArray[] = {false, true};

  uint8_t oUp  ;
  uint8_t oDown;
  UpDownMotor udm = UpDownMotor();
  udm.setup(oUp, oDown);
  EXPECT_EQ(OFF, *udm.p_outputUp  );
  EXPECT_EQ(OFF, *udm.p_outputDown);
  oUp   = ON;
  oDown = OFF;

  for (uint8_t i = 0; i < 2; i++) {
    udm.inMotorDownMode = inMotorDownModeArray[i];

    udm.motorDownStart();
    EXPECT_EQ(millisSet, udm.motorDownStartTime);
    EXPECT_TRUE(udm.inMotorDownMode);
  }

  releaseArduinoMock();
}

TEST_F(UpDownMotorTest, MotorUpUpdate) {

  // States:
  // 1) Not in motor up  mode
  //    p_outputUp = OFF
  // 2) In motor up mode
  //    a) Before start deley
  //       inMotorUpMode = true
  //       p_outputUp = OFF
  //    b) Before up/down timeout
  //       inMotorUpMode = true
  //       p_outputUp = ON
  //    c) After up/down timeout
  //       inMotorUpMode = false
  //       p_outputUp = OFF

  ArduinoMock * arduinoMock = arduinoMockInstance();

  EXPECT_CALL(*arduinoMock, millis())
    .Times(testing::AtLeast(1));

  uint32_t startTime = 9283*60*60;
  uint8_t oUp  ;
  uint8_t oDown;
  UpDownMotor udm = UpDownMotor();
  udm.setup(oUp, oDown);
  EXPECT_EQ(OFF, *udm.p_outputUp  );
  EXPECT_EQ(OFF, *udm.p_outputDown);
  oUp   = OFF;
  oDown = OFF;

  udm.inMotorUpMode = false;
  udm.motorUpStartTime = startTime;
  
  // State (1)
  udm.motorUpUpdate();
  EXPECT_EQ(oUp, *udm.p_outputUp);
  EXPECT_EQ(OFF, oUp);
  EXPECT_FALSE(udm.inMotorUpMode);

  // State (2a)
  udm.inMotorUpMode = true;
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEMOTORDELAY/2);
  udm.motorUpUpdate();
  EXPECT_EQ(oUp, *udm.p_outputUp);
  EXPECT_EQ(OFF, oUp);
  EXPECT_TRUE(udm.inMotorUpMode);

  // State (2b)
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEMOTORDELAY + 1);
  udm.motorUpUpdate();
  EXPECT_EQ(oUp, *udm.p_outputUp);
  EXPECT_EQ(ON, oUp);
  EXPECT_TRUE(udm.inMotorUpMode);

  // State (2c)
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEOUTMOTORUPDOWN + 1);
  udm.motorUpUpdate();
  EXPECT_EQ(oUp, *udm.p_outputUp);
  EXPECT_EQ(OFF, oUp);
  EXPECT_FALSE(udm.inMotorUpMode);

  releaseArduinoMock();
}

TEST_F(UpDownMotorTest, MotorDownUpdate) {

  // States:
  // See MotorUpUpdate test

  ArduinoMock * arduinoMock = arduinoMockInstance();

  EXPECT_CALL(*arduinoMock, millis())
    .Times(testing::AtLeast(1));


  uint32_t startTime = 9283*60*60;
  uint8_t oUp  ;
  uint8_t oDown;
  UpDownMotor udm = UpDownMotor();
  udm.setup(oUp, oDown);
  EXPECT_EQ(OFF, *udm.p_outputUp  );
  EXPECT_EQ(OFF, *udm.p_outputDown);
  oUp   = OFF;
  oDown = OFF;

  udm.inMotorDownMode = false;
  udm.motorDownStartTime = startTime;
  
  // State (1)
  udm.motorDownUpdate();
  EXPECT_EQ(oDown, *udm.p_outputDown);
  EXPECT_EQ(OFF, oDown);
  EXPECT_FALSE(udm.inMotorDownMode);

  // State (2a)
  udm.inMotorDownMode = true;
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEMOTORDELAY/2);
  udm.motorDownUpdate();
  EXPECT_EQ(oDown, *udm.p_outputDown);
  EXPECT_EQ(OFF, oDown);
  EXPECT_TRUE(udm.inMotorDownMode);

  // State (2b)
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEMOTORDELAY + 1);
  udm.motorDownUpdate();
  EXPECT_EQ(oDown, *udm.p_outputDown);
  EXPECT_EQ(ON, oDown);
  EXPECT_TRUE(udm.inMotorDownMode);

  // State (2c)
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEOUTMOTORUPDOWN + 1);
  udm.motorDownUpdate();
  EXPECT_EQ(oDown, *udm.p_outputDown);
  EXPECT_EQ(OFF, oDown);
  EXPECT_FALSE(udm.inMotorDownMode);

  releaseArduinoMock();
}


TEST_F(UpDownMotorTest, MotorUpdate) {
  ArduinoMock * arduinoMock = arduinoMockInstance();
  SerialMock  * serialMock  = serialMockInstance();

  EXPECT_CALL(*arduinoMock, millis())
    .Times(testing::AtLeast(1));


  EXPECT_CALL(*serialMock, println(Matcher<const char *>(StrEq("ERROR: In UpDownMotor::motorUpdate() found ((inMotorUpMode && inMotorDownMode) || (*p_outputUp && *p_outputDown))"))))
    .Times(2);
  EXPECT_CALL(*serialMock, println(Matcher<const char *>(StrEq("       Calling motorUpStop() and motorDownStop()"))))
    .Times(2);

  uint32_t startTime = 9283*60*60;
  uint8_t oUp  ;
  uint8_t oDown;
  UpDownMotor udm = UpDownMotor();
  udm.setup(oUp, oDown);
  EXPECT_EQ(OFF, *udm.p_outputUp  );
  EXPECT_EQ(OFF, *udm.p_outputDown);
  oUp   = OFF;
  oDown = OFF;

  // Error: In both motor up and motor down mode:
  udm.inMotorUpMode   = true;
  udm.inMotorDownMode = true;
  oUp   = OFF;
  oDown = OFF;
  udm.motorUpdate(); // Will cause above errors to be printed

  // Error: Both motor channels on
  udm.inMotorUpMode   = true;
  udm.inMotorDownMode = false;
  oUp   = ON;
  oDown = ON;
  udm.motorUpdate(); // Will cause above errors to be printed

  // Case: Motor up
  oUp   = OFF;
  oDown = OFF;
  udm.inMotorUpMode   = true;
  udm.inMotorDownMode = false;
  arduinoMock->setMillisRaw(startTime);
  udm.motorUpStartTime = millis();
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEOUTMOTORUPDOWN/2);
  udm.motorUpdate();
  EXPECT_EQ(oUp, *udm.p_outputUp);
  EXPECT_EQ(oDown, *udm.p_outputDown);
  EXPECT_EQ(ON, oUp);
  EXPECT_EQ(OFF, oDown);
  EXPECT_TRUE(udm.inMotorUpMode);
  EXPECT_FALSE(udm.inMotorDownMode);

  // Case: Motor down
  oUp   = OFF;
  oDown = OFF;
  udm.inMotorUpMode   = false;
  udm.inMotorDownMode = true;
  arduinoMock->setMillisRaw(startTime);
  udm.motorDownStartTime = millis();
  arduinoMock->setMillisRaw(startTime + LUCKY7_TIMEOUTMOTORUPDOWN/2);
  udm.motorUpdate();
  EXPECT_EQ(oUp, *udm.p_outputUp);
  EXPECT_EQ(oDown, *udm.p_outputDown);
  EXPECT_EQ(OFF, oUp);
  EXPECT_EQ(ON, oDown);
  EXPECT_FALSE(udm.inMotorUpMode);
  EXPECT_TRUE(udm.inMotorDownMode);


  releaseSerialMock();
  releaseArduinoMock();
}

