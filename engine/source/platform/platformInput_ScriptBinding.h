//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "platform/event.h"
#include "platform/platformInput.h"
#if defined(TORQUE_SDL) || defined(TORQUE_SDL_GAMEPAD)
#ifndef _SDLGAMEPAD_H_
#include "platformSDL/sdlGamepad.h"
#endif
#endif

#ifndef TORQUE_SDL
//------------------------------------------------------------------------------
/*! Use the activateKeyboard function to enable directInput polling of the keyboard.
    @return Returns a true if polling was successfully enabled.
    @sa deactivateKeyboard
*/
ConsoleFunctionWithDocs( activateKeyboard, ConsoleBool, 1, 1, ())
{
	return Input::activateKeyboard();
}

//------------------------------------------------------------------------------
/*! Use the deactivateKeyboard function to disable directInput polling of the keyboard.
    @return No return value.
    @sa activateKeyboard
*/
ConsoleFunctionWithDocs( deactivateKeyboard, ConsoleVoid, 1, 1, ())
{
	Input::deactivateKeyboard();
}

//------------------------------------------------------------------------------
/*! Use the enableMouse function to enable mouse input.
    @return No return value.
    @sa disableMouse
*/
ConsoleFunctionWithDocs( enableMouse, ConsoleVoid, 1, 1, ())
{
	Input::enableMouse();
}

//------------------------------------------------------------------------------
/*! Use the disableMouse function to disable mouse input.
    @return No return value.
    @sa enableMouse
*/
ConsoleFunctionWithDocs( disableMouse, ConsoleVoid, 1, 1, ())
{
	Input::disableMouse();
}

//------------------------------------------------------------------------------
/*! Use the echoInputState function to dump the input state of the mouse, keyboard, and joystick to the console.
    @return No return value.
    @sa activateDirectInput, deactivateDirectInput, activateKeyboard, deactivateKeyboard, disableJoystick, enableJoystick, enableMouse, disableMouse
*/
ConsoleFunctionWithDocs( echoInputState, ConsoleVoid, 1, 1, ())
{
	Input::echoInputState();
}
#endif // !TORQUE_SDL

//------------------------------------------------------------------------------
/*! Use the enableJoystick function to enable joystick and gamepad input if present.
    @return Will return true if the subsystem was successfully enabled, false otherwise.
    @sa disableJoystick, getJoystickAxes, isJoystickDetected
*/
ConsoleFunctionWithDocs( enableJoystick, ConsoleBool, 1, 1, ())
{
	return Input::enableJoystick();
}

//------------------------------------------------------------------------------
/*! Use the disableJoystick function to disable joystick and gamepad input.
    @return No return value.
    @sa enableJoystick, getJoystickAxes, isJoystickEnabled
*/
ConsoleFunctionWithDocs( disableJoystick, ConsoleVoid, 1, 1, ())
{
	Input::disableJoystick();
}

#if defined(TORQUE_SDL) || defined(TORQUE_SDL_GAMEPAD)

//------------------------------------------------------------------------------
/*! Determine if one or more joysticks or gamepads are connected to the system.
    @return Returns true if at least one gamepad or raw joystick is detected.
*/
ConsoleFunctionWithDocs( isJoystickDetected, ConsoleBool, 1, 1, ())
{
   return SDLGamepad::isJoystickDetected();
}

//------------------------------------------------------------------------------
/*! Get the current axes layout string for a raw joystick instance.
    @param instance A non-negative joystick slot index.
    @return Returns a tab-delimited string starting with the axis count.
*/
ConsoleFunctionWithDocs( getJoystickAxes, ConsoleString, 2, 2, (instance))
{
   return SDLGamepad::getJoystickAxesString(dAtoi(argv[1]));
}

//------------------------------------------------------------------------------
/*! Return the number of connected raw joysticks.
    @return The number of connected raw joysticks.
*/
ConsoleFunctionWithDocs( getJoystickCount, ConsoleInt, 1, 1, ())
{
   return SDLGamepad::getJoystickCount();
}

//------------------------------------------------------------------------------
/*! Return the number of connected SDL gamepads.
    @return The number of connected gamepads.
*/
ConsoleFunctionWithDocs( getGamepadCount, ConsoleInt, 1, 1, ())
{
   return SDLGamepad::getGamepadCount();
}

//------------------------------------------------------------------------------
/*! Check whether a gamepad slot is currently connected.
    @param slot Zero-based gamepad slot index (0..7).
    @return True if connected.
*/
ConsoleFunctionWithDocs( isGamepadConnected, ConsoleBool, 2, 2, (int slot))
{
   return SDLGamepad::isGamepadConnected(dAtoi(argv[1]));
}

//------------------------------------------------------------------------------
/*! Return the product/device name of the gamepad in a slot.
    @param slot Zero-based gamepad slot index (0..7).
    @return Gamepad name, or empty string if disconnected.
*/
ConsoleFunctionWithDocs( getGamepadName, ConsoleString, 2, 2, (int slot))
{
   const char* name = SDLGamepad::getGamepadName(dAtoi(argv[1]));
   char* ret = Con::getReturnBuffer(dStrlen(name) + 1);
   dStrcpy(ret, name);
   return ret;
}

//------------------------------------------------------------------------------
/*! Return the normalized controller family for a gamepad slot.
    @param slot Zero-based gamepad slot index (0..7).
    @return Controller type ("xbox", "playstation", "nintendo", "generic", or "unknown").
*/
ConsoleFunctionWithDocs( getGamepadType, ConsoleString, 2, 2, (int slot))
{
   const char* typeName = SDLGamepad::getGamepadType(dAtoi(argv[1]));
   char* ret = Con::getReturnBuffer(dStrlen(typeName) + 1);
   dStrcpy(ret, typeName);
   return ret;
}

//------------------------------------------------------------------------------
/*! Return the face-button glyph label for a gamepad button.
    @param slot Zero-based gamepad slot index (0..7).
    @param buttonName Button token or SDL button name (e.g. "btn_a", "south").
    @return Glyph label ("A", "B", "X", "Y", "Cross", "Circle", "Square", "Triangle", "Unknown").
*/
ConsoleFunctionWithDocs( getGamepadButtonLabel, ConsoleString, 3, 3, (int slot, string buttonName))
{
   const char* label = SDLGamepad::getGamepadButtonLabel(dAtoi(argv[1]), argv[2]);
   char* ret = Con::getReturnBuffer(dStrlen(label) + 1);
   dStrcpy(ret, label);
   return ret;
}

//------------------------------------------------------------------------------
/*! Trigger low/high frequency motor rumble on a gamepad slot.
    @param slot Zero-based gamepad slot index (0..7).
    @param lowFreq Normalized low-frequency motor strength [0..1].
    @param highFreq Normalized high-frequency motor strength [0..1].
    @param durationMs Rumble duration in milliseconds.
    @return True on success.
*/
ConsoleFunctionWithDocs( setGamepadRumble, ConsoleBool, 5, 5, (int slot, float lowFreq, float highFreq, int durationMs))
{
   return SDLGamepad::setGamepadRumble(
      dAtoi(argv[1]),
      dAtof(argv[2]),
      dAtof(argv[3]),
      (U32)dAtoi(argv[4]));
}

//------------------------------------------------------------------------------
/*! Trigger adaptive/impulse trigger rumble on a supported gamepad slot.
    @param slot Zero-based gamepad slot index (0..7).
    @param left Normalized left trigger motor strength [0..1].
    @param right Normalized right trigger motor strength [0..1].
    @param durationMs Rumble duration in milliseconds.
    @return True on success.
*/
ConsoleFunctionWithDocs( setGamepadTriggerRumble, ConsoleBool, 5, 5, (int slot, float left, float right, int durationMs))
{
   return SDLGamepad::setGamepadTriggerRumble(
      dAtoi(argv[1]),
      dAtof(argv[2]),
      dAtof(argv[3]),
      (U32)dAtoi(argv[4]));
}

//------------------------------------------------------------------------------
/*! Enable SDL gamepad support (legacy XInput compatibility wrapper).
    @return True if gamepad subsystem is initialized.
*/
ConsoleFunctionWithDocs( enableXInput, ConsoleBool, 1, 1, ())
{
   SDLGamepad::setGamepadEnabled(true);
   return SDLGamepad::isInitialized() || SDLGamepad::init();
}

//------------------------------------------------------------------------------
/*! Disable SDL gamepad event emission (legacy XInput compatibility wrapper).
    @return No return value.
*/
ConsoleFunctionWithDocs( disableXInput, ConsoleVoid, 1, 1, ())
{
   SDLGamepad::setGamepadEnabled(false);
}

//------------------------------------------------------------------------------
/*! Re-emit current gamepad connection and input states (legacy XInput compatibility wrapper).
    @return No return value.
*/
ConsoleFunctionWithDocs( resetXInput, ConsoleVoid, 1, 1, ())
{
   SDLGamepad::resetGamepadStates();
}

//------------------------------------------------------------------------------
/*! Check whether a controller slot is connected (legacy XInput compatibility wrapper).
    @param controllerID Zero-based index of the controller to check.
    @return True if connected.
*/
ConsoleFunctionWithDocs( isXInputConnected, ConsoleBool, 2, 2, ( int controllerID ))
{
   return SDLGamepad::isGamepadConnected(dAtoi(argv[1]));
}

//------------------------------------------------------------------------------
/*! Query the current or previous state of a connected gamepad (legacy XInput compatibility wrapper).
    @param controllerID Zero-based index of the controller to return information about.
    @param property Name of input action being queried, such as "XI_THUMBLX".
    @param current True checks current frame state, false checks previous frame state.
    @return Queried state value, or -1 if invalid/disconnected.
*/
ConsoleFunctionWithDocs( getXInputState, ConsoleInt, 3, 4, ( int controllerID, string property, bool current ) )
{
   struct PropertyEntry
   {
      const char* name;
      S32 code;
   };
   static const PropertyEntry kProperties[] =
   {
      { "XI_THUMBLX",        XI_THUMBLX },
      { "XI_THUMBLY",        XI_THUMBLY },
      { "XI_THUMBRX",        XI_THUMBRX },
      { "XI_THUMBRY",        XI_THUMBRY },
      { "XI_LEFT_TRIGGER",   XI_LEFT_TRIGGER },
      { "XI_RIGHT_TRIGGER",  XI_RIGHT_TRIGGER },
      { "SI_UPOV",           SI_UPOV },
      { "SI_DPOV",           SI_DPOV },
      { "SI_LPOV",           SI_LPOV },
      { "SI_RPOV",           SI_RPOV },
      { "XI_START",          XI_START },
      { "XI_BACK",           XI_BACK },
      { "XI_GUIDE",          XI_GUIDE },
      { "XI_MISC1",          XI_MISC1 },
      { "XI_TOUCHPAD",       XI_TOUCHPAD },
      { "XI_LEFT_THUMB",     XI_LEFT_THUMB },
      { "XI_RIGHT_THUMB",    XI_RIGHT_THUMB },
      { "XI_LEFT_SHOULDER",  XI_LEFT_SHOULDER },
      { "XI_RIGHT_SHOULDER", XI_RIGHT_SHOULDER },
      { "XI_A",              XI_A },
      { "XI_B",              XI_B },
      { "XI_X",              XI_X },
      { "XI_Y",              XI_Y },
      { "XI_PADDLE1",        XI_PADDLE1 },
      { "XI_PADDLE2",        XI_PADDLE2 },
      { "XI_PADDLE3",        XI_PADDLE3 },
      { "XI_PADDLE4",        XI_PADDLE4 }
   };

   const S32 slot = dAtoi(argv[1]);
   const bool current = (argc >= 4) ? (dAtoi(argv[3]) == 1 || dAtob(argv[3])) : true;

   for (U32 i = 0; i < sizeof(kProperties) / sizeof(kProperties[0]); ++i)
   {
      if (!dStricmp(argv[2], kProperties[i].name))
         return SDLGamepad::getXInputState(slot, kProperties[i].code, current);
   }

   return -1;
}

//------------------------------------------------------------------------------
/*! Activate vibration motors on the named device ("gamepad0".."gamepad7" or "joystick0".."joystick7").
    @param device Name of the device to rumble.
    @param xRumble Low-frequency intensity [0..1].
    @param yRumble High-frequency intensity [0..1].
*/
ConsoleFunctionWithDocs( rumble, ConsoleVoid, 4, 4, (string device, float xRumble, float yRumble) )
{
   SDLGamepad::rumbleDevice(argv[1], dAtof(argv[2]), dAtof(argv[3]));
}

#else

/*!	Enables XInput for Xbox 360 controllers.
	@return No return value.
*/
ConsoleFunctionWithDocs( enableXInput, ConsoleBool, 1, 1, ())
{
   return( DInputManager::enableXInput() );
}

/*! Disables XInput for Xbox 360 controllesr
	@return No return value.
*/

ConsoleFunctionWithDocs( disableXInput, ConsoleVoid, 1, 1, ())
{
   DInputManager::disableXInput();
}

/*!	Rebuilds the XInput section of the InputManager.
     @return No return value.
*/
ConsoleFunctionWithDocs( resetXInput, ConsoleVoid, 1, 1, ())
{
   // This function requests a full "refresh" of events for all controllers the
   // next time we go through the input processing loop. This is useful to call
   // at the beginning of your game code after your actionMap is set up to hook
   // all of the appropriate events
   
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() ) 
      mgr->resetXInput();
}

/*!	Checks to see if an Xbox 360 controller is connected
	@param controllerID Zero-based index of the controller to check.
	@return 1 if the controller is connected, 0 if it isn't, and 205 if XInput
	hasn't been initialized.
*/

ConsoleFunctionWithDocs( isXInputConnected, ConsoleBool, 2, 2, ( int controllerID ))
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() ) return mgr->isXInputConnected( atoi( argv[1] ) );
   return false;
}

/*!	Queries the current state of a connected Xbox 360 controller.
	@param controllerID Zero-based index of the controller to return information about.
    @param property Name of input action being queried, such as \"XI_THUMBLX\".
    @param current True checks current device in action.
    @return Button queried - 1 if the button is pressed, 0 if it's not.
    @return Thumbstick queried - Int representing displacement from rest position.
    @return %Trigger queried - Int from 0 to 255 representing how far the trigger is displaced.
*/

//------------------------------------------------------------------------------
ConsoleFunctionWithDocs( getXInputState, ConsoleInt, 3, 4, ( int controllerID, string property, bool current ) )
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );

   if ( !mgr || !mgr->isEnabled() ) 
      return -1;

   // Use a little bit of macro magic to simplify this otherwise monolothic
   // block of code.
#define GET_XI_STATE(constName) \
   if (!dStricmp(argv[2], #constName)) \
      return mgr->getXInputState( dAtoi( argv[1] ), constName, ( dAtoi ( argv[3] ) == 1) );
   
   GET_XI_STATE(XI_THUMBLX);
   GET_XI_STATE(XI_THUMBLY);
   GET_XI_STATE(XI_THUMBRX);
   GET_XI_STATE(XI_THUMBRY);
   GET_XI_STATE(XI_LEFT_TRIGGER);
   GET_XI_STATE(XI_RIGHT_TRIGGER);
   GET_XI_STATE(SI_UPOV);
   GET_XI_STATE(SI_DPOV);
   GET_XI_STATE(SI_LPOV);
   GET_XI_STATE(SI_RPOV);
   GET_XI_STATE(XI_START);
   GET_XI_STATE(XI_BACK);
   GET_XI_STATE(XI_LEFT_THUMB);
   GET_XI_STATE(XI_RIGHT_THUMB);
   GET_XI_STATE(XI_LEFT_SHOULDER);
   GET_XI_STATE(XI_RIGHT_SHOULDER);
   GET_XI_STATE(XI_A);
   GET_XI_STATE(XI_B);
   GET_XI_STATE(XI_X);
   GET_XI_STATE(XI_Y);
#undef GET_XI_STATE

   return -1;
}


/*!	Activates the vibration motors in the specified controller.
    The controller will constantly at it's xRumble and yRumble intensities until
    changed or told to stop.
     
	 Valid inputs for xRumble/yRumble are [0 - 1].

	 @param device Name of the device to rumble.
      @param xRumble Intensity to apply to the left motor.
      @param yRumble Intensity to apply to the right motor.

	 @note in an Xbox 360 controller, the left motor is low-frequency,
     while the right motor is high-frequency.

*/

ConsoleFunctionWithDocs( rumble, ConsoleVoid, 4, 4, (string device, float xRumble, float yRumble) )
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() )
   {
      mgr->rumble(argv[1], dAtof(argv[2]), dAtof(argv[3]));
   }
   else
   {
      Con::printf( "DirectInput/XInput is not enabled." );
   }
}

#endif // TORQUE_SDL || TORQUE_SDL_GAMEPAD
