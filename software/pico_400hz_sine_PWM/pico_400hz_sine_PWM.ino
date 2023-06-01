/****************************************************************************************************************************
  PWM_Waveform_Fast.ino
  For RP2040 boards
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/RP2040_PWM
  Licensed under MIT license

  The RP2040 PWM block has 8 identical slices. Each slice can drive two PWM output signals, or measure the frequency
  or duty cycle of an input signal. This gives a total of up to 16 controllable PWM outputs. All 30 GPIO pins can be driven
  by the PWM block
*****************************************************************************************************************************/

// These define's must be placed at the beginning before #include "TimerInterrupt_Generic.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     4

// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "RPi_Pico_TimerInterrupt.h"
#include <math.h>

#ifndef LED_BUILTIN
  #define LED_BUILTIN       25
#endif

#define PIN_T0              4         // Pin D1 mapped to pin GPIO1 of RPI_PICO
#define PIN_T1              2         // Pin D1 mapped to pin GPIO1 of RPI_PICO

// This example to demo the new function setPWM_manual(uint8_t pin, uint16_t top, uint8_t div, uint16_t level, bool phaseCorrect = false)
// used to generate a waveform. Check https://github.com/khoih-prog/RP2040_PWM/issues/6

#define _PWM_LOGLEVEL_        2

#if ( defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
      defined(ARDUINO_GENERIC_RP2040) ) && defined(ARDUINO_ARCH_MBED)

#if(_PWM_LOGLEVEL_>3)
  #warning USING_MBED_RP2040_PWM
#endif

#elif ( defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
        defined(ARDUINO_GENERIC_RP2040) ) && !defined(ARDUINO_ARCH_MBED)

#if(_PWM_LOGLEVEL_>3)
  #warning USING_RP2040_PWM
#endif
#else
#error This code is intended to run on the RP2040 mbed_nano, mbed_rp2040 or arduino-pico platform! Please check your Tools->Board setting.
#endif

#include "RP2040_PWM.h"

#define LED_ON        LOW
#define LED_OFF       HIGH

#define pinLed        25    // On-board BUILTIN_LED
#define pin4          0     // PWM channel 0A
#define pin1          2     // PWM channel 1A
#define pin2          4     // PWM channel 2A
#define pin3          6     // PWM channel 3A
#define pin0          16    // PWM channel 4B
#define pin5          10    // PWM channel 5A
#define pin11         11    // PWM channel 5B
#define pin6          12    // PWM channel 6A
#define pin7          14    // PWM channel 7A


uint32_t PWM_Pins[]     = { 0, 1, 6, 7, 16, 19, 5 };

#define NUM_OF_PINS       ( sizeof(PWM_Pins) / sizeof(uint32_t) )

float dutyCycle[NUM_OF_PINS]  = { 50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f };

RP2040_PWM* PWM_Instance[NUM_OF_PINS];

#define NUM_PWM_POINTS      256

// PWM freq 143783 @ cpu 133000000
// 400 Hz * 360 degrees
uint16_t PWM_data_idle_top = 924;
uint8_t PWM_data_idle_div = 1;

uint16_t sine256[]  = {
  
  // 0 to 90    (0-63)
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,
  217,219,221,223,225,227,229,231,233,234,236,238,239,240,242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,
  // 90 to 180  (64-127)
  254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,221,219,
  217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,

  // 180 to 270 (128-191)
  127,124,121,118,115,111,108,105,102, 99, 96, 93, 90, 87, 84, 81, 78, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
   37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 16, 15, 14, 12, 11, 10,  9,  7,  6,  5,  5,  4,  3,  2,  2,  1,  1,  1,  0,  0,  0,
  // 270 to 360 (192-255)
    0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  5,  6,  7,  9, 10, 11, 12, 14, 15, 16, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
   37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76, 78, 81, 84, 87, 90, 93, 96, 99,102,105,108,111,115,118,121,124

};
bool buttonPress = false;
unsigned long buttonTime = 0; // To prevent debounce

float frequency;

// You can select any value
uint16_t PWM_data_idle = 124;

#define TIMER0_INTERVAL            48 //39

// Init RPI_PICO_Timer, can use any from 0-15 pseudo-hardware timers
RPI_PICO_Timer ITimer0(0);
//RPI_PICO_Timer ITimer1(1);


char dashLine[] = "=============================================================";
static int step256 = 0;

float target = 3.0;
float scale1;
float scale2;
float level1;
float level2;
float test;


bool TimerHandler0(struct repeating_timer *t)
{ 
  (void) t;
  
  static bool toggle0 = false;
  step256+=5;
  if(step256  > 255) step256 = 0; // should be >255, kludge to trim frequency to 400 Hz
// center resultant waveform around 50% PWM full scale is 100.0 * ( sine256[step256%256] / 256.0f )
//#if 0
  test = 100.0 * ( sine256[step256%256] / 256.0f );
//#else  
  level1 = ( 50.0 - scale1 ) + ( 2 * scale1 ) * ( sine256[step256%256] / 256.0f );
//#endif  
  level2 = ( 50.0 - scale2 ) + ( 2 * scale2 ) * ( sine256[step256%256] / 256.0f );

  //timer interrupt toggles pin LED_BUILTIN
  digitalWrite(PIN_T0, toggle0);
  toggle0 = !toggle0;

  digitalWrite(PIN_T1,step256==0);
  //uint16_t level2=1000/256*sine256[(index+80)%256];

  PWM_Instance[0]->setPWM_DCPercentage_manual(PWM_Pins[0], level1);
  PWM_Instance[1]->setPWM_DCPercentage_manual(PWM_Pins[1], level2);
  PWM_Instance[2]->setPWM_DCPercentage_manual(PWM_Pins[2], level1);
  PWM_Instance[3]->setPWM_DCPercentage_manual(PWM_Pins[3], level2);
  PWM_Instance[4]->setPWM_DCPercentage_manual(PWM_Pins[4], level1);
  PWM_Instance[5]->setPWM_DCPercentage_manual(PWM_Pins[5], level2);
  PWM_Instance[6]->setPWM_DCPercentage_manual(PWM_Pins[6], test);

  return true;
}


void buttonPressed() {
  step256=0;
  //Set timer to work for your loop code time   
//  if (millis() - buttonTime > 250) {
    //button press ok
    buttonPress= true;
//  }
//  buttonTime = millis();
}

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  delay(100);

  Serial.print(F("\nStarting TimerInterruptTest on ")); Serial.println(BOARD_NAME);
  Serial.println(RPI_PICO_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));


  Serial.print(F("\nStarting PWM_Waveform_Fast on "));
  Serial.println(BOARD_NAME);
  Serial.println(RP2040_PWM_VERSION);

  pinMode(PIN_T0, OUTPUT);
  pinMode(PIN_T1, OUTPUT);
  pinMode( 1, OUTPUT);
  pinMode(12, INPUT_PULLUP);
  attachInterrupt(12, buttonPressed, RISING);
  // can be CHANGE or LOW or RISING or FALLING or HIGH

  frequency = 1000;
  for (uint8_t index = 0; index < NUM_OF_PINS; index++)
  {
    Serial.print(index);
    Serial.print("\t");
    Serial.print(PWM_Pins[index]);
    Serial.print("\t");
    Serial.print(frequency);
    Serial.print("\t\t");
    Serial.print(dutyCycle[index]);

    PWM_Instance[index] = new RP2040_PWM(PWM_Pins[index], frequency, dutyCycle[index]);

    if (PWM_Instance[index])
    {
      PWM_Instance[index]->setPWM_manual(PWM_Pins[index], PWM_data_idle_top, PWM_data_idle_div, PWM_data_idle, true);
//      PWM_Instance[index]->setPWM();

      uint32_t div = PWM_Instance[index]->get_DIV();
      uint32_t top = PWM_Instance[index]->get_TOP();

      Serial.print("\t\t");
      Serial.println(PWM_Instance[index]->getActualFreq());

      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance[index]->get_freq_CPU());
    }
    else
    {
      Serial.println();
    }
  }

  Serial.println(dashLine);

  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL, TimerHandler0))
  {
    Serial.print(F("Starting ITimer0 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));

  Serial.println(dashLine);

  target = 0.0;
  scale1=(sin(target)*50.0);
  scale2=(cos(target)*50.0);

  Serial.println();
  Serial.print("Scales: ");
  Serial.print(scale1);
  Serial.print(", ");
  Serial.print(scale2);
  Serial.println();
 

}

void printPWMInfo(RP2040_PWM* PWM_Instance)
{
  uint32_t div = PWM_Instance->get_DIV();
  uint32_t top = PWM_Instance->get_TOP();

  // PWM_Freq = ( F_CPU ) / [ ( TOP + 1 ) * ( DIV + DIV_FRAC/16) ]
  PWM_LOGINFO1("Actual PWM Frequency = ",
               PWM_Instance->get_freq_CPU() / ( (PWM_Instance->get_TOP() + 1) * (PWM_Instance->get_DIV() ) ) );

  PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance->get_freq_CPU());
}

void loop()
{
  
//  for (int index = 0; index < NUM_PWM_POINTS; index++)
  float angle;
  for (angle = 0.0; angle < (2*M_PI); angle += 0.1)
  {
    if(buttonPress)
    {
      Serial.println(F("Button A Pressed"));
      buttonPress= false;
      break;
    }

    // Use at low freq to check
    printPWMInfo(PWM_Instance[0]);
    printPWMInfo(PWM_Instance[1]);

  scale1=(sin(angle)*50.0);
  scale2=(cos(angle)*50.0);
  #if 0
  Serial.println();
  Serial.print("Scales: ");
  Serial.print(scale1);
  Serial.print(", ");
  Serial.print(scale2);
  Serial.println();
  #endif
    // delay something here between data
    delay(100);
  }
}
