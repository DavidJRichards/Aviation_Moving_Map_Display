/* ===================================================================== *
 *                                                                       *
 * Menu Callback Function                                                *
 *                                                                       *
 * ===================================================================== *
 *
 * EXAMPLE CODE:

// *********************************************************************
void your_function_name(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    //LCDML_UNUSED(param);
    // setup
    // is called only if it is started

    // starts a trigger event for the loop function every 100 milliseconds
    LCDML.FUNC_setLoopInterval(100);

    // uncomment this line when the menu should go back to the last called position
    // this could be a cursor position or the an active menu function
    // GBA means => go back advanced
    //LCDML.FUNC_setGBA() 

    //
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop
    // is called when it is triggered
    // - with LCDML_DISP_triggerMenu( milliseconds )
    // - with every button or event status change

    // uncomment this line when the screensaver should not be called when this function is running
    // reset screensaver timer
    //LCDML.SCREEN_resetTimer();

    // check if any button is pressed (enter, up, down, left, right)
    if(LCDML.BT_checkAny()) {
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // loop end
    // you can here reset some global vars or delete it
    // this function is always called when the functions ends.
    // this means when you are calling a jumpTo ore a goRoot function
    // that this part is called before a function is closed
  }
}


 * ===================================================================== *
 */

// *********************************************************************
void mFunc_information(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    // clear lcd
    display.fillScreen(_LCDML_BACKGROUND_COLOR);
    // set text color / Textfarbe setzen
    display.setTextColor(_LCDML_TEXT_COLOR);  
    // set text size / Textgroesse setzen
    display.setTextSize(_LCDML_FONT_SIZE);
  
    display.setCursor(0, _LCDML_FONT_H * 0); // line 0
    display.println(F("To close this")); 
    display.setCursor(0, _LCDML_FONT_H * 1); // line 1
    display.println(F("function press")); 
    display.setCursor(0, _LCDML_FONT_H * 2); // line 2
    display.println(F("any button or use")); 
    display.setCursor(0, _LCDML_FONT_H * 3); // line 3
    display.println(F("back button")); 
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if(LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      // LCDML_goToMenu stops a running menu function and goes to the menu
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}

// *********************************************************************
uint8_t g_func_timer_info = 0;  // time counter (global variable)
unsigned long g_timer_1 = 0;    // timer variable (global variable)

// *********************************************************************
void mFunc_showSettings(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    // clear lcd
    display.fillScreen(_LCDML_BACKGROUND_COLOR);
    // set text color / Textfarbe setzen
    display.setTextColor(_LCDML_TEXT_COLOR);  
    // set text size / Textgroesse setzen
    display.setTextSize(_LCDML_FONT_SIZE);
  
    display.setCursor(0, _LCDML_FONT_H * 0); // line 0
    display.println(F("To close this")); 
    display.setCursor(0, _LCDML_FONT_H * 1); // line 1
    display.println(F("function press")); 
    display.setCursor(0, _LCDML_FONT_H * 2); // line 2
    display.println(F("any button or use")); 
    display.setCursor(0, _LCDML_FONT_H * 3); // line 3
    display.println(F("back button")); 

    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds

    LCDML.TIMER_msReset(g_timer_1);
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function

    // reset screensaver timer
    LCDML.SCREEN_resetTimer();

     // this function is called every 100 milliseconds

    // this method checks every 1000 milliseconds if it is called
    if(LCDML.TIMER_ms(g_timer_1, 1000))
    {
      g_timer_1 = millis();
      g_func_timer_info--;                // increment the value every second
      displayUpdate();
    }

  }


  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if(LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      // LCDML_goToMenu stops a running menu function and goes to the menu
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}


// *********************************************************************
//uint8_t g_func_timer_info = 0;  // time counter (global variable)
//unsigned long g_timer_1 = 0;    // timer variable (global variable)
void mFunc_timer_info(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    g_func_timer_info = 20;       // reset and set timer

    char buf[20];
    sprintf (buf, "wait %d seconds", g_func_timer_info);

    // clear lcd
    display.fillScreen(_LCDML_BACKGROUND_COLOR);
    // set text color / Textfarbe setzen
    display.setTextColor(_LCDML_TEXT_COLOR);  
    // set text size / Textgroesse setzen
    display.setTextSize(_LCDML_FONT_SIZE);
  
    display.setCursor(0, _LCDML_FONT_H * 0); // line 0
    display.println(buf); 
    display.setCursor(0, _LCDML_FONT_H * 1); // line 1
    display.println(F("or press back button")); 
    
    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds

    LCDML.TIMER_msReset(g_timer_1);
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function

    // reset screensaver timer
    LCDML.SCREEN_resetTimer();

     // this function is called every 100 milliseconds

    // this method checks every 1000 milliseconds if it is called
    if(LCDML.TIMER_ms(g_timer_1, 1000))
    {
      g_timer_1 = millis();
      g_func_timer_info--;                // increment the value every second
      char buf[20];
      sprintf (buf, "wait %d seconds", g_func_timer_info);

      // clear lcd
      display.fillScreen(_LCDML_BACKGROUND_COLOR);
      // set text color / Textfarbe setzen
      display.setTextColor(_LCDML_TEXT_COLOR);  
      // set text size / Textgroesse setzen
      display.setTextSize(_LCDML_FONT_SIZE);
    
      display.setCursor(0, _LCDML_FONT_H * 0); // line 0
      display.println(buf); 
      display.setCursor(0, _LCDML_FONT_H * 1); // line 1
      display.println(F("or press back button")); 

    }

    // this function can only be ended when quit button is pressed or the time is over
    // check if the function ends normally
    if (g_func_timer_info <= 0)
    {
      // leave this function
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}

// *********************************************************************
uint8_t g_button_value = 0; // button value counter (global variable)
void mFunc_p2(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    // print LCD content
    char buf[17];
    sprintf (buf, "count: %d of 3", 0);

    // clear lcd
    display.fillScreen(_LCDML_BACKGROUND_COLOR);
    // set text color / Textfarbe setzen
    display.setTextColor(_LCDML_TEXT_COLOR);  
    // set text size / Textgroesse setzen
    display.setTextSize(_LCDML_FONT_SIZE);
  
    display.setCursor(0, _LCDML_FONT_H * 0); // line 0
    display.println(F("press a or w button")); 
    display.setCursor(0, _LCDML_FONT_H * 1); // line 1
    display.println(buf); 

    // Reset Button Value
    g_button_value = 0;

    // Disable the screensaver for this function until it is closed
    LCDML.FUNC_disableScreensaver();
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function

    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      if (LCDML.BT_checkLeft() || LCDML.BT_checkUp()) // check if button left is pressed
      {
        LCDML.BT_resetLeft(); // reset the left button
        LCDML.BT_resetUp(); // reset the left button
        g_button_value++;

        // update LCD content
        char buf[20];
        sprintf (buf, "count: %d of 3", g_button_value);

         // clear lcd
        display.fillScreen(_LCDML_BACKGROUND_COLOR);
        // set text color / Textfarbe setzen
        display.setTextColor(_LCDML_TEXT_COLOR);  
        // set text size / Textgroesse setzen
        display.setTextSize(_LCDML_FONT_SIZE);
      
        display.setCursor(0, _LCDML_FONT_H * 0); // line 0
        display.println(F("press a or w button")); 
        display.setCursor(0, _LCDML_FONT_H * 1); // line 1
        display.println(buf); 
      }
    }

   // check if button count is three
    if (g_button_value >= 3) {
      LCDML.FUNC_goBackToMenu();      // leave this function
    }
  }

  if(LCDML.FUNC_close())     // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}


// *********************************************************************
void mFunc_screensaver(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function

    // clear lcd
    display.fillScreen(_LCDML_BACKGROUND_COLOR);
    // set text color / Textfarbe setzen
    display.setTextColor(_LCDML_TEXT_COLOR);  
    // set text size / Textgroesse setzen
    display.setTextSize(_LCDML_FONT_SIZE);
  
    display.setCursor(0, _LCDML_FONT_H * 0); // line 0
    display.println(F("screensaver")); 
    display.setCursor(0, _LCDML_FONT_H * 1); // line 1
    display.println(F("press any key")); 
    display.setCursor(0, _LCDML_FONT_H * 2); // line 2
    display.println(F("to leave it")); 

    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }

  if(LCDML.FUNC_close())          // ****** STABLE END *********
  {
    // The screensaver go to the root menu
    LCDML.MENU_goRoot();
  }
}



// *********************************************************************
void mFunc_back(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // end function and go an layer back
    LCDML.FUNC_goBackToMenu(1);      // leave this function and go a layer back
  }
}


// *********************************************************************
void mFunc_goToRootMenu(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // go to root and display menu
    LCDML.MENU_goRoot();
  }
}


// *********************************************************************
void mFunc_jumpTo_timer_info(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // Jump to main screen
    LCDML.OTHER_jumpToFunc(mFunc_timer_info);
  }
}

// *********************************************************************
void mFunc_toggleAuto(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // Jump to main screen
    automatic = ! automatic;
  }
  LCDML.FUNC_goBackToMenu();
}

// *********************************************************************
void mFunc_resetAbs(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // Jump to main screen
    absolute = 0;
    coarse_offset = DEFAULT_COARSE_OFFSET;
    medium_offset = DEFAULT_MEDIUM_OFFSET;
    fine_offset = DEFAULT_FINE_OFFSET;
    abs2res(absolute);
  }
  LCDML.FUNC_goBackToMenu();
}


// *********************************************************************
void mFunc_para(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {

    char buf[20];
    sprintf (buf, "parameter: %d", param);
    
    // setup function
     // clear lcd
    display.fillScreen(_LCDML_BACKGROUND_COLOR);
    // set text color / Textfarbe setzen
    display.setTextColor(_LCDML_TEXT_COLOR);  
    // set text size / Textgroesse setzen
    display.setTextSize(_LCDML_FONT_SIZE);
  
    display.setCursor(0, _LCDML_FONT_H * 0); // line 0
    display.println(buf); 
    display.setCursor(0, _LCDML_FONT_H * 1); // line 1
    display.println(F("press any key")); 
    display.setCursor(0, _LCDML_FONT_H * 2); // line 2
    display.println(F("to return")); 

    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds
  }

  if(LCDML.FUNC_loop())               // ****** LOOP *********
  {
    // For example
    switch(param)
    {
      case 51:
        autostep = 0.2;
        break;

      case 52:
        autostep = 2;
        break;

      case 53:
        autostep = ratio2;  //30
        break;

      case 54:
        autostep = ratio1; // 900
        break;

      case 55:
        autostep = 27000;
        break;

      case 61:
        autodelay = 1;
        break;

      case 62:
        autodelay = 10;
        break;

      case 63:
        autodelay = 100;
        break;

      case 64:
        autodelay = 1000;
        break;

      case 71:
        sine_table.sync_offset = SYNC_OFFSET_COUNT;
        break;

      case 72:
        sine_table.sync_offset = SYNC_OFFSET_COUNT+NUM_SINE_ELEMENTS/4;
        break;

      case 73:
        sine_table.sync_offset = SYNC_OFFSET_COUNT+NUM_SINE_ELEMENTS/2;
        break;

      case 74:
        sine_table.sync_offset = SYNC_OFFSET_COUNT+3*NUM_SINE_ELEMENTS/4.;
        break;

      case 81:
        coarse_offset = 0;
        abs2res(0);
        break;

      case 82:
        coarse_offset = 90;
        abs2res(0);
        break;

      case 83:
        coarse_offset = 180;
        abs2res(0);
       break;

      case 84:
        coarse_offset = -90;
        abs2res(0);
       break;

      default:
        // do nothing
        break;
    }

    char buf[20];
    sprintf (buf, "autodelay: %d", autodelay);
    display.setCursor(0, _LCDML_FONT_H * 3); // line 0
    display.println(buf); 
    sprintf (buf, "autostep: %f", autostep);
    display.setCursor(0, _LCDML_FONT_H * 4); // line 0
    display.println(buf); 


//    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}




// *********************************************************************
void mFunc_exampleEventHandling(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    LCDML_UNUSED(param);
    
    // setup
    // is called only if it is started

    // starts a trigger event for the loop function every 100 milliseconds
    //LCDML.FUNC_setLoopInterval(100);

    // uncomment this line when the menu should go back to the last called position
    // this could be a cursor position or the an active menu function
    // GBA means => go back advanced
    LCDML.FUNC_setGBA(); 

    // display content
    Serial.println("Event handling function is active");
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop
    // is called when it is triggered
    // - with LCDML_DISP_triggerMenu( milliseconds )
    // - with every button status change

    // uncomment this line when the screensaver should not be called when this function is running
    // reset screensaver timer
    // this function is only working when a loop timer is set
    //LCDML.SCREEN_resetTimer();

    // check if any button is pressed (enter, up, down, left, right)
    if(LCDML.BT_checkAny()) {
      LCDML.FUNC_goBackToMenu();
    }   

    // check if any event is active  
    if(LCDML.CE_checkAny())
    { 
      // check if the defined command char is pressed 
      if(LCDML.CE_check(0))
      {
        Serial.println(F("The defined command char is enabled"));         
      }
      
      // check if a special char was pressed
      // there are 10 defined special chars in control tab
      for(uint8_t i=1; i<=10; i++)
      {
        if(LCDML.CE_check(i))
        {
          Serial.print(F("The command number: "));
          Serial.print(i-1);
          Serial.println(F(" is pressed"));
        }
      }        
    }
    else
    {
      Serial.println(F("This function is called by menu, not over the callback function")); 
      Serial.println(F("Press 'c' or a number to display the content"));
    }
    LCDML.CE_resetAll();
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // loop end
    // you can here reset some global vars or delete it

    // reset all events 
    LCDML.CE_resetAll();
  }
}
