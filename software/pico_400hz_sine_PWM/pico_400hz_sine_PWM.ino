/****************************************************************************************************************************
  djrm Jun 4 2023
  based on RP2040_PWM library demo

  test functions to drive Ferranti moving map display
  generate three pairs of resolver 400 Hz sin/cos waveforms
  ralative amplitudes try to match fine, medium, and coarse requirements
  to position map film at arbitary absolute position
  see https://github.com/DavidJRichards/Aviation_Moving_Map_Display#exercise-software
  WIP, not yest fully working 

  https://github.com/khoih-prog/RP2040_PWM
  https://github.com/khoih-prog/RPI_PICO_TimerInterrupt
  https://github.com/Uberi/Arduino-CommandParser

  // Rotary Encoder library
  http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx

  // specific info about explorer board
  https://forums.pimoroni.com/t/program-the-pico-explorer-with-arduino-ide/17983

  https://www.mikrocontroller.net/attachment/346746/AND9282-D_AC_Zero_Crossing.pdf
  https://www.edn.com/mains-driven-zero-crossing-detector-uses-only-a-few-high-voltage-parts/
  https://d1.amobbs.com/bbs_upload782111/files_40/ourdev_643643NSY57M.pdf
  https://www.edn.com/a-circuit-for-mains-synchronization-has-two-separate-outputs-for-each-half-period/

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
#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     0 //4
#define _PWM_LOGLEVEL_                2 //2

#include "RPi_Pico_TimerInterrupt.h" // pwm duty cycle change timer 
#include <math.h>            // use sin and cos functions in main loop only
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // used by LCD display
#include <CommandParser.h>   // serial commands
#include "RP2040_PWM.h"      // to define PWM channels
#include <RotaryEncoder.h>

#define ENCODER_PIN_IN2   20
#define ENCODER_PIN_IN1   22 
#define ENCODER_SW_PIN    26

// Setup a RotaryEncoder with 4 steps per latch for the 2 signal input pins:
// RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

// Setup a RotaryEncoder with 2 steps per latch for the 2 signal input pins:
RotaryEncoder encoder(ENCODER_PIN_IN1, ENCODER_PIN_IN2, RotaryEncoder::LatchMode::TWO03);


#define TFT_CS        17
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        16
#define TFT_MOSI      19
#define TFT_SCLK      18

typedef CommandParser<> MyCommandParser;
MyCommandParser parser;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

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

#define LED_ON        LOW
#define LED_OFF       HIGH

#define pinLED        25    // On-board BUILTIN_LED
#define pinOpSync     27    // 
#define pinIpTrig     28

#define ButtonA       12    // PWM 6A
#define ButtonB       13    // PWM 6B
#define ButtonX       14    // PWM 7A
#define ButtonY       15    // PWM 7B

uint32_t PWM_Pins[]     = { 0, 2, 4, 6, 1, 3, 21, 7  };
#define NUM_OF_PINS       ( sizeof(PWM_Pins) / sizeof(uint32_t) )
RP2040_PWM* PWM_Instance[NUM_OF_PINS];

/*
#define pin1          0     // PWM channel 0A tx
#define pin2          1     // PWM channel 0B rx
#define pin4          2     // PWM channel 1A gp2
#define pin5          3     // PWM channel 1B int
#define pin6          4     // PWM channel 2A sda
#define pin7          5     // PWM channel 2B scl
#define pin9          6     // PWM channel 3A r
#define pin10         7     // PWM channel 3B g
#define pin11         8     // PWM channel 4A b (mot1-)
#define pin12         9     // PWM channel 4B (mot1+)
#define pin14         10    // PWM channel 5A (mot2-)
#define pin15         11    // PWM channel 5B (mot2+)
#define pin16         12    // PWM channel 6A (button A)
#define pin17         13    // PWM channel 6B (button B)
#define pin34         28    // PWM channel 6A adc2
#define pin19         14    // PWM channel 7A (button x)
#define pin20         15    // PWM channel 7B (button y)
*/

uint16_t PWM_data_idle_top = 1330; // to give 100 kHz PWM with 133MHz clock
uint8_t PWM_data_idle_div = 1;
// You can select any value
uint16_t PWM_data_idle = 124;
// dummy values for channel creation
float PWM_frequency = 1000;
float PWM_dutyCycle = 50.0f;

// 36 table entries,  === 36 samples per cycle
// 1E6Hz  / 400Hz / (36) = 69.444 uS timer interval
// 69 = 402.576 Hz
// 70 = 396.825 Hz

// 40 table entries,  === 40 samples per cycle
// 1E6Hz  / 400Hz / (40) = . uS timer interval
// 62 = 403.2 Hz
// 64 = 396.8 Hz

// 20 table entries,  === 20 samples per cycle
// 1E6Hz  / 400Hz / (20) = . uS timer interval
// 125 = 400.0 Hz

#define NUM_SINE_ELEMENTS 36        // steps per cycle of 400Hz wave
// choose 396 to select lower of two possible frequencies, 400 selects 402
#define SINEWAVE_FREQUENCY_HZ 400   // target frequency
#define SYNC_OFFSET_COUNT 4
#define PULSE_OFFSET_COUNT 2
#define DIV_CONST 740

struct sine_table_ {
  int num_elements=NUM_SINE_ELEMENTS;
  int sinewave_frequency=SINEWAVE_FREQUENCY_HZ;
  int sync_offset=SYNC_OFFSET_COUNT;
  //uint16_t elements[NUM_SINE_ELEMENTS]; // not needed in wave generating code, values for display / reference only
  float factors[NUM_SINE_ELEMENTS];
  int16_t levels[NUM_SINE_ELEMENTS];
  uint16_t timer_interval;              // calculated in build sinetable function
  float stepsize;                       // calculated in build sinetable function
} sine_table;

bool buttonAPress = false;
bool buttonBPress = false;
bool encoderPress = false;

int stepidx=1;
int steplist[] = {1,30,900};

int delayidx=1;
float delaylist[] = {10.0 ,100.0, 1000.0 };

#define TIMER1_INTERVAL_MS        20
#define DEBOUNCING_INTERVAL_MS    100
#define LONG_PRESS_INTERVAL_MS    1000

// Init RPI_PICO_Timer, can use any from 0-15 pseudo-hardware timers
RPI_PICO_Timer ITimer0(0);
RPI_PICO_Timer ITimer1(1);

//RP2040_PWM* PWM_Instance[NUM_OF_PINS];
// todo: use something like these ...
struct PWM_ {
  const char    *name;
  RP2040_PWM*    PWM_Instance;
  const uint32_t PWM_pin;       // pin number to initialise to
  float          PWM_duty;      // working PWM duty cycle variable
};

struct resolver_ {
  const char    *name;
  struct PWM_   PWM[2];         // PWM channels for this resolver
  float         angle;          // target resolver angle
  int           amplitude[2];   // channel 400Hz sin/cos amplitude vectors (int +- 500)
  float         level[2];       // channel 400Hz sin/cos amplitude vectors (float +- 50.0)
 };

struct transport_ {
  struct resolver_ resolvers[4];
  long absolute;                // moving map absolute position
  int  autostep;                // step size for automatic (or encoder) movement
  bool  automatic;               // enable automatic map movement (E/W)
  int  autodelay;               // time between automatic steps
};        

struct transport_ transport = {
// name,      instance, pin, duty1, instance, pin, duty2, angle, amp1, amp2, duty1, duty2  
  "Fine",     "sin",  NULL,     0,   0.0,   "cos",  NULL,     2,   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, duty, instance, pin, duty, angle, scale1, scale2
  "Medium",   "sin",  NULL,     4,   0.0,   "cos",  NULL,     6,   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Coarse",   "sin",  NULL,     1,   0.0,   "cos",  NULL,     3,   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Reference","ref+", NULL,    21,   0.0,   "ref-", NULL,     7,   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  0L,                                                                          // absolute
  10,                                                                          // autostep
  false,                                                                       // automatic
  10,                                                                          // autodelay
};


char dashLine[] = "=====================================================================";

// button debounce variables
volatile bool SWPressed     = false;
volatile bool SWLongPressed = false;

// index into 400Hz sine table for PWM frequency generation
volatile int step_index = 0; 

// menu variables TODO use transport structure variables
float autostep =  30;
bool  automatic = false;
int   autodelay = 10;
float absolute =  0;
int amplitude=DIV_CONST;

// screensaver
#define SLEEPTIME 300
bool awaken =     false;
bool asleep =     false;
int sleeptimer =  SLEEPTIME;

void build_sintable(void)
{
  int n;
  sine_table.stepsize = 360 / sine_table.num_elements;
  sine_table.timer_interval = 1E6 / sine_table.sinewave_frequency / sine_table.num_elements;

  for(n=0; n<sine_table.num_elements; n++)
  {
    sine_table.factors[n] =   (sin(M_PI*(n*sine_table.stepsize)/180.0) * 128) / 256.0;  // table entries -1.0 to + 1.0
    sine_table.levels[n] = int(sin(M_PI*(n*sine_table.stepsize)/180.0) * 512);          // table entries -512 to +512
  }

#if 1
  Serial.print("Sine table num elements: ");
  Serial.println(sine_table.num_elements);
  Serial.print("Sine table step degrees: ");
  Serial.println(sine_table.stepsize);
  Serial.print("Timer interval uS: ");
  Serial.println(sine_table.timer_interval);

  Serial.println();
  for(int j=0; j<4; j++)
  {
    for(int i=0;  i<sine_table.num_elements/4; i++)
    {
      Serial.print(sine_table.levels[(j*sine_table.num_elements/4) + i]);
      Serial.print(",\t");
    }
  Serial.println();
  }
  Serial.println(dashLine);
#endif
}


// PWM duty change used in stepping sine values of 400 Hz waveforms
bool TimerHandler0(struct repeating_timer *t)
{ 
  (void) t;
  float float_sine_step_value;
  int16_t int_sine_step_value;
  uint16_t dc_levels[8];
  float dc_percent[8];

  if(step_index >= sine_table.num_elements)
  {
    step_index = 0;
  }

#if 0 // use float percentage 0.0 to 100.0
  #define MID_POINT_FLOAT 50.0

// center resultant waveform around 50% PWM full scale is 100.0 * ( sine256[step256%256] / 256.0f )
  float_sine_step_value = ( sine_table.factors[step_index] ); 

  // fine resolver output
  dc_percent[0] = MID_POINT_FLOAT +  transport.resolvers[0].level[0] * float_sine_step_value;
  PWM_Instance[0]->setPWM_DCPercentage_manual(PWM_Pins[0], dc_percent[0]);
  dc_percent[1] = MID_POINT_FLOAT +  transport.resolvers[0].level[1] * float_sine_step_value;
  PWM_Instance[1]->setPWM_DCPercentage_manual(PWM_Pins[1], dc_percent[1]);

  // medium resolver output
  dc_percent[2] = MID_POINT_FLOAT + transport.resolvers[1].level[0] * float_sine_step_value;
  PWM_Instance[2]->setPWM_DCPercentage_manual(PWM_Pins[2], dc_percent[2]);
  dc_percent[3] = MID_POINT_FLOAT + transport.resolvers[1].level[1] * float_sine_step_value;  
  PWM_Instance[3]->setPWM_DCPercentage_manual(PWM_Pins[3], dc_percent[3]);

  // coarse resolver output
  dc_percent[4] = MID_POINT_FLOAT + transport.resolvers[2].level[0] * float_sine_step_value;
  PWM_Instance[4]->setPWM_DCPercentage_manual(PWM_Pins[4], dc_percent[4]);
  dc_percent[5] = MID_POINT_FLOAT + transport.resolvers[2].level[1] * float_sine_step_value;  
  PWM_Instance[5]->setPWM_DCPercentage_manual(PWM_Pins[5], dc_percent[5]);
 
  // reference channel
  dc_percent[6] = MID_POINT_FLOAT + transport.resolvers[3].level[0] * float_sine_step_value; // reference sinewave output
  PWM_Instance[6]->setPWM_DCPercentage_manual(PWM_Pins[6], dc_percent[6] );
  dc_percent[7] = MID_POINT_FLOAT + transport.resolvers[3].level[1] * float_sine_step_value; // reference sinewave output
  PWM_Instance[7]->setPWM_DCPercentage_manual(PWM_Pins[7], dc_percent[7] );

#else // use int level 0 to 1000 TODO, maybe faster
#define MID_POINT_INT 500
  int_sine_step_value = ( sine_table.levels[step_index] ); 

// manual level is integer 0 to 1000 (actually 800 due to output voltage limitation problem)  
// sine_table is +- 512 ( 10 bits )
// scale is +- 500 (10 bits )
// product is 20 bits - needs to be 10 - so divide by 10 bits 
  dc_levels[0] = MID_POINT_INT +    transport.resolvers[0].amplitude[0]  * int_sine_step_value / amplitude;
  PWM_Instance[0]->setPWM_manual_Fast(PWM_Pins[0], dc_levels[0]);
  dc_levels[1] = MID_POINT_INT +    transport.resolvers[0].amplitude[1]  * int_sine_step_value / amplitude;
  PWM_Instance[1]->setPWM_manual_Fast(PWM_Pins[1], dc_levels[1]);

  dc_levels[2] = MID_POINT_INT +    transport.resolvers[1].amplitude[0]  * int_sine_step_value / amplitude;
  PWM_Instance[2]->setPWM_manual_Fast(PWM_Pins[2], dc_levels[2]);
  dc_levels[3] = MID_POINT_INT +    transport.resolvers[1].amplitude[1]  * int_sine_step_value / amplitude;
  PWM_Instance[3]->setPWM_manual_Fast(PWM_Pins[3], dc_levels[3]);

  dc_levels[4] = MID_POINT_INT +    transport.resolvers[2].amplitude[0]  * int_sine_step_value / amplitude;
  PWM_Instance[4]->setPWM_manual_Fast(PWM_Pins[4], dc_levels[4]);
  dc_levels[5] = MID_POINT_INT +    transport.resolvers[2].amplitude[1]  * int_sine_step_value / amplitude;
  PWM_Instance[5]->setPWM_manual_Fast(PWM_Pins[5], dc_levels[5]);

  dc_levels[6] = MID_POINT_INT +   transport.resolvers[3].amplitude[0] * int_sine_step_value / amplitude; // reference +sinewave output
  PWM_Instance[6]->setPWM_manual_Fast(PWM_Pins[6], dc_levels[6]);
  dc_levels[7] = MID_POINT_INT +   transport.resolvers[3].amplitude[1] * int_sine_step_value / amplitude; // reference -sinewave output
  PWM_Instance[7]->setPWM_manual_Fast(PWM_Pins[7], dc_levels[7]);

#endif

  digitalWrite(pinOpSync,step_index == PULSE_OFFSET_COUNT); // sync pulse output
  step_index++;
  return true;
}

// ISR for frequency sync input
void syncInput(void) {
    ITimer0.stopTimer();
    step_index=sine_table.sync_offset;
    ITimer0.restartTimer();
}

// ISR for button presses
// n.b. no debounce

void buttonAPressed() {
  buttonAPress= true;
}
void buttonBPressed() {
  buttonBPress= true;
}
void encoderPressed() {
  encoderPress= true;
}

// these constants represent the gearing between the resolvers 
const int ratio1 = 30;        // fine to medium
const int ratio2 = ratio1*30; // fine to coarse

#define offset_coarse 0  // range on coarse is -90 to +90, so offset is +90 (to verify)

// back convert resolver settings to absolute film position
unsigned long res2abs(int fine, int medium, int coarse)
{
// use top bits of each resolver, get bottom bits from next in chain  
    return      (fine   % ratio1)     
    + (ratio1 * (medium % ratio1))   
    + (ratio2 * (coarse % ratio2));
}

// convert required absolute film position to resolver angles
void abs2res(long absolute, float *fine, float *medium, float *coarse)
{
  if(absolute < 0) absolute = 0;
  *coarse = (absolute / ratio2) - offset_coarse;
  *medium = (absolute / ratio1) % 360;
  *fine  =   absolute           % 360;
}        

// transfer required resolver angles to PWM scale multipliers
void anglesUpdate(void)
{
  float target;

  target = fmod(transport.resolvers[0].angle, 360) * M_PI/180.0;
  transport.resolvers[0].amplitude[0] = sin(target) * 500;
  transport.resolvers[0].amplitude[1] = cos(target) * 500;
  transport.resolvers[0].level[0] = sin(target) * 50.0;
  transport.resolvers[0].level[1] = cos(target) * 50.0;

  target = fmod(transport.resolvers[1].angle, 360) * M_PI/180.0;
  transport.resolvers[1].amplitude[0] = sin(target) * 500;
  transport.resolvers[1].amplitude[1] = cos(target) * 500;
  transport.resolvers[1].level[0] = sin(target) * 50.0;
  transport.resolvers[1].level[1] = cos(target) * 50.0;

  target = fmod(transport.resolvers[2].angle, 360) * M_PI/180.0;
  transport.resolvers[2].amplitude[0] = sin(target) * 500;
  transport.resolvers[2].amplitude[1] = cos(target) * 500;
  transport.resolvers[2].level[0] = sin(target) * 50.0;
  transport.resolvers[2].level[1] = cos(target) * 50.0;
 
  transport.resolvers[3].amplitude[0] = 500.0;
  transport.resolvers[3].amplitude[1] = -500.0;
  transport.resolvers[3].level[0] = +50.0;
  transport.resolvers[3].level[1] = -50.0;
}

// screensaver
void displayDisable(void)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_BLACK);
}

// show common system settings and individual PWM channel settings
void displayUpdate(void)
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.println();
  Serial.println("=====================");

// show details for individual PWM channel settings
  for (int i = 0; i < 3; i++)
  {
  #if 1
    Serial.println();
    Serial.print(transport.resolvers[i].name);
    Serial.print(" Angle = ");
    Serial.println(transport.resolvers[i].angle);

    Serial.print("Scales ");
    Serial.print(transport.resolvers[i].level[0]);
    Serial.print(", ");
    Serial.println(transport.resolvers[i].level[1]);
  #endif
    tft.setCursor(0, 0 + i * 50);
    tft.print(transport.resolvers[i].name);
    tft.print(" = ");
    tft.print(transport.resolvers[i].angle);
    tft.println("\xF7"); // degree symbol

    tft.print(transport.resolvers[i].PWM[0].name);
    tft.print("=");
    tft.print(transport.resolvers[i].level[0],1);
    tft.print("% ");
    tft.print(transport.resolvers[i].PWM[1].name);
    tft.print("=");
    tft.print(transport.resolvers[i].level[1],1);
    tft.println("%");
  }

  absolute = res2abs(transport.resolvers[0].angle, transport.resolvers[1].angle, transport.resolvers[2].angle);

// show common detail information
#if 1
  Serial.println();

  Serial.print("Step = ");
  Serial.print(autostep);

  Serial.print(", Delay= ");
  Serial.println(autodelay);

  Serial.print("Auto = ");
  Serial.print(automatic);

  Serial.print(", Amp = ");
  Serial.println(amplitude);

  Serial.print("Absolute = ");
  Serial.println(absolute);
#endif
  tft.println();
  tft.print("Absolute = ");
  tft.println(absolute);
  tft.println();

  tft.print("Step =     ");
  tft.println(autostep);

  tft.print("Delay =    ");
  tft.println(autodelay);

  tft.print("Auto =     ");
  tft.println(automatic);
}

// serial command handler functions
void cmd_report(MyCommandParser::Argument *args, char *response) {
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_abs(MyCommandParser::Argument *args, char *response) {
  absolute = args[0].asDouble;
  abs2res(absolute, &transport.resolvers[0].angle, &transport.resolvers[1].angle, &transport.resolvers[2].angle);
  anglesUpdate();
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_amplitude(MyCommandParser::Argument *args, char *response) {
  amplitude = args[0].asDouble;
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_step(MyCommandParser::Argument *args, char *response) {
  autostep = args[0].asDouble;
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_automatic(MyCommandParser::Argument *args, char *response) {
  automatic = ! automatic;
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_autodelay(MyCommandParser::Argument *args, char *response) {
  autodelay = args[0].asDouble;
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_fin(MyCommandParser::Argument *args, char *response) {
  transport.resolvers[0].angle = args[0].asDouble;
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
  anglesUpdate();
  displayUpdate();
}

void cmd_med(MyCommandParser::Argument *args, char *response) {
  transport.resolvers[1].angle = args[0].asDouble;
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
  anglesUpdate();
  displayUpdate();
}

void cmd_coa(MyCommandParser::Argument *args, char *response) {
  transport.resolvers[2].angle = args[0].asDouble;
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
  anglesUpdate();
  displayUpdate();
}

// button debounce timer -----------------------------------------------------------------------------------------

bool TimerHandler1(struct repeating_timer *t)
{ 
  (void) t;
  
  static unsigned int debounceCountSWPressed  = 0;
  static unsigned int debounceCountSWReleased = 0;

#if (LOCAL_DEBUG > 1)
  static unsigned long SWPressedTime;
  static unsigned long SWReleasedTime;

  unsigned long currentMillis = millis();
#endif

  if ( (!digitalRead(ENCODER_SW_PIN)) )
  {
    // Start debouncing counting debounceCountSWPressed and clear debounceCountSWReleased
    debounceCountSWReleased = 0;

    if (++debounceCountSWPressed >= DEBOUNCING_INTERVAL_MS / TIMER1_INTERVAL_MS)
    {
      // Call and flag SWPressed
      if (!SWPressed)
      {
#if (LOCAL_DEBUG > 1)   
        SWPressedTime = currentMillis;
        
        Serial.print("SW Press, from millis() = "); Serial.println(SWPressedTime);
#endif

        SWPressed = true;
        // Do something for SWPressed here in ISR
        // But it's better to use outside software timer to do your job instead of inside ISR
        //Your_Response_To_Press();
      }

      if (debounceCountSWPressed >= LONG_PRESS_INTERVAL_MS / TIMER1_INTERVAL_MS)
      {
        // Call and flag SWLongPressed
        if (!SWLongPressed)
        {
#if (LOCAL_DEBUG > 1)
          Serial.print("SW Long Pressed, total time ms = "); Serial.print(currentMillis);
          Serial.print(" - "); Serial.print(SWPressedTime);
          Serial.print(" = "); Serial.println(currentMillis - SWPressedTime);                                           
#endif

          SWLongPressed = true;
          // Do something for SWLongPressed here in ISR
          // But it's better to use outside software timer to do your job instead of inside ISR
          //Your_Response_To_Long_Press();
        }
      }
    }
  }
  else
  {
    // Start debouncing counting debounceCountSWReleased and clear debounceCountSWPressed
    if ( SWPressed && (++debounceCountSWReleased >= DEBOUNCING_INTERVAL_MS / TIMER1_INTERVAL_MS))
    {
#if (LOCAL_DEBUG > 1)      
      SWReleasedTime = currentMillis;

      // Call and flag SWPressed
      Serial.print("SW Released, from millis() = "); Serial.println(SWReleasedTime);
#endif

      SWPressed     = false;
      SWLongPressed = false;

      // Do something for !SWPressed here in ISR
      // But it's better to use outside software timer to do your job instead of inside ISR
      //Your_Response_To_Release();

      // Call and flag SWPressed
#if (LOCAL_DEBUG > 1)
      Serial.print("SW Pressed total time ms = ");
      Serial.println(SWReleasedTime - SWPressedTime);
#endif

      debounceCountSWPressed = 0;
    }
  }

  return true;
}

// setup -----------------------------------------------------------------------------------------

void setup()
{
  tft.init(240, 240);           // Init ST7789 240x240
  tft.setRotation(2);
  tft.setSPISpeed(40000000);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Resolver PWM");

  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  delay(100);
  Serial.println();
  Serial.println(dashLine);
  Serial.println("Moving map transport exerciser.");
  Serial.println(dashLine);
  
  build_sintable();

  pinMode(pinLED, OUTPUT);
  pinMode(pinOpSync, OUTPUT);
  pinMode(pinIpTrig, INPUT);
  pinMode(ButtonA, INPUT_PULLUP);
  pinMode(ButtonB, INPUT_PULLUP);
  pinMode(ButtonX, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  attachInterrupt(ButtonA, buttonAPressed, RISING);
  attachInterrupt(ButtonB, buttonBPressed, RISING);
  attachInterrupt(ENCODER_SW_PIN, encoderPressed, RISING);
  
  // can be CHANGE or LOW or RISING or FALLING or HIGH

#if 0
  Serial.print(F("\nStarting PWM_Waveform_Fast on "));
  Serial.println(BOARD_NAME);
  Serial.println(RP2040_PWM_VERSION);
#endif

  for (uint8_t index = 0; index < NUM_OF_PINS; index++)
  {
    // use dummy values
    PWM_Instance[index] = new RP2040_PWM(PWM_Pins[index], PWM_frequency, PWM_dutyCycle);

    if (PWM_Instance[index])
    {
      // initial values
      PWM_Instance[index]->setPWM_manual(PWM_Pins[index], PWM_data_idle_top, PWM_data_idle_div, PWM_data_idle, true);

      uint32_t div = PWM_Instance[index]->get_DIV();
      uint32_t top = PWM_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance[index]->get_freq_CPU());
    }
  }

#if 0
  Serial.print(F("\nStarting TimerInterruptTest on ")); Serial.println(BOARD_NAME);
  Serial.println(RPI_PICO_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));
#endif

 // Interval in microsecs
  if (ITimer0.attachInterruptInterval(sine_table.timer_interval, TimerHandler0))
  {
    Serial.print(F("Starting ITimer0 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));

  Serial.print("PWM Actual frequency[7] = ");
  Serial.print(PWM_Instance[7]->getActualFreq()/10);
  Serial.println(" kHz");

  attachInterrupt(pinIpTrig, syncInput, RISING);

  // Interval in microsecs
  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1))
  {
    Serial.print(F("Starting ITimer1 OK, millis() = ")); Serial.println(millis());
  }
  else
  {
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));
  }

  parser.registerCommand("rep", "",  &cmd_report);
  parser.registerCommand("fin", "d", &cmd_fin);
  parser.registerCommand("med", "d", &cmd_med);
  parser.registerCommand("coa", "d", &cmd_coa);
  parser.registerCommand("abs", "d", &cmd_abs);
  parser.registerCommand("step", "d", &cmd_step);
  parser.registerCommand("auto", "", &cmd_automatic);
  parser.registerCommand("del",  "d", &cmd_autodelay);
  parser.registerCommand("amp",  "d", &cmd_amplitude);
  
  Serial.println("to show summary of current settings:");
  Serial.println("registered command: rep ");
  Serial.println();
  Serial.println("to set fine, medium, or coarse resolver angles in degrees:");
  Serial.println("registered command: fin <double> ");
  Serial.println("registered command: med <double> ");
  Serial.println("registered command: coa <double> ");
  Serial.println();
  Serial.println("to set absolute film transport index value:");
  Serial.println("registered command: abs <double> ");
  Serial.println("to set step value (+ve or -ve) used for automatic increment:");
  Serial.println("registered command: step <double> ");
  Serial.println("Note , Button A to enable automatic increment");
  Serial.println("Note , Button B to disable automatic increment");
  Serial.println();
  Serial.println("to enable (1) or disable (0) automatic increment:");
  Serial.println("registered command: auto <double> ");
  Serial.println("delay between automatic updates mS:");
  Serial.println("registered command: del <double> ");
  Serial.println();
  Serial.println("Note fine step for 1 degree is absolute 1");
  Serial.println("Note medium step for 1 degree is absolute 30");
  Serial.println("Note coarse step for 1 degree is absolute 900");
 
  anglesUpdate();
  displayUpdate();
}

// activated when IMER_ INTERRUPT_ DEBUG > 2
void printPWMInfo(RP2040_PWM* PWM_Instance)
{
  uint32_t div = PWM_Instance->get_DIV();
  uint32_t top = PWM_Instance->get_TOP();

  // PWM_Freq = ( F_CPU ) / [ ( TOP + 1 ) * ( DIV + DIV_FRAC/16) ]
  PWM_LOGINFO1("Actual PWM Frequency = ",
               PWM_Instance->get_freq_CPU() / ( (PWM_Instance->get_TOP() + 1) * (PWM_Instance->get_DIV() ) ) );

  PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance->get_freq_CPU());

//  delay(100);
}

// loop -----------------------------------------------------------------------------------------

void loop()
{
  static int refresh_time=0;
  static int old_absolute=0;

  if (Serial.available()) {
    char line[128];
    size_t lineLength = Serial.readBytesUntil('\n', line, 127);
    line[lineLength] = '\0';

    char response[MyCommandParser::MAX_RESPONSE_SIZE];
    if(!parser.processCommand(line, response))
      Serial.println(response);
  }

  static int pos = 0;
  encoder.tick();

  int newPos = encoder.getPosition();
  
  if(newPos < 0)
  {
    encoder.setPosition(0);
    newPos = 0;
  }

  if (pos != newPos) 
  {
    #if 0
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.println((int)(encoder.getDirection()));
    #endif
    pos = newPos;
 
    if(!automatic)
    {
      absolute = pos * autostep;
      abs2res(absolute, &transport.resolvers[0].angle, &transport.resolvers[1].angle, &transport.resolvers[2].angle);
      anglesUpdate();
    }
    awaken = true;
  } // if

  if(buttonAPress)
  {
  //  Serial.println(F("Button A Pressed"));
    buttonAPress= false;
    automatic = true;
    if(++delayidx > 2) delayidx = 0;
    autodelay = delaylist[delayidx];
    displayUpdate();
    awaken = true;
  }

  if(buttonBPress)
  {
  //  Serial.println(F("Button B Pressed"));
    buttonBPress= false;
    automatic = false;
    if(++stepidx > 2) stepidx = 0;
    autostep = steplist[stepidx];
    encoder.setPosition(absolute/autostep); // update encode value after auto run
    displayUpdate();
    awaken = true;
  }

#if 0
  if(SWPressed == true)
  {
    if(++stepidx > 2) stepidx = 0;
    autostep = steplist[stepidx];
    encoder.setPosition(absolute/autostep); // update encode value after auto run
    //displayUpdate();
    old_absolute=!absolute;
    awaken = true;
  }
#endif

#if 1
  if(SWLongPressed == true)
  {
    absolute = 0;
    abs2res(absolute, &transport.resolvers[0].angle, &transport.resolvers[1].angle, &transport.resolvers[2].angle); 
    anglesUpdate();
    //displayUpdate();
    old_absolute=!absolute;
    awaken = true;
  }
#endif

#if 0
  if(encoderPress)
  {
    Serial.println(F("Encoder Button Pressed"));
    encoderPress = false;
    absolute = 0;
    abs2res(absolute, &transport.resolvers[0].angle, &transport.resolvers[1].angle, &transport.resolvers[2].angle); 
    anglesUpdate();
//    displayUpdate();
    awaken = true;
  }
#endif

  if(automatic)
  {
    static int del_count=0;
    int del_value;
    del_value=millis();
    refresh_time=del_value; // inhibit auto display refresh (force display update with button press)
    if(del_value - del_count > autodelay)
    {
      del_count=del_value;
      absolute += autostep;   
      abs2res(absolute, &transport.resolvers[0].angle, &transport.resolvers[1].angle, &transport.resolvers[2].angle); 
      anglesUpdate();
    }
  }

  // update display every second if absolute value has changed
  if (millis() - refresh_time > 1000) 
  {
    static bool ledon = true;
    digitalWrite(pinLED,ledon); // LED pulse output
    if(awaken == true)
    {
      sleeptimer = SLEEPTIME;
      awaken = false;
    }
    if(sleeptimer > 0)
    {
      sleeptimer -= 1;
      asleep = false;
      ledon = true;
    }
    else
    {
      ledon = ! ledon;
      asleep = true;
      displayDisable();
    }

    if(old_absolute != absolute)
    {
      old_absolute = absolute;
      displayUpdate();
    }
    refresh_time = millis();
  }

#if 0
  // Use at low freq to check
  printPWMInfo(PWM_Instance[0]);
  printPWMInfo(PWM_Instance[1]);
  printPWMInfo(PWM_Instance[2]);
  printPWMInfo(PWM_Instance[3]);
  printPWMInfo(PWM_Instance[4]);
  printPWMInfo(PWM_Instance[5]);
  printPWMInfo(PWM_Instance[6]);
  printPWMInfo(PWM_Instance[7]);
  delay(500);
#endif

}
// end