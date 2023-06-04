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
#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     4

// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "RPi_Pico_TimerInterrupt.h"
#include <math.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <CommandParser.h>
typedef CommandParser<> MyCommandParser;
MyCommandParser parser;

#define TFT_CS        17
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        16
#define TFT_MOSI      19
#define TFT_SCLK      18

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

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
#define pinOpSync     7    // 
#define pinIpTrig     5

#define ButtonA       12    // PWM 6A
#define ButtonB       13    // PWM 6B
#define ButtonX       14    // PWM 7A
#define ButtonY       15    // PWM 7B

#define pin1          0     // PWM channel 0A
//#define pin2          1     // PWM channel 0B
#define pin4          2     // PWM channel 1A
//#define pin5          3     // PWM channel 1B
#define pin6          4     // PWM channel 2A
//#define pin7          5     // PWM channel 2B
#define pin9          6     // PWM channel 3A
//#define pin10         7     // PWM channel 3B
#define pin11         8     // PWM channel 4A (mot1-)
//#define pin12         9     // PWM channel 4B (mot1+)
#define pin14         10    // PWM channel 5A (mot2-)
//#define pin15         11    // PWM channel 5B (mot2+)
//#define pin16         12    // PWM channel 6A (button A)
//#define pin17         13    // PWM channel 6B (button B)
#define pin34         28    // PWM channel 6A 
//#define pin19         14    // PWM channel 7A
//#define pin20         15    // PWM channel 7B


uint32_t PWM_Pins[]     = { 0, 2, 4, 6, 1, 3, /*8, 10,*/ 28 };

#define NUM_OF_PINS       ( sizeof(PWM_Pins) / sizeof(uint32_t) )

float dutyCycle = 50.0f;

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
bool buttonAPress = false;
bool buttonBPress = false;
unsigned long buttonATime = 0; // To prevent debounce
unsigned long buttonBTime = 0; // To prevent debounce

float frequency;

// You can select any value
uint16_t PWM_data_idle = 124;

#define TIMER0_INTERVAL            78 //39

// Init RPI_PICO_Timer, can use any from 0-15 pseudo-hardware timers
RPI_PICO_Timer ITimer0(0);
//RPI_PICO_Timer ITimer1(1);


char dashLine[] = "=============================================================";

static int step256 = 0;
float target = 3.0;

float scale1;
float scale2;
float scale3;
float scale4;
float scale5;
float scale6;

float level1;
float level2;
float level3;
float level4;
float level5;
float level6;

float test;
int step=10;
int automatic=0;
long absolute=0;

float angle0;
float angle1;
float angle2;


bool TimerHandler0(struct repeating_timer *t)
{ 
  (void) t;
  float temp;
//  static bool toggle0 = false;
  step256+=8;
  if(step256  > 255) step256 = 0; // should be >255, kludge to trim frequency to 400 Hz
// center resultant waveform around 50% PWM full scale is 100.0 * ( sine256[step256%256] / 256.0f )

  temp = ( sine256[step256%256] / 256.0f );

  level1 = ( 50.0 - scale1 ) + ( 2 * scale1 ) * temp;
  level2 = ( 50.0 - scale2 ) + ( 2 * scale2 ) * temp;
  PWM_Instance[0]->setPWM_DCPercentage_manual(PWM_Pins[0], level1);
  PWM_Instance[1]->setPWM_DCPercentage_manual(PWM_Pins[1], level2);

  level3 = ( 50.0 - scale3 ) + ( 2 * scale3 ) * temp;
  level4 = ( 50.0 - scale4 ) + ( 2 * scale4 ) * temp;
  PWM_Instance[2]->setPWM_DCPercentage_manual(PWM_Pins[2], level3);
  PWM_Instance[3]->setPWM_DCPercentage_manual(PWM_Pins[3], level4);

  level5 = ( 50.0 - scale5 ) + ( 2 * scale5 ) * temp;
  level6 = ( 50.0 - scale6 ) + ( 2 * scale6 ) * temp;
  PWM_Instance[4]->setPWM_DCPercentage_manual(PWM_Pins[4], level5);
  PWM_Instance[5]->setPWM_DCPercentage_manual(PWM_Pins[5], level6);

  test = 100.0 * temp;
  PWM_Instance[6]->setPWM_DCPercentage_manual(PWM_Pins[6], test);

  //timer interrupt toggles pin LED_BUILTIN
  //digitalWrite(PIN_T0, toggle0);
  //toggle0 = !toggle0;

  digitalWrite(pinOpSync,step256==0);
  //uint16_t level2=1000/256*sine256[(index+80)%256];

  return true;
}

void syncInput(void) {
  step256=0;
}

void buttonAPressed() {
  //Set timer to work for your loop code time   
//  if (millis() - buttonTime > 250) {
    //button press ok
    buttonAPress= true;
//  }
//  buttonTime = millis();
}
void buttonBPressed() {
  //Set timer to work for your loop code time   
//  if (millis() - buttonTime > 250) {
    //button press ok
    buttonBPress= true;
//  }
//  buttonTime = millis();
}

#if 0
void cmd_test(MyCommandParser::Argument *args, char *response) {
  Serial.print("string: "); Serial.println(args[0].asString);
  Serial.print("double: "); Serial.println(args[1].asDouble);
  Serial.print("int64: "); Serial.println((int32_t)args[2].asInt64); // NOTE: on older AVR-based boards, Serial doesn't support printing 64-bit values, so we'll cast it down to 32-bit
  Serial.print("uint64: "); Serial.println((uint32_t)args[3].asUInt64); // NOTE: on older AVR-based boards, Serial doesn't support printing 64-bit values, so we'll cast it down to 32-bit
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}
#endif

const int ratio1 = 30;
const int ratio2 = ratio1*30;
#define offset_coarse 0  // range on coarse is -90 to +90, so offset is +90 (to verify)

unsigned long res2abs(int fine, int medium, int coarse)
{
// use top bits of each resolver, get bottom bits from next in chain  
    return      (fine   % ratio1)     
    + (ratio1 * (medium % ratio1))   
    + (ratio2 * (coarse % ratio2));
}

void abs2res(long absolute, float *fine, float *medium, float *coarse)
{
        *coarse = (absolute / ratio2) - offset_coarse;
        *medium = (absolute / ratio1) % 360;
        *fine  =   absolute           % 360;

}        

void  printDetails(const char * name, int index, float angle, float scaleA, float scaleB, float levelA, float levelB) 
{
#if 0  
  Serial.println();
  Serial.println(name);
  Serial.print("Angle=");
  Serial.println(angle);
  Serial.print("Scale ");
  Serial.print(scaleA);
  Serial.print(", ");
  Serial.println(scaleB);
#endif

  tft.setCursor(0, 0 + index * 55);
  tft.println(name);
  tft.print("Angle= ");
  tft.println(angle);
  tft.print("Scale ");
  tft.print(scaleA);
  tft.print(", ");
  tft.println(scaleB);
}

void displayUpdate(void)
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  printDetails("Fine", 0, angle0, scale1, scale2, level1, level2); 
  printDetails("Medium", 1, angle1, scale3, scale4, level3, level4); 
  printDetails("Coarse", 2, angle2, scale5, scale6, level5, level6); 

  absolute = res2abs(angle0, angle1, angle2);
  tft.println();

  tft.print("Absolute= ");
  tft.println(absolute);

  tft.print("Step= ");
  tft.println(step);

  tft.print("Automatic= ");
  tft.println(automatic);
}

void cmd_abs(MyCommandParser::Argument *args, char *response) {
  Serial.println();
  Serial.print("double: "); Serial.println(args[0].asDouble);
  absolute = args[0].asDouble;
  abs2res(absolute, &angle0, &angle1, &angle2);
  anglesUpdate();
#if 0
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.print("Absolute= ");
  tft.println(absolute_);

  tft.print("Fine ");
  tft.println(angle0);
  tft.print("Medium ");
  tft.println(angle1);
  tft.print("Coarse ");
  tft.println(angle2);
#endif
   displayUpdate();
}

void cmd_step(MyCommandParser::Argument *args, char *response) {
  Serial.println();
  Serial.print("double: "); Serial.println(args[0].asDouble);
  step = args[0].asDouble;
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}

void cmd_automatic(MyCommandParser::Argument *args, char *response) {
  Serial.println();
  Serial.print("double: "); Serial.println(args[0].asDouble);
  automatic = args[0].asDouble;
  displayUpdate();
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}


void anglesUpdate(void)
{
  target = fmod(angle0, 360) * M_PI/180.0;
  scale1=(sin(target)*50.0);
  scale2=(cos(target)*50.0);

  target = fmod(angle1, 360) * M_PI/180.0;
  scale3=(sin(target)*50.0);
  scale4=(cos(target)*50.0);

  target = fmod(angle2, 360) * M_PI/180.0;
  scale5=(sin(target)*50.0);
  scale6=(cos(target)*50.0);

}

void cmd_fin(MyCommandParser::Argument *args, char *response) {
  Serial.println();
  Serial.print("double: "); Serial.println(args[0].asDouble);
  angle0 = args[0].asDouble;
  target = fmod(angle0, 360) * M_PI/180.0;
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
  scale1=(sin(target)*50.0);
  scale2=(cos(target)*50.0);
  displayUpdate();
}

void cmd_med(MyCommandParser::Argument *args, char *response) {
  Serial.println();
  Serial.print("double: "); Serial.println(args[0].asDouble);
  angle1 = args[0].asDouble;
  target = fmod(angle1, 360) * M_PI/180.0;
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
  scale3=(sin(target)*50.0);
  scale4=(cos(target)*50.0);
  displayUpdate();
}


void cmd_coa(MyCommandParser::Argument *args, char *response) {
  Serial.println();
  Serial.print("double: "); Serial.println(args[0].asDouble);
  angle2 = args[0].asDouble;
  target = fmod(angle2, 360) * M_PI/180.0;
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
  scale5=(sin(target)*50.0);
  scale6=(cos(target)*50.0);
  displayUpdate();
}

#if 0
void tftPrintTest() {
  float p = 3.1415926;
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  tft.println();
//  delay(1500);
//  tft.setCursor(0, 0);
//  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}
#endif

void setup()
{
  tft.init(240, 240);           // Init ST7789 240x240
  tft.setRotation(2);
  tft.setSPISpeed(40000000);
  tft.fillScreen(ST77XX_BLACK);
//  tftPrintTest();
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Resolver PWM");


  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  delay(100);

  Serial.print(F("\nStarting TimerInterruptTest on ")); Serial.println(BOARD_NAME);
  Serial.println(RPI_PICO_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  Serial.print(F("\nStarting PWM_Waveform_Fast on "));
  Serial.println(BOARD_NAME);
  Serial.println(RP2040_PWM_VERSION);

  pinMode(pinOpSync, OUTPUT);

  pinMode(pinIpTrig, INPUT_PULLUP);
  pinMode(ButtonA, INPUT_PULLUP);
  pinMode(ButtonB, INPUT_PULLUP);
  pinMode(ButtonX, INPUT_PULLUP);
//  attachInterrupt(pinIpTrig, syncInput, RISING);
  attachInterrupt(pinIpTrig, syncInput, FALLING);
  attachInterrupt(ButtonA, buttonAPressed, RISING);
  attachInterrupt(ButtonB, buttonBPressed, RISING);
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
    Serial.print(dutyCycle);

    PWM_Instance[index] = new RP2040_PWM(PWM_Pins[index], frequency, dutyCycle);

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

//  parser.registerCommand("TEST", "sdiu", &cmd_test);
//  parser.registerCommand("fin", "d", &cmd_target);
  parser.registerCommand("fin", "d", &cmd_fin);
  parser.registerCommand("med", "d", &cmd_med);
  parser.registerCommand("coa", "d", &cmd_coa);
  parser.registerCommand("abs", "d", &cmd_abs);
  parser.registerCommand("step", "d", &cmd_step);
  parser.registerCommand("automatic", "d", &cmd_automatic);
//  Serial.println("registered command: TEST <string> <double> <int64> <uint64>");
//  Serial.println("example: TEST \"\\x41bc\\ndef\" -1.234e5 -123 123");
  Serial.println("registered command: fin <double> ");
  Serial.println("registered command: med <double> ");
  Serial.println("registered command: coa <double> ");
  Serial.println("registered command: abs <double> ");
  Serial.println("registered command: step <double> ");
  Serial.println("registered command: automatic <double> ");
  Serial.println("to set resolver angles in degrees");

  Serial.println(dashLine);

  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL, TimerHandler0))
  {
    Serial.print(F("Starting ITimer0 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));

  Serial.println(dashLine);

  target = 0.707107;
  scale1=(sin(target)*50.0);
  scale2=(cos(target)*50.0);
  scale3=(sin(target)*50.0);
  scale4=(cos(target)*50.0);
  scale5=(sin(target)*50.0);
  scale6=(cos(target)*50.0);

  Serial.println();
  Serial.print("Scales: ");
  Serial.print(scale1);
  Serial.print(", ");
  Serial.print(scale2);
  Serial.print(", ");
  Serial.print(scale3);
  Serial.print(", ");
  Serial.print(scale4);
  Serial.print(", ");
  Serial.print(scale5);
  Serial.print(", ");
  Serial.print(scale6);
  Serial.println();

//  tftPrintTest();
  anglesUpdate();
  displayUpdate();

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
  
  if (Serial.available()) {
    char line[128];
    size_t lineLength = Serial.readBytesUntil('\n', line, 127);
    line[lineLength] = '\0';

    char response[MyCommandParser::MAX_RESPONSE_SIZE];
    parser.processCommand(line, response);
    Serial.println(response);
  }

  if(buttonAPress)
  {
  //  Serial.println(F("Button A Pressed"));
    buttonAPress= false;
//    absolute += step;
    automatic = 1;
//    abs2res(absolute, &angle0, &angle1, &angle2);
//    anglesUpdate();
    displayUpdate();
  }

  if(buttonBPress)
  {
  //  Serial.println(F("Button B Pressed"));
    buttonBPress= false;
    //absolute -= step;
    automatic = 0;
    //abs2res(absolute, &angle0, &angle1, &angle2);
    //anglesUpdate();
    displayUpdate();
  }

  if(automatic)
  {
    absolute += step;   
    abs2res(absolute, &angle0, &angle1, &angle2); 
    anglesUpdate();
//    displayUpdate();
  }

  delay(100);


  // Use at low freq to check
  printPWMInfo(PWM_Instance[0]);
  printPWMInfo(PWM_Instance[1]);
  // delay something here between data
  
}
