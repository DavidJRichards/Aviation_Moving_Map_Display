/* ===================================================================== *
 *                                                                       *
 * Dynamic content                                                       *
 *                                                                       *
 * ===================================================================== *
 */

uint8_t g_dynParam = 100; // when this value comes from an EEPROM, load it in setup
                          // at the moment here is no setup function (To-Do)
void mDyn_para(uint8_t line)
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
        g_dynParam++;
        LCDML.BT_resetUp();
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        g_dynParam--;
        LCDML.BT_resetDown();
      }


      if(LCDML.BT_checkLeft())
      {
        g_dynParam++;
        LCDML.BT_resetLeft();
      }
      
      if(LCDML.BT_checkRight())
      {
        g_dynParam--;
        LCDML.BT_resetRight();
      }
    }
  }

  char buf[20];
  sprintf (buf, "dynValue: %d", g_dynParam);

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
  sprintf (buf, "absolute: %.1f", absolute);

  display.setCursor(20, _LCDML_FONT_H * (line));
  display.println(buf);
}
