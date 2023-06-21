/* ===================================================================== *
 *                                                                       *
 * Dynamic content                                                       *
 *                                                                       *
 * ===================================================================== *
 */

uint8_t g_dynParam = 100; // when this value comes from an EEPROM, load it in setup
                          // at the moment here is no setup function (To-Do)
void mDyn_refPhase(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        ref_phase_update(1);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        ref_phase_update(-1);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        ref_phase_update(1);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        ref_phase_update(-1);
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Ref phase: %d", sine_table.sync_offset);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}


void mDyn_amplitude(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        amplitude_div+=16;
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        amplitude_div-=16;
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        amplitude_div+=16;
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        amplitude_div-=16;
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Vout: %0.1f (%d)", AMPLITUDE_FS * DIV_FACT / amplitude_div, amplitude_div);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}

void mDyn_amplitude2(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        amplitude_ref+=16;
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        amplitude_ref-=16;
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        amplitude_ref+=16;
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        amplitude_ref-=16;
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Vref: %0.1f (%d)", AMPLITUDE_FS * DIV_FACT / amplitude_ref, amplitude_ref);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}


void mDyn_stepsize(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        autostep++;
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        autostep--;
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        autostep++;
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        autostep--;
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Step Size: %.0f", autostep);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}


void mDyn_autodelay(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        autodelay++;
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        autodelay--;
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        autodelay++;
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        autodelay--;
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Auto delay: %d", autodelay);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}

void mDyn_heading(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        heading2res(autostep);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        heading2res(-autostep);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        heading2res(autostep);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        heading2res(-autostep);
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Heading: %.1f", heading);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}



void mDyn_ntos(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        ntos2res(autostep);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        ntos2res(-autostep);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        ntos2res(autostep);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        ntos2res(-autostep);
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "N to S: %.1f", ntos);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------
void mDyn_fine(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        fine2res(autostep);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        fine2res(-autostep);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        fine2res(autostep);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        fine2res(-autostep);
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Fine: %.1f", fine);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}

void mDyn_medium(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        medium2res(autostep);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        medium2res(-autostep);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        medium2res(autostep);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        medium2res(-autostep);
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Medium: %.1f", medium);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}
void mDyn_coarse(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        coarse2res(autostep);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        coarse2res(-autostep);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        coarse2res(autostep);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        coarse2res(-autostep);
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "Coarse: %.1f", coarse);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}
//-----------------------------------------------------------------------------------------------------------------------------------
void mDyn_absolute(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
        
        LCDML.BT_resetEnter();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        abs2res(autostep);
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        abs2res(-autostep);
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        abs2res(autostep);
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        abs2res(-autostep);
        LCDML.BT_resetRight();
      }
    }
  }


  char buf[20];
  sprintf (buf, "Abs: %.0f", absolute);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}
