#include "aircraft.h"
#include <EEPROM.h>

// There's an interrupt collision with the IR routines and the PWM
// Outputs 1, 5, 6 can be dimmed
// Outputs 2, 3, 7 cannont be dimmed
// Outputs 4, 8 and 13 are digital, cannot be dimmed
//
// Output 8 is the red led on the board
// Output 13 is the blue led on the board

// Output mapping to lights/motors
// Lights:
// 
//  Output types: (d)  = dimable, (nd) = not dimable
//
//  01 Identification (d):
//     Mid-Fuselete Bottom Identification (3)
// 
//  05 Position (nd):
//     Wing Tips (2)
//     Tail (1)
// 
//  06 Formation (d):
//     Wing Top (6)
//     Fuselage Top (3)
// 
//  04 Illumination (nd):
//     Wheel Wells (3)
// 
//  02 Landing (nd):
//     Wing Bottom Retractable Landing Lights (2)
// 
// Motors:
//  03 Retractable Landing Light Motor Up (nd)
//     Motor Up (1)
//  07 Retractable Landing Light Motor Down (nd)
//     Motor Down (1)
// 
// Others:
//  No channel needed yet:
//     Interior
//     Tail Illumination

UpDownMotor upDownMotor = UpDownMotor();

// Decay settings for position lights
// Note: Added .01 sec to on & decay lengths so will not be in sync with F-16
uint32_t positionOnLengths[1]          =    {110}; // On for .11 seconds
uint32_t positionDecayLengths[1]       =   {1110}; // Decay for 1.11
uint8_t  positionMaxLightLevels[1]     =     {ON}; // Full on when on  
uint32_t positionTauInMilliseconds[1]  =    {175}; // Half-life = .05 seconds

// Decay settings for taxi lights during day and night
uint32_t   landingDayOnLengths[1]      = {300000}; // On 5 minutes
uint32_t   landingDayDecayLengths[1]   = { 60000}; // Off for 1 minute (changed 7/18/2016, was 5 min)
uint8_t    landingDayMaxLightLevels[1] =     {ON}; // Full on when on  
uint32_t * landingDayTauInMilliseconds =     NULL; // On/Off, no decay

// Light objects to control each channel
Light      ident    ; // Identification: Mid-Fuselete Bottom Identification (3)
DecayLight position ; // Position      : Wing Tips (2), Tail (1)
Light      formation; // Formation     : Wing Top (6), Fuselage Top (3)
DecayLight landing  ; // Landing       : Wing Bot Retractable Landing Lights (2)
Light      illum    ; // Illumination  : Wheel Wells (3)

void serialPrintBanner() {
    Serial.println(F("NMNSH B-29 Lighting Controller v1.0"));
}

float getBatteryLowValue() {
  // Provide a non-default value if needed.  Default is BATTERYLOWDEFAULT
  // When voltage drops at or below this value, mode will switch to MODE_BATTERYLOW
  return BATTERYLOWDEFAULT;
}

float getBatteryLowResetValue() {
  // Provide a non-default value if needed.  Default is BATTERYLOWRESETDEFAULT
  // Will switch out of MODE_BATTERYLOW only after voltage rises at or above this value
  return BATTERYLOWRESETDEFAULT;
}

bool overrideBatteryLow() {
  // If either of these is on, battery may read as low, so don't check battery voltage.
  return hw.o3 || hw.o7;
}

void allLightsOn() {
  ident.on();
  landing.on();
  illum.on();
  position.on();
  formation.on();
}

void allLightsOff() {
  ident.off();
  landing.off();
  illum.off();
  position.off();
  formation.off();
}

void updateAll() {
  ident.update();
  landing.update();
  illum.update();
  position.update();
  formation.update();
  upDownMotor.motorUpdate();
}

void allOff() {
  upDownMotor.motorStop();
  allLightsOff();
}

// -------------------- Time of Day Settings ----------------
void setEvening() {
  ident.on();
  landing.on();
  //  illum.on();
  illum.off();
  position.flash();
  formation.on();
}

void setNight() {
  allOff();
}

void setPreDawn() {
  ident.on();
  landing.on();
  //  illum.on();
  illum.off();
  position.flash();
  formation.on();
}

void setMorning() {
  ident.on();
  landing.on();
  //  illum.on();
  illum.off();
  position.flash();
  formation.on();
}

void setDay() {
  ident.on();
  landing.flash();
  illum.off();
  position.flash();
  formation.on();
}

void processKey(const uint32_t key) {
  Serial.print(F("key "));
  Serial.println(key, HEX);
  switch (key) {
  case '0':
  case RC65X_KEY0:
  case RC65X_KEYDOWN: // Control wheel down
  case RM_YD065_KEY0:
  case RM_YD065_KEYDOWN:
    Serial.print(F("Got remote \"0\"\n"));
    setToMode(MODE_OVERRIDE);
    allOff();
    break;
  case '1':
  case RC65X_KEY1:
  case RM_YD065_KEY1:
    Serial.print(F("Got remote \"1\"\n"));
    setToMode(MODE_OVERRIDE);
    ident.toggle();
    break;
  case '2':
  case RC65X_KEY2:
  case RM_YD065_KEY2:
    Serial.print(F("Got remote \"2\"\n"));
    setToMode(MODE_OVERRIDE);
    landing.toggle();
    break;
//    case '3':
//    case RC65X_KEY3:
//    case RM_YD065_KEY3:
//        hw.o3Toggle();
//        break;
  case '4':
  case RC65X_KEY4:
  case RM_YD065_KEY4:
    Serial.print(F("Got remote \"4\"\n"));
    setToMode(MODE_OVERRIDE);
    illum.toggle();
    break;
  case '5':
  case RC65X_KEY5:
  case RM_YD065_KEY5:
    Serial.print(F("Got remote \"5\"\n"));
    setToMode(MODE_OVERRIDE);
    position.toggle();
    break;
  case '6':
  case RC65X_KEY6:
  case RM_YD065_KEY6:
    Serial.print(F("Got remote \"6\"\n"));
    setToMode(MODE_OVERRIDE);
    formation.toggle();
    break;
//    case '7':
//    case RC65X_KEY7:
//    case RM_YD065_KEY7:
//        hw.o7Toggle();
//        break;
  case 'B':
  case RC65X_KEYRED:
  case RM_YD065_KEYRED:
    Serial.print(F("Got remote \"B\"\n"));
    setToMode(MODE_BATTERYLOW);
    break;
  case '8':
  case RC65X_KEY8:
  case RC65X_KEYUP: // Control wheel up
  case RM_YD065_KEY8:
  case RM_YD065_KEYUP:
    Serial.print(F("Got remote \"8\"\n"));
    setToMode(MODE_OVERRIDE);
    allLightsOn();
    break;
  case 'U':
  case RC65X_KEYCHANUP:
  case RM_YD065_KEYVOLUMEUP:
  case RM_YD065_KEYPROGUP:
    Serial.print(F("Got remote \"U\"\n"));
    upDownMotor.motorUpStart();
    break;
  case 'D':
  case RC65X_KEYCHANDOWN:
  case RM_YD065_KEYVOLUMEDOWN:
  case RM_YD065_KEYPROGDOWN:
    Serial.print(F("Got remote \"D\"\n"));
    upDownMotor.motorDownStart();
    break;
  case 'P':
  case RC65X_KEYPLAY:
  case RC65X_KEYSELECT:
  case RM_YD065_KEYPLAY:
  case RM_YD065_KEYOK:
    Serial.print(F("Got remote \"P\"\n"));
    const uint16_t lightLevel = hw.photocell2();
    const TimeOfDay::DayPart dayPart = timeOfDay.updateAverage(lightLevel);
    setToMode(dayPart);
    break;
  }
}

void serialPrintCustomStatus()
{
  //                             1      2        3    4      5         6
  serialPrintCustomStatusDefault(&ident,&landing,NULL,&illum,&position,&formation,
                                 NULL);
  //                             7
}

void setupLightingAndMotorChannels()
{
  ident     .setup(hw.o1, ON);
  landing   .setup(hw.o2, ON, 1, landingDayOnLengths, landingDayDecayLengths,
                   landingDayMaxLightLevels, landingDayTauInMilliseconds);
  illum     .setup(hw.o4, ON);
  position  .setup(hw.o5, ON, 1, positionOnLengths, positionDecayLengths,
                   positionMaxLightLevels, positionTauInMilliseconds);
  formation .setup(hw.o6, ON);

  upDownMotor.setup(hw.o3, hw.o7); // Initialize with (up, down) outputs
}

