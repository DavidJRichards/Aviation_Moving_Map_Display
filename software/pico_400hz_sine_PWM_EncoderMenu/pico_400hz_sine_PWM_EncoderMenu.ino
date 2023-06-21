/****************************************************************************************************************************
  djrm Jun 18 2023
 
  test functions to drive Ferranti moving map display
  generate three pairs of resolver 400 Hz sin/cos waveforms
  ralative amplitudes try to match fine, medium, and coarse requirements
  to position map film at arbitary absolute position

  enhanced to drive extra two pairs of resolver outputs for heading and NtoS movement
  see https://github.com/DavidJRichards/Aviation_Moving_Map_Display#exercise-software

  // TFTeSPI graphic display library
  https://github.com/Bodmer/TFT_eSPI

  // LCD Menu 2 library
  https://github.com/Jomelo/LCDMenuLib2

  // Rotary Encoder library
  http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx

  // specific info about explorer board used for development
  https://forums.pimoroni.com/t/program-the-pico-explorer-with-arduino-ide/17983

  // zero crossing detector for 400Hz suply synchronization
  https://www.edn.com/mains-driven-zero-crossing-detector-uses-only-a-few-high-voltage-parts/

  // using for base PWM
  https://github.com/khoih-prog/RP2040_PWM
 
  PWM_Waveform_Fast.ino
  For RP2040 boards
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/RP2040_PWM
  Licensed under MIT license

  The RP2040 PWM block has 8 identical slices. Each slice can drive two PWM output signals, or measure the frequency
  or duty cycle of an input signal. This gives a total of up to 16 controllable PWM outputs. All 30 GPIO pins can be driven
  by the PWM block

  // using timer for 400Hz sinewave generation
  https://github.com/khoih-prog/RPI_PICO_TimerInterrupt

*****************************************************************************************************************************/
// These define's must be placed at the beginning before #include "TimerInterrupt_Generic.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     0 //4
#define _PWM_LOGLEVEL_                2 //2
#include "RPi_Pico_TimerInterrupt.h" // pwm duty cycle change timer 
#include "RP2040_PWM.h"      // to define PWM channels
#include <math.h>            // use sin and cos functions in main loop only

#define _LCDML_cfg_use_ram 
#include <LCDMenuLib2.h>  
#include <RotaryEncoder.h>

#include <SPI.h>             // used by LCD display
//#define DISABLE_ALL_LIBRARY_WARNINGS
#define DISABLE_TOUCH_LIBRARY_WARNING
#include <TFT_eSPI.h>

  #define _LCDML_TEXT_COLOR       TFT_WHITE
  #define _LCDML_BACKGROUND_COLOR TFT_BLACK
  
  #define _LCDML_FONT_SIZE   2   
  #define _LCDML_FONT_W      (6*_LCDML_FONT_SIZE)             // font width 
  #define _LCDML_FONT_H      (8*_LCDML_FONT_SIZE)             // font heigt 
  
  // settings for  lcd
  #define _LCDML_lcd_w       240            // lcd width
  #define _LCDML_lcd_h       240             // lcd height
 
  // nothing change here
  #define _LCDML_cols_max    (_LCDML_lcd_w/_LCDML_FONT_W)  
  #define _LCDML_rows_max    (_LCDML_lcd_h/_LCDML_FONT_H) 

  // rows and cols 
  // when you use more rows or cols as allowed change in LCDMenuLib.h the define "_LCDML_DISP_cfg_max_rows" and "_LCDML_DISP_cfg_max_string_length"
  // the program needs more ram with this changes
  #define _LCDML_cols        20                   // max cols
  #define _LCDML_rows        _LCDML_rows_max  // max rows 

  // scrollbar width
  #define _LCDML_scrollbar_w 6  // scrollbar width  

  // old defines with new content
  #define _LCDML_DISP_cols      _LCDML_cols
  #define _LCDML_DISP_rows      _LCDML_rows 

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

// general purpose utility macro
#define NUMELE(array) ( sizeof(array) / sizeof(array[0]) )

enum RESOLVERS {ABSOLUTE=-1,FINE=0,MEDIUM,COARSE,REFERENCE,HEADING,NtoS};

// *********************************************************************
// Prototypes
// *********************************************************************
  void lcdml_menu_display();
  void lcdml_menu_clear();
  void lcdml_menu_control();


// *********************************************************************
// Objects
// *********************************************************************
  LCDMenuLib2_menu LCDML_0 (255, 0, 0, NULL, NULL); // root menu element (do not change)
  LCDMenuLib2 LCDML(LCDML_0, _LCDML_DISP_rows, _LCDML_DISP_cols, lcdml_menu_display, lcdml_menu_clear, lcdml_menu_control);


// *********************************************************************
// LCDML MENU/DISP
// *********************************************************************
  // LCDML_add(id, prev_layer, new_num, lang_char_array, callback_function)
  // LCDMenuLib_addAdvanced(id, prev_layer, new_num, condition,   lang_char_array, callback_function, parameter (0-255), menu function type  )

  LCDML_addAdvanced ( 0 , LCDML_0   ,       1  ,  NULL,         ""               , mDyn_heading,     0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced ( 1 , LCDML_0   ,       2  ,  NULL,         ""               , mDyn_ntos,        0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced ( 2 , LCDML_0    ,      3  ,  NULL,         ""               , mDyn_absolute,    0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced ( 3 , LCDML_0   ,       4  ,  NULL,         ""               , mDyn_fine,        0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced ( 4 , LCDML_0   ,       5  ,  NULL,         ""               , mDyn_medium,      0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced ( 5 , LCDML_0   ,       6  ,  NULL,         ""               , mDyn_coarse,      0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function

  LCDML_add         ( 6 , LCDML_0        ,  7  , "Show settings"                 , mFunc_showSettings); 
  LCDML_add         ( 7 , LCDML_0        ,  8  , "Toggle auto"                   , mFunc_toggleAuto);                                                       // NULL = no menu function
  LCDML_add         ( 8 , LCDML_0        ,  9  , "Reset abs"                     , mFunc_resetAbs);                                                         // NULL = no menu function


  LCDML_addAdvanced ( 9 , LCDML_0         ,10  , NULL,          "Stepsize"       , mFunc_para,        0,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (10 , LCDML_0_10      , 1  , NULL,          ""               , mDyn_stepsize,     0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced (11 , LCDML_0_10      , 2  , NULL,          "Stepsize 1"     , mFunc_para,       51,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (12 , LCDML_0_10      , 3  , NULL,          "Stepsize 30"    , mFunc_para,       52,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (13 , LCDML_0_10      , 4  , NULL,          "Stepsize 900"   , mFunc_para,       53,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (14 , LCDML_0_10      , 5  , NULL,          "Stepsize 27000" , mFunc_para,       54,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_add         (15 , LCDML_0_10      , 6  , "Back"        , mFunc_back);

  LCDML_addAdvanced (16 , LCDML_0         ,17  , NULL,          "AutoDelay"      , mFunc_para,       0,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (17 , LCDML_0_17      , 1  , NULL,          ""               , mDyn_autodelay,   0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced (18 , LCDML_0_17      , 2  , NULL,          "AutoDelay 1"    , mFunc_para,      61,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (19 , LCDML_0_17      , 3  , NULL,          "AutoDelay 10"   , mFunc_para,      62,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (20 , LCDML_0_17      , 4  , NULL,          "AutoDelay 100"  , mFunc_para,      63,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (21 , LCDML_0_17      , 5  , NULL,          "AutoDelay 1000" , mFunc_para,      64,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_add         (22 , LCDML_0_17      , 6  , "Back"        , mFunc_back); 

  LCDML_addAdvanced (23 , LCDML_0   ,      13  ,  NULL,         ""               , mDyn_amplitude,   0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced (24 , LCDML_0   ,      14  ,  NULL,         ""               , mDyn_amplitude2,  0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function

  LCDML_addAdvanced (25 , LCDML_0         ,26  , NULL,          "Ref Phase"      , mFunc_para,       0,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (26 , LCDML_0_26      , 1  , NULL,          ""               , mDyn_refPhase,    0,            _LCDML_TYPE_dynParam);                   // NULL = no menu function
  LCDML_addAdvanced (27 , LCDML_0_26      , 2  , NULL,          "Ref Phase 0"    , mFunc_para,      71,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (28 , LCDML_0_26      , 3  , NULL,          "Ref Phase 90"   , mFunc_para,      72,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (29 , LCDML_0_26      , 4  , NULL,          "Ref Phase 180"  , mFunc_para,      73,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (30 , LCDML_0_26      , 5  , NULL,          "Ref Phase 270"  , mFunc_para,      74,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_add         (31 , LCDML_0_26      , 6  , "Back"        , mFunc_back); 

  // Example for dynamic content
  // 1. set the string to ""
  // 2. use type  _LCDML_TYPE_dynParam   instead of    _LCDML_TYPE_default
  // this function type can not be used in combination with different parameters
  // LCDMenuLib_addAdvanced(id, prev_layer,  new_num, condition,   lang_char_array, callback_function, parameter (0-255), menu function type  )

  // Example for conditions (for example for a screensaver)
  // 1. define a condition as a function of a boolean type -> return false = not displayed, return true = displayed
  // 2. set the function name as callback (remove the braces '()' it gives bad errors)
  // LCDMenuLib_addAdvanced(id, prev_layer,     new_num, condition,   lang_char_array, callback_function, parameter (0-255), menu function type  )
  LCDML_addAdvanced (32 ,      LCDML_0         , 12  ,    COND_hide,  "screensaver"        , mFunc_screensaver,        0,   _LCDML_TYPE_default);       // this menu function can be found on "LCDML_display_menuFunction" tab

  // Example function for event handling (only serial output in this example)  
//  LCDML_add         (23 ,      LCDML_0         , 8  , "Event Handling"                 , mFunc_exampleEventHandling);  // this menu function can be found on "LCDML_display_menuFunction" tab

  // ***TIP*** Try to update _LCDML_DISP_cnt when you add a menu element.

  // menu element count - last element id
  // this value must be the same as the last menu element
  #define _LCDML_DISP_cnt    32

  // create menu
  LCDML_createMenu(_LCDML_DISP_cnt);

// *********************************************************************
// I/O pin definitions
// *********************************************************************


#define ButtonA             12    // PWM 6A
#define ButtonB             13    // PWM 6B
//#define ButtonX             14    // PWM 7A
//#define ButtonY             15    // PWM 7B

#define pinOpSync           14
#define pinIpTrig           15

/* these are defined in User_Setup.h
#define TFT_DC              16
#define TFT_CS              17
#define TFT_RST             -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_SCLK            18
#define TFT_MOSI            19
//define TFT_MISO           ?? // not needed
*/

#define encoder_button_pin  20 
#define ENCODER_PIN_IN2     21
#define ENCODER_PIN_IN1     22

// Setup a RotaryEncoder with 2 steps per latch for the 2 signal input pins:
RotaryEncoder encoder(ENCODER_PIN_IN1, ENCODER_PIN_IN2, RotaryEncoder::LatchMode::TWO03);

// unavailable: SMPS        23
// unavailable: Vbus sense  24

#define pinLED              25    // On-board BUILTIN_LED

// ADC pins now unused - and available
#define unused26            26    // adc0
#define unused27            27    // adc1
#define unused28            28    // adc2

TFT_eSPI display = TFT_eSPI();

#define LED_ON              LOW
#define LED_OFF             HIGH

//                          Fine Medium Coarse Ref Heading NtoS
// PWM channel              0A 0B 1A 1B 2A 2B 3A 3B 4A 4B 5A 5B
uint32_t PWM_Pins[]     = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11  };
#define NUM_OF_PINS NUMELE(PWM_Pins)
RP2040_PWM* PWM_Instance[NUM_OF_PINS];

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

#define NUM_SINE_ELEMENTS 100        // steps per cycle of 400Hz wave
// choose 396 to select lower of two possible frequencies, 400 selects 402
#define SINEWAVE_FREQUENCY_HZ 400   // target frequency
#define SYNC_OFFSET_COUNT 4         // sin table index to use when sync pulse detected
#define PULSE_OFFSET_COUNT 2        // sine table index to use for sync output pulse

struct sine_table_ {
  int num_elements=NUM_SINE_ELEMENTS;
  int sinewave_frequency=SINEWAVE_FREQUENCY_HZ;
  int sync_offset=SYNC_OFFSET_COUNT;
  int16_t levels[NUM_SINE_ELEMENTS];
  uint16_t timer_interval;              // calculated in build sinetable function
  float stepsize;                       // calculated in build sinetable function
} sine_table;

int stepidx=1;
int steplist[] = {1,30,900};

int delayidx=1;
float delaylist[] = {10.0 ,100.0, 1000.0 };

#define TIMER1_INTERVAL_MS        20
#define DEBOUNCING_INTERVAL_MS    100
#define LONG_PRESS_INTERVAL_MS    1000

// Init RPI_PICO_Timer, can use any from 0-15 pseudo-hardware timers
RPI_PICO_Timer ITimer0(0);

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
  struct resolver_ resolvers[6];
  long absolute;                // moving map absolute position
  int  autostep;                // step size for automatic (or encoder) movement
  bool  automatic;               // enable automatic map movement (E/W)
  int  autodelay;               // time between automatic steps
};        



struct transport_ transport = {
// name,      instance, pin, duty1, instance, pin, duty2, angle, amp1, amp2, duty1, duty2  
  "Fine",     "sin",  NULL,     PWM_Pins[ 0],   0.0,   "cos",  NULL,     PWM_Pins[ 1],   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Medium",   "sin",  NULL,     PWM_Pins[ 2],   0.0,   "cos",  NULL,     PWM_Pins[ 3],   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Coarse",   "sin",  NULL,     PWM_Pins[ 4],   0.0,   "cos",  NULL,     PWM_Pins[ 5],   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Reference","ref+", NULL,     PWM_Pins[ 6],   0.0,   "ref-", NULL,     PWM_Pins[ 7],   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Heading",  "sin" , NULL,     PWM_Pins[ 8],   0.0,   "cos",  NULL,     PWM_Pins[ 9],   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "NtoS",     "sin" , NULL,     PWM_Pins[10],   0.0,   "cos",  NULL,     PWM_Pins[11],   0.0,   0.0,   0,0,   0.0,    0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  0L,                                                                          // absolute
  10,                                                                          // autostep
  false,                                                                       // automatic
  10,                                                                          // autodelay
};


char dashLine[] = "=====================================================================";

// index into 400Hz sine table for PWM frequency generation
volatile int step_index = 0; 

// menu variables TODO use transport structure variables
float autostep =  30;
bool  automatic = false;
int   autodelay = 10;
float absolute =  0.0;
float fine = 0.0;
float medium = 0.0;
float coarse = 0.0;
float heading = 0.0;
float ntos = 0.0;

#define AMPLITUDE_FS 16.33    // volts rms ful scale output, measured
#define DIV_CONST 768         // divisor for desired 12 volt ouput
#define REF_CONST 1136        // divisor for 8 volt reference output
#define DIV_FACT  560         // multiplier, for menu voltage out calculation - found by experiment
int amplitude_div=DIV_CONST;
int amplitude_ref=REF_CONST;  // default reference amplitude is just 8v to limit amplifier power dissipation


//#define H6_DISPLAY
/*
  when H6_DISPLY is defined softwre has following changes
  Heading outputs connects to roll inputs, resolver calc changed to synchro calc
  NtoS outputs connects to pitch inputs, resolver calc changed to synchro calc
  Fine control output on Coarse sin output connects to verticle needle input, - sin output
  Medium control output on Coarse cos output connects to horizontal needle input, - sin output 
  Sync input is fed from isolated 400Hz zero crossing detector fed by 400Hz 115V display power supply.
*/


//-------------------------------------------------------------------------------------------

void build_sintable(void)
{
  int n;
  sine_table.stepsize = 360.0 / sine_table.num_elements;
  sine_table.timer_interval = 1E6 / sine_table.sinewave_frequency / sine_table.num_elements;

  for(n=0; n<sine_table.num_elements; n++)
  {
    sine_table.levels[n] = int(sin(M_PI*(n*sine_table.stepsize)/180.0) * 512.0);          // table entries -512 to +512
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
  static bool pulse_sent=false;
  (void) t;
  int16_t int_sine_step_value;
  uint16_t dc_levels[NUM_OF_PINS];

  if(step_index >= sine_table.num_elements)
  {
    step_index = 0;
  }

#define MID_POINT_INT 500
  int_sine_step_value = ( sine_table.levels[step_index] ); 

// manual level is integer 0 to 1000 (actually 800 due to output voltage limitation problem)  
// sine_table is +- 512 ( 10 bits )
// scale is +- 500 (10 bits )
// product is 20 bits - needs to be 10 - so divide by 10 bits 
  dc_levels[0] = MID_POINT_INT +    transport.resolvers[0].amplitude[0]  * int_sine_step_value / amplitude_div;
  PWM_Instance[0]->setPWM_manual_Fast(PWM_Pins[0], dc_levels[0]);
  dc_levels[1] = MID_POINT_INT +    transport.resolvers[0].amplitude[1]  * int_sine_step_value / amplitude_div;
  PWM_Instance[1]->setPWM_manual_Fast(PWM_Pins[1], dc_levels[1]);

  dc_levels[2] = MID_POINT_INT +    transport.resolvers[1].amplitude[0]  * int_sine_step_value / amplitude_div;
  PWM_Instance[2]->setPWM_manual_Fast(PWM_Pins[2], dc_levels[2]);
  dc_levels[3] = MID_POINT_INT +    transport.resolvers[1].amplitude[1]  * int_sine_step_value / amplitude_div;
  PWM_Instance[3]->setPWM_manual_Fast(PWM_Pins[3], dc_levels[3]);

  dc_levels[4] = MID_POINT_INT +    transport.resolvers[2].amplitude[0]  * int_sine_step_value / amplitude_div;
  PWM_Instance[4]->setPWM_manual_Fast(PWM_Pins[4], dc_levels[4]);
  dc_levels[5] = MID_POINT_INT +    transport.resolvers[2].amplitude[1]  * int_sine_step_value / amplitude_div;
  PWM_Instance[5]->setPWM_manual_Fast(PWM_Pins[5], dc_levels[5]);

  dc_levels[6] = MID_POINT_INT +   transport.resolvers[3].amplitude[0] * int_sine_step_value / amplitude_ref; // reference +sinewave output
  PWM_Instance[6]->setPWM_manual_Fast(PWM_Pins[6], dc_levels[6]);
  dc_levels[7] = MID_POINT_INT +   transport.resolvers[3].amplitude[1] * int_sine_step_value / amplitude_ref; // reference -sinewave output
  PWM_Instance[7]->setPWM_manual_Fast(PWM_Pins[7], dc_levels[7]);

  dc_levels[8] = MID_POINT_INT +   transport.resolvers[4].amplitude[0] * int_sine_step_value / amplitude_div;
  PWM_Instance[8]->setPWM_manual_Fast(PWM_Pins[8], dc_levels[8]);
  dc_levels[9] = MID_POINT_INT +   transport.resolvers[4].amplitude[1] * int_sine_step_value / amplitude_div;
  PWM_Instance[9]->setPWM_manual_Fast(PWM_Pins[9], dc_levels[9]);

  dc_levels[10] = MID_POINT_INT +   transport.resolvers[5].amplitude[0] * int_sine_step_value / amplitude_div;
  PWM_Instance[10]->setPWM_manual_Fast(PWM_Pins[10], dc_levels[10]);
  dc_levels[11] = MID_POINT_INT +   transport.resolvers[5].amplitude[1] * int_sine_step_value / amplitude_div;
  PWM_Instance[11]->setPWM_manual_Fast(PWM_Pins[11], dc_levels[11]);

  

  if(step_index >= PULSE_OFFSET_COUNT)
  {
    if(pulse_sent == false)
    {
      digitalWrite(pinOpSync, 0);
      pulse_sent=true;
    }
  }
  else
  {
    if(pulse_sent == true)
    {
      digitalWrite(pinOpSync, 1);    
      pulse_sent = false;
    }
  }

//  digitalWrite(pinOpSync,step_index < PULSE_OFFSET_COUNT); // sync pulse output

  step_index++;
  return true;
}

void ref_phase_update(long bump)
{

  sine_table.sync_offset += bump;  
}


// ISR for frequency sync input
void syncInput(void) {
    ITimer0.stopTimer();
    step_index=sine_table.sync_offset;
    ITimer0.restartTimer();
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
void abs2res(long bump)
{
  float target;
  absolute += bump;
  if(absolute < 0) absolute = 0;
  coarse = transport.resolvers[2].angle = (absolute / ratio2) - offset_coarse;
  medium = transport.resolvers[1].angle = fmod(absolute / ratio1, 360);
  fine   = transport.resolvers[0].angle = fmod(absolute,          360);

// fine
  target = fmod(transport.resolvers[0].angle, 360) * M_PI/180.0;
  transport.resolvers[0].amplitude[0] = sin(target) * 500;
  transport.resolvers[0].amplitude[1] = cos(target) * 500;
// medium
  target = fmod(transport.resolvers[1].angle, 360) * M_PI/180.0;
  transport.resolvers[1].amplitude[0] = sin(target) * 500;
  transport.resolvers[1].amplitude[1] = cos(target) * 500;
// coarse
  target = fmod(transport.resolvers[2].angle, 360) * M_PI/180.0;
  transport.resolvers[2].amplitude[0] = sin(target) * 500;
  transport.resolvers[2].amplitude[1] = cos(target) * 500;
 // reference
  transport.resolvers[3].amplitude[0] = 500.0;
  transport.resolvers[3].amplitude[1] = -500.0;
}

void fine2res(long bump)
{
  float target;
  fine = fmod(fine+bump,360);
  transport.resolvers[0].angle  =   fine;
  target = fmod(transport.resolvers[0].angle, 360) * M_PI/180.0;
#ifndef H6_DISPLAY 
  transport.resolvers[0].amplitude[0] = sin(target) * 500;
  transport.resolvers[0].amplitude[1] = cos(target) * 500;
#else
  transport.resolvers[2].amplitude[0] = -sin(target) * 500;
#endif
}
void medium2res(long bump)
{
  float target;
  medium = fmod(medium+bump,360);

  transport.resolvers[1].angle  =   medium;
  target = fmod(transport.resolvers[1].angle, 360) * M_PI/180.0;
#ifndef H6_DISPLAY
  transport.resolvers[1].amplitude[0] = sin(target) * 500;
  transport.resolvers[1].amplitude[1] = cos(target) * 500;
#else
  transport.resolvers[2].amplitude[1] = -sin(target) * 500;
#endif
}
void coarse2res(long bump)
{
  float target;
  coarse = fmod(coarse+bump,360);

  transport.resolvers[2].angle  =   coarse;
  target = fmod(transport.resolvers[2].angle, 360) * M_PI/180.0;
  transport.resolvers[2].amplitude[0] = sin(target) * 500;
  transport.resolvers[2].amplitude[1] = cos(target) * 500;
}

void heading2res(long bump)
{
  float target;
  heading = fmod(heading+bump,360);
  transport.resolvers[4].angle  =   heading;
#ifndef H6_DISPLAY
  // output to resolver receiver
  target = fmod(transport.resolvers[4].angle, 360) * M_PI/180.0;
  transport.resolvers[4].amplitude[0] = sin(target) * 500;
  transport.resolvers[4].amplitude[1] = cos(target) * 500;
#else
  // output to synchro receiver
  target = fmod(transport.resolvers[4].angle+120, 360) * M_PI/180.0;
  transport.resolvers[4].amplitude[0] = sin(target) * 500;

  target = -fmod(transport.resolvers[4].angle+240, 360) * M_PI/180.0;
  transport.resolvers[4].amplitude[1] = sin(target) * 500;
#endif  
}

void ntos2res(long bump)
{
  float target;
  ntos = fmod(ntos+bump,360);
  transport.resolvers[5].angle  =   ntos;
#ifndef H6_DISPLAY
  target = fmod(transport.resolvers[5].angle, 360) * M_PI/180.0;
  transport.resolvers[5].amplitude[0] = sin(target) * 500;
  transport.resolvers[5].amplitude[1] = cos(target) * 500;
#else
  // output to synchro receiver
  target = fmod(transport.resolvers[5].angle+120, 360) * M_PI/180.0;
  transport.resolvers[5].amplitude[0] = sin(target) * 500;

  target = -fmod(transport.resolvers[5].angle+240, 360) * M_PI/180.0;
  transport.resolvers[5].amplitude[1] = sin(target) * 500;
#endif
}

// screensaver
void displayDisable(void)
{
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_BLACK);
}

// show common system settings and individual PWM channel settings
void displayUpdate(void)
{
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_GREEN);
  display.setTextSize(2);
  display.println();
  Serial.println("=====================");

// show details for individual PWM channel settings
  for (int i = 0; i < 6; i++)
  {
  #if 1
    Serial.println();


    Serial.print("Channel # ");
    Serial.print(i);
    Serial.print(", ");
    Serial.println(transport.resolvers[i].name);

    Serial.print("Angle = ");
    Serial.println(transport.resolvers[i].angle);

    Serial.print("GPIO#s = ");
    Serial.print(transport.resolvers[i].PWM[0].PWM_pin);
    Serial.print(", ");
    Serial.println(transport.resolvers[i].PWM[1].PWM_pin);

    Serial.print("Scales ");
    Serial.print(transport.resolvers[i].amplitude[0]);
    Serial.print(", ");
    Serial.println(transport.resolvers[i].amplitude[1]);

  #endif
    display.setCursor(0, 0 + i * 20);
    display.print(transport.resolvers[i].name);
    display.print(" = ");
    display.print(transport.resolvers[i].angle);
    display.println("\xF7"); // degree symbol

#if 0
    display.print(transport.resolvers[i].PWM[0].name);
    display.print("=");
    display.print(transport.resolvers[i].level[0],1);
    display.print("% ");
    display.print(transport.resolvers[i].PWM[1].name);
    display.print("=");
    display.print(transport.resolvers[i].level[1],1);
    display.println("%");
#endif    
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
  Serial.println(amplitude_div);

  Serial.print("Absolute = ");
  Serial.println(absolute);
#endif
  display.println();
  display.print("Absolute = ");
  display.println(absolute);
  display.println();

  display.print("Step =     ");
  display.println(autostep);

  display.print("Delay =    ");
  display.println(autodelay);

  display.print("Auto =     ");
  display.println(automatic);
}

// setup -----------------------------------------------------------------------------------------

void setup()
{
  display.init();           // Init ST7789 240x240
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_GREEN);
  display.setTextSize(3);
  display.println("Resolver PWM");

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
#if 0
  pinMode(ButtonA, INPUT_PULLUP);
  pinMode(ButtonB, INPUT_PULLUP);
  pinMode(ButtonX, INPUT_PULLUP);
#endif

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

  Serial.println("Note fine step for 1 degree is absolute 1");
  Serial.println("Note medium step for 1 degree is absolute 30");
  Serial.println("Note coarse step for 1 degree is absolute 900");
 
  abs2res(0);
  coarse2res(0);
  medium2res(0);
  fine2res(0);
  heading2res(0);
  ntos2res(0);
  displayUpdate();

    /* INIT LCDML */
    // LCDMenuLib Setup
    LCDML_setup(_LCDML_DISP_cnt);

    // Enable Menu Rollover
    LCDML.MENU_enRollover();

    // Enable Screensaver (screensaver menu function, time to activate in ms)
    //LCDML.SCREEN_enable(mFunc_screensaver, 60000); // set to 60 seconds
    LCDML.SCREEN_disable();

    // Some needful methods

    // You can jump to a menu function from anywhere with
    //LCDML.OTHER_jumpToFunc(mFunc_p2); // the parameter is the function name
}

// activated when IMER_ INTERRUPT_ DEBUG > 2

// sometimes compiler complains when this function declared with void return type
/*int*/ void printPWMInfo(RP2040_PWM* PWM_Instance)
{
  uint32_t div = PWM_Instance->get_DIV();
  uint32_t top = PWM_Instance->get_TOP();

  // PWM_Freq = ( F_CPU ) / [ ( TOP + 1 ) * ( DIV + DIV_FRAC/16) ]
  PWM_LOGINFO1("Actual PWM Frequency = ",
               PWM_Instance->get_freq_CPU() / ( (PWM_Instance->get_TOP() + 1) * (PWM_Instance->get_DIV() ) ) );

  PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance->get_freq_CPU());

//  return 0;
}

// loop -----------------------------------------------------------------------------------------

void loop()
{
  static int refresh_time=0;
  static int old_absolute=0;

  static int pos = 0;

  encoder.tick(); // used by menu functions
  LCDML.loop();   // lcd and serial user interface


  if(automatic)   // advance absolute film position
  {
    static int del_count=0;
    int del_value;
    del_value=millis();
    refresh_time=del_value; // inhibit auto display refresh (force display update with button press)
    if(del_value - del_count > autodelay)
    {
      del_count=del_value;
      //abs2res(autostep);
      heading2res(autostep);
      //ntos2res(autostep);

    }
  }

/*
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
*/

}
// end