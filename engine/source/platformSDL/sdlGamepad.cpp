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

#if defined(TORQUE_SDL) || defined(TORQUE_SDL_GAMEPAD)

#include "platform/platform.h"
#include "platform/platformInput.h"
#include "platform/event.h"
#include "game/gameInterface.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "input/actionMap.h"
#include "math/mMathFn.h"
#include "collection/vector.h"
#include "platformSDL/sdlGamepad.h"
#ifdef TORQUE_SDL
#include "platformSDL/sdlInput.h"
#endif

#include <SDL3/SDL.h>

namespace
{
   struct JoystickAxisInfo
   {
      S32 type;
      S32 minValue;
      S32 maxValue;
   };

   struct GamepadSlot
   {
      SDL_Gamepad* handle;
      SDL_JoystickID instanceId;
      Sint16 rawAxes[SDL_GAMEPAD_AXIS_COUNT];
      Sint16 prevRawAxes[SDL_GAMEPAD_AXIS_COUNT];
      F32 lastSentAxes[SDL_GAMEPAD_AXIS_COUNT];
      bool buttons[SDL_GAMEPAD_BUTTON_COUNT];
      bool prevButtons[SDL_GAMEPAD_BUTTON_COUNT];
   };

   struct JoystickSlot
   {
      SDL_Joystick* handle;
      SDL_JoystickID instanceId;
      Vector<JoystickAxisInfo> axisList;
      Vector<U8> hatStates;
      Vector<bool> buttonStates;
   };

   GamepadSlot sGamepads[SDLGamepad::MaxGamepads];
   JoystickSlot sJoysticks[SDLGamepad::MaxJoysticks];

   bool sInitialized = false;
   bool sActive = false;
   bool sGamepadEnabled = true;
   bool sJoystickEnabled = true;
   bool sPrefsRegistered = false;
   bool sPumpedThisFrame = false;

   F32 sThumbDeadzone = 0.24f;
   F32 sTriggerDeadzone = 0.12f;
   const char* sMappingFile = "gamecontrollerdb.txt";

   void clearGamepadSlot( S32 slot )
   {
      GamepadSlot& gp = sGamepads[slot];
      gp.handle = NULL;
      gp.instanceId = 0;
      for ( S32 i = 0; i < SDL_GAMEPAD_AXIS_COUNT; ++i )
      {
         gp.rawAxes[i] = 0;
         gp.prevRawAxes[i] = 0;
         gp.lastSentAxes[i] = 0.0f;
      }
      for ( S32 i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i )
      {
         gp.buttons[i] = false;
         gp.prevButtons[i] = false;
      }
   }

   void clearJoystickSlot( S32 slot )
   {
      JoystickSlot& js = sJoysticks[slot];
      js.handle = NULL;
      js.instanceId = 0;
      js.axisList.clear();
      js.hatStates.clear();
      js.buttonStates.clear();
   }

   S32 findGamepadSlotByInstance( SDL_JoystickID instanceId )
   {
      if ( instanceId == 0 )
         return -1;

      for ( S32 i = 0; i < SDLGamepad::MaxGamepads; ++i )
      {
         if ( sGamepads[i].handle != NULL && sGamepads[i].instanceId == instanceId )
            return i;
      }
      return -1;
   }

   S32 findJoystickSlotByInstance( SDL_JoystickID instanceId )
   {
      if ( instanceId == 0 )
         return -1;

      for ( S32 i = 0; i < SDLGamepad::MaxJoysticks; ++i )
      {
         if ( sJoysticks[i].handle != NULL && sJoysticks[i].instanceId == instanceId )
            return i;
      }
      return -1;
   }

   F32 normalizeSignedAxis( Sint16 value )
   {
      if ( value < 0 )
         return mClampF( F32( value ) / 32768.0f, -1.0f, 0.0f );
      return mClampF( F32( value ) / 32767.0f, 0.0f, 1.0f );
   }

   S32 mapGamepadButtonToTorque( SDL_GamepadButton button )
   {
      switch ( button )
      {
         case SDL_GAMEPAD_BUTTON_SOUTH:          return XI_A;
         case SDL_GAMEPAD_BUTTON_EAST:           return XI_B;
         case SDL_GAMEPAD_BUTTON_WEST:           return XI_X;
         case SDL_GAMEPAD_BUTTON_NORTH:          return XI_Y;
         case SDL_GAMEPAD_BUTTON_BACK:           return XI_BACK;
         case SDL_GAMEPAD_BUTTON_GUIDE:          return XI_GUIDE;
         case SDL_GAMEPAD_BUTTON_START:          return XI_START;
         case SDL_GAMEPAD_BUTTON_LEFT_STICK:     return XI_LEFT_THUMB;
         case SDL_GAMEPAD_BUTTON_RIGHT_STICK:    return XI_RIGHT_THUMB;
         case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return XI_LEFT_SHOULDER;
         case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return XI_RIGHT_SHOULDER;
         case SDL_GAMEPAD_BUTTON_DPAD_UP:        return SI_UPOV;
         case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return SI_DPOV;
         case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return SI_LPOV;
         case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return SI_RPOV;
         case SDL_GAMEPAD_BUTTON_MISC1:          return XI_MISC1;
         case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1:  return XI_PADDLE1;
         case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2:  return XI_PADDLE2;
         case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1:   return XI_PADDLE3;
         case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2:   return XI_PADDLE4;
         case SDL_GAMEPAD_BUTTON_TOUCHPAD:       return XI_TOUCHPAD;
         default:                                return -1;
      }
   }

   SDL_GamepadButton mapTorqueTokenToGamepadButton( const char* name )
   {
      if ( name == NULL || !name[0] )
         return SDL_GAMEPAD_BUTTON_INVALID;

      if ( !dStricmp( name, "btn_a" ) )       return SDL_GAMEPAD_BUTTON_SOUTH;
      if ( !dStricmp( name, "btn_b" ) )       return SDL_GAMEPAD_BUTTON_EAST;
      if ( !dStricmp( name, "btn_x" ) )       return SDL_GAMEPAD_BUTTON_WEST;
      if ( !dStricmp( name, "btn_y" ) )       return SDL_GAMEPAD_BUTTON_NORTH;
      if ( !dStricmp( name, "btn_back" ) )    return SDL_GAMEPAD_BUTTON_BACK;
      if ( !dStricmp( name, "btn_guide" ) )   return SDL_GAMEPAD_BUTTON_GUIDE;
      if ( !dStricmp( name, "btn_start" ) )   return SDL_GAMEPAD_BUTTON_START;
      if ( !dStricmp( name, "btn_lt" ) )      return SDL_GAMEPAD_BUTTON_LEFT_STICK;
      if ( !dStricmp( name, "btn_rt" ) )      return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
      if ( !dStricmp( name, "btn_l" ) )       return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
      if ( !dStricmp( name, "btn_r" ) )       return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
      if ( !dStricmp( name, "dpadu" ) )       return SDL_GAMEPAD_BUTTON_DPAD_UP;
      if ( !dStricmp( name, "dpadd" ) )       return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
      if ( !dStricmp( name, "dpadl" ) )       return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
      if ( !dStricmp( name, "dpadr" ) )       return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
      if ( !dStricmp( name, "btn_misc1" ) )   return SDL_GAMEPAD_BUTTON_MISC1;
      if ( !dStricmp( name, "btn_paddle1" ) ) return SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1;
      if ( !dStricmp( name, "btn_paddle2" ) ) return SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2;
      if ( !dStricmp( name, "btn_paddle3" ) ) return SDL_GAMEPAD_BUTTON_LEFT_PADDLE1;
      if ( !dStricmp( name, "btn_paddle4" ) ) return SDL_GAMEPAD_BUTTON_LEFT_PADDLE2;
      if ( !dStricmp( name, "btn_touchpad" ) ) return SDL_GAMEPAD_BUTTON_TOUCHPAD;

      return SDL_GetGamepadButtonFromString( name );
   }

   const char* gamepadTypeToString( SDL_GamepadType type )
   {
      switch ( type )
      {
         case SDL_GAMEPAD_TYPE_XBOX360:
         case SDL_GAMEPAD_TYPE_XBOXONE:
            return "xbox";
         case SDL_GAMEPAD_TYPE_PS3:
         case SDL_GAMEPAD_TYPE_PS4:
         case SDL_GAMEPAD_TYPE_PS5:
            return "playstation";
         case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
         case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
         case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
         case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
            return "nintendo";
         case SDL_GAMEPAD_TYPE_STANDARD:
            return "generic";
         default:
            return "unknown";
      }
   }

   const char* buttonLabelToString( SDL_GamepadButtonLabel label )
   {
      switch ( label )
      {
         case SDL_GAMEPAD_BUTTON_LABEL_A:        return "A";
         case SDL_GAMEPAD_BUTTON_LABEL_B:        return "B";
         case SDL_GAMEPAD_BUTTON_LABEL_X:        return "X";
         case SDL_GAMEPAD_BUTTON_LABEL_Y:        return "Y";
         case SDL_GAMEPAD_BUTTON_LABEL_CROSS:    return "Cross";
         case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:   return "Circle";
         case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:   return "Square";
         case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE: return "Triangle";
         default:                                return "Unknown";
      }
   }

   U8 currentJoystickModifiers()
   {
#if defined(TORQUE_OS_WIN)
      return Input::getModifierKeys();
#elif defined(TORQUE_SDL)
      return (U8)KeyMapSDL::getTorqueModFromSDL( SDL_GetModState() );
#else
      return 0;
#endif
   }

   void postGamepadEvent( S32 slot, U16 objType, U16 objInst, U8 action, F32 fValue )
   {
      if ( !Game || !sActive || !sGamepadEnabled )
         return;

      InputEvent ievent;
      ievent.deviceType = GamepadDeviceType;
      ievent.deviceInst = U32( slot );
      ievent.modifier = 0;
      ievent.ascii = 0;
      ievent.objType = objType;
      ievent.objInst = objInst;
      ievent.action = action;
      ievent.fValues[0] = fValue;
      Game->postEvent( ievent );
   }

   void emitProcessedThumbstickPair( S32 slot, SDL_GamepadAxis axisX, SDL_GamepadAxis axisY,
      U16 objX, U16 objY, bool forceEmit )
   {
      GamepadSlot& gp = sGamepads[slot];
      F32 outX = 0.0f;
      F32 outY = 0.0f;
      SDLGamepad::applyRadialDeadzone(
         normalizeSignedAxis( gp.rawAxes[axisX] ),
         normalizeSignedAxis( gp.rawAxes[axisY] ),
         mClampF( sThumbDeadzone, 0.0f, 0.99f ),
         outX, outY );

      if ( forceEmit || outX != gp.lastSentAxes[axisX] )
      {
         gp.lastSentAxes[axisX] = outX;
         postGamepadEvent( slot, XI_AXIS, objX, XI_MOVE, outX );
      }
      if ( forceEmit || outY != gp.lastSentAxes[axisY] )
      {
         gp.lastSentAxes[axisY] = outY;
         postGamepadEvent( slot, XI_AXIS, objY, XI_MOVE, outY );
      }
   }

   void emitProcessedTrigger( S32 slot, SDL_GamepadAxis axis, U16 objInst, bool forceEmit )
   {
      GamepadSlot& gp = sGamepads[slot];
      const F32 raw = mClampF( F32( gp.rawAxes[axis] ) / 32767.0f, 0.0f, 1.0f );
      const F32 adj = SDLGamepad::applyTriggerDeadzone( raw, mClampF( sTriggerDeadzone, 0.0f, 0.99f ) );

      if ( forceEmit || adj != gp.lastSentAxes[axis] )
      {
         gp.lastSentAxes[axis] = adj;
         postGamepadEvent( slot, XI_AXIS, objInst, XI_MOVE, adj );
      }
   }

   void syncGamepadSlotFromSDL( S32 slot, bool emitEvents )
   {
      GamepadSlot& gp = sGamepads[slot];
      if ( gp.handle == NULL )
         return;

      for ( S32 i = 0; i < SDL_GAMEPAD_AXIS_COUNT; ++i )
         gp.rawAxes[i] = SDL_GetGamepadAxis( gp.handle, (SDL_GamepadAxis)i );

      emitProcessedThumbstickPair( slot, SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY,
         XI_THUMBLX, XI_THUMBLY, emitEvents );
      emitProcessedThumbstickPair( slot, SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY,
         XI_THUMBRX, XI_THUMBRY, emitEvents );
      emitProcessedTrigger( slot, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, XI_LEFT_TRIGGER, emitEvents );
      emitProcessedTrigger( slot, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, XI_RIGHT_TRIGGER, emitEvents );

      for ( S32 b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b )
      {
         const bool pressed = SDL_GetGamepadButton( gp.handle, (SDL_GamepadButton)b );
         const bool changed = ( pressed != gp.buttons[b] );
         gp.buttons[b] = pressed;

         if ( emitEvents || changed )
         {
            const S32 torqueCode = mapGamepadButtonToTorque( (SDL_GamepadButton)b );
            if ( torqueCode >= 0 && ( pressed || changed ) )
            {
               postGamepadEvent( slot, XI_BUTTON, U16( torqueCode ),
                  pressed ? XI_MAKE : XI_BREAK, pressed ? 1.0f : 0.0f );
            }
         }
      }
   }

   S32 parseAxisType( S32 axisNum, const char* namedType )
   {
      if ( namedType != NULL )
      {
         if ( !dStricmp( namedType, "xaxis" ) )       return SI_XAXIS;
         if ( !dStricmp( namedType, "yaxis" ) )       return SI_YAXIS;
         if ( !dStricmp( namedType, "zaxis" ) )       return SI_ZAXIS;
         if ( !dStricmp( namedType, "rxaxis" ) )      return SI_RXAXIS;
         if ( !dStricmp( namedType, "ryaxis" ) )      return SI_RYAXIS;
         if ( !dStricmp( namedType, "rzaxis" ) )      return SI_RZAXIS;
         if ( !dStricmp( namedType, "slider" ) )      return SI_SLIDER;
      }

      static const S32 kAxisDefaults[] =
      {
         SI_XAXIS, SI_YAXIS, SI_ZAXIS,
         SI_RXAXIS, SI_RYAXIS, SI_RZAXIS,
         SI_SLIDER
      };
      if ( axisNum >= 0 && axisNum < S32( sizeof( kAxisDefaults ) / sizeof( kAxisDefaults[0] ) ) )
         return kAxisDefaults[axisNum];

      return -1;
   }

   void loadJoystickCalibration( S32 slot )
   {
      JoystickSlot& js = sJoysticks[slot];
      js.axisList.clear();
      if ( js.handle == NULL )
         return;

      const S32 numAxes = SDL_GetNumJoystickAxes( js.handle );
      for ( S32 i = 0; i < numAxes; ++i )
      {
         JoystickAxisInfo axisInfo;
         axisInfo.type = -1;
         axisInfo.minValue = -32768;
         axisInfo.maxValue = 32767;

         char varName[128];
         dSprintf( varName, sizeof( varName ), "$Pref::Input::Joystick%d::Axis%d", slot, i );
         const char* axisStr = Con::getVariable( varName );
         if ( axisStr == NULL || !axisStr[0] )
         {
            axisInfo.type = parseAxisType( i, NULL );
         }
         else
         {
            char buf[256];
            dStrncpy( buf, axisStr, sizeof( buf ) - 1 );
            buf[sizeof( buf ) - 1] = '\0';

            char* tok = dStrtok( buf, " \t" );
            if ( tok )
            {
               axisInfo.type = parseAxisType( i, tok );
               tok = dStrtok( NULL, " \t" );
               if ( tok )
               {
                  axisInfo.minValue = dAtoi( tok );
                  tok = dStrtok( NULL, " \t" );
                  if ( tok )
                     axisInfo.maxValue = dAtoi( tok );
               }
            }
         }

         js.axisList.push_back( axisInfo );
      }

      const S32 numButtons = SDL_GetNumJoystickButtons( js.handle );
      js.buttonStates.clear();
      for ( S32 i = 0; i < numButtons; ++i )
         js.buttonStates.push_back( false );

      const S32 numHats = SDL_GetNumJoystickHats( js.handle );
      js.hatStates.clear();
      for ( S32 i = 0; i < numHats; ++i )
         js.hatStates.push_back( SDL_HAT_CENTERED );
   }

   void closeJoystickSlot( S32 slot )
   {
      if ( slot < 0 || slot >= SDLGamepad::MaxJoysticks )
         return;

      if ( sJoysticks[slot].handle != NULL )
      {
         SDL_CloseJoystick( sJoysticks[slot].handle );
         Con::printf( "Joystick %d disconnected.", slot );
      }
      clearJoystickSlot( slot );
   }

   void openGamepadById( SDL_JoystickID instanceId )
   {
      if ( instanceId == 0 || !SDL_IsGamepad( instanceId ) )
         return;

      if ( findGamepadSlotByInstance( instanceId ) >= 0 )
         return;

      // If this device was open as a raw joystick prior to a mapping update, release it cleanly.
      const S32 existingJoy = findJoystickSlotByInstance( instanceId );
      if ( existingJoy >= 0 )
         closeJoystickSlot( existingJoy );

      S32 freeSlot = -1;
      for ( S32 i = 0; i < SDLGamepad::MaxGamepads; ++i )
      {
         if ( sGamepads[i].handle == NULL )
         {
            freeSlot = i;
            break;
         }
      }
      if ( freeSlot < 0 )
      {
         Con::warnf( "SDLGamepad: maximum %d simultaneous gamepads already open.", SDLGamepad::MaxGamepads );
         return;
      }

      SDL_Gamepad* gamepad = SDL_OpenGamepad( instanceId );
      if ( gamepad == NULL )
      {
         Con::warnf( "SDLGamepad: failed to open gamepad (id=%u): %s", U32( instanceId ), SDL_GetError() );
         return;
      }

      clearGamepadSlot( freeSlot );
      sGamepads[freeSlot].handle = gamepad;
      sGamepads[freeSlot].instanceId = instanceId;
      SDL_SetGamepadPlayerIndex( gamepad, freeSlot );

      const char* name = SDL_GetGamepadName( gamepad );
      Con::printf( "Gamepad %d connected: %s (%s)",
         freeSlot,
         name ? name : "Unknown Gamepad",
         gamepadTypeToString( SDL_GetGamepadType( gamepad ) ) );

      postGamepadEvent( freeSlot, XI_BUTTON, XI_CONNECT, XI_MAKE, 1.0f );
      syncGamepadSlotFromSDL( freeSlot, false );
   }

   void closeGamepadById( SDL_JoystickID instanceId )
   {
      const S32 slot = findGamepadSlotByInstance( instanceId );
      if ( slot < 0 )
         return;

      postGamepadEvent( slot, XI_BUTTON, XI_CONNECT, XI_BREAK, 0.0f );
      SDL_CloseGamepad( sGamepads[slot].handle );
      clearGamepadSlot( slot );
      Con::printf( "Gamepad %d disconnected.", slot );
   }

   void openJoystickById( SDL_JoystickID instanceId )
   {
      if ( instanceId == 0 || SDL_IsGamepad( instanceId ) )
         return;

      if ( findGamepadSlotByInstance( instanceId ) >= 0 || findJoystickSlotByInstance( instanceId ) >= 0 )
         return;

      S32 freeSlot = -1;
      for ( S32 i = 0; i < SDLGamepad::MaxJoysticks; ++i )
      {
         if ( sJoysticks[i].handle == NULL )
         {
            freeSlot = i;
            break;
         }
      }
      if ( freeSlot < 0 )
      {
         Con::warnf( "SDLGamepad: maximum %d simultaneous raw joysticks already open.", SDLGamepad::MaxJoysticks );
         return;
      }

      SDL_Joystick* joystick = SDL_OpenJoystick( instanceId );
      if ( joystick == NULL )
      {
         Con::warnf( "SDLGamepad: failed to open raw joystick (id=%u): %s", U32( instanceId ), SDL_GetError() );
         return;
      }

      clearJoystickSlot( freeSlot );
      sJoysticks[freeSlot].handle = joystick;
      sJoysticks[freeSlot].instanceId = instanceId;
      loadJoystickCalibration( freeSlot );

      const char* name = SDL_GetJoystickName( joystick );
      Con::printf( "Joystick %d connected (raw): %s (%d axes, %d buttons, %d hats)",
         freeSlot,
         name ? name : "Unknown Joystick",
         SDL_GetNumJoystickAxes( joystick ),
         SDL_GetNumJoystickButtons( joystick ),
         SDL_GetNumJoystickHats( joystick ) );
   }

   void postRawJoystickEvent( S32 slot, U16 objType, U16 objInst, U8 action, F32 fValue )
   {
      if ( !Game || !sActive || !sJoystickEnabled )
         return;

      InputEvent ievent;
      ievent.deviceInst = U32( slot );
      ievent.deviceType = JoystickDeviceType;
      ievent.modifier = currentJoystickModifiers();
      ievent.ascii = 0;
      ievent.objType = objType;
      ievent.objInst = objInst;
      ievent.action = action;
      ievent.fValues[0] = fValue;
      Game->postEvent( ievent );
   }

   void handleRawHatMotion( S32 slot, U8 hatIndex, U8 currHatState )
   {
      JoystickSlot& js = sJoysticks[slot];
      if ( hatIndex >= js.hatStates.size() )
         return;

      const U8 prevHatState = js.hatStates[hatIndex];
      if ( prevHatState == currHatState )
         return;

      js.hatStates[hatIndex] = currHatState;

      struct HatMapping
      {
         U8 bit;
         U16 objInst;
      };
      static const HatMapping kHatMappings[] =
      {
         { SDL_HAT_UP,    SI_UPOV },
         { SDL_HAT_DOWN,  SI_DPOV },
         { SDL_HAT_LEFT,  SI_LPOV },
         { SDL_HAT_RIGHT, SI_RPOV }
      };

      for ( S32 i = 0; i < 4; ++i )
      {
         if ( ( prevHatState & kHatMappings[i].bit ) && !( currHatState & kHatMappings[i].bit ) )
            postRawJoystickEvent( slot, SI_POV, kHatMappings[i].objInst, SI_BREAK, 0.0f );
      }
      for ( S32 i = 0; i < 4; ++i )
      {
         if ( !( prevHatState & kHatMappings[i].bit ) && ( currHatState & kHatMappings[i].bit ) )
            postRawJoystickEvent( slot, SI_POV, kHatMappings[i].objInst, SI_MAKE, 1.0f );
      }
   }

   void handleRawAxisMotion( S32 slot, U8 axisIndex, Sint16 axisValue )
   {
      JoystickSlot& js = sJoysticks[slot];
      if ( axisIndex >= js.axisList.size() )
         return;

      const JoystickAxisInfo& info = js.axisList[axisIndex];
      if ( info.type == -1 )
         return;

      F32 scaled = 0.0f;
      if ( axisValue < 0 && info.minValue < 0 )
         scaled = F32( axisValue ) / -F32( info.minValue );
      else if ( axisValue > 0 && info.maxValue > 0 )
         scaled = F32( axisValue ) / F32( info.maxValue );

      scaled = mClampF( scaled, -1.0f, 1.0f );
      postRawJoystickEvent( slot, U16( info.type ), 0, SI_MOVE, scaled );
   }

   void advanceFrameSnapshots()
   {
      for ( S32 i = 0; i < SDLGamepad::MaxGamepads; ++i )
      {
         GamepadSlot& gp = sGamepads[i];
         if ( gp.handle == NULL )
            continue;

         for ( S32 a = 0; a < SDL_GAMEPAD_AXIS_COUNT; ++a )
            gp.prevRawAxes[a] = gp.rawAxes[a];
         for ( S32 b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b )
            gp.prevButtons[b] = gp.buttons[b];
      }
   }

   void registerVariables()
   {
      if ( sPrefsRegistered )
         return;

      sPrefsRegistered = true;
      Con::addVariable( "$Pref::Input::GamepadThumbDeadzone", TypeF32, &sThumbDeadzone );
      Con::addVariable( "$Pref::Input::GamepadTriggerDeadzone", TypeF32, &sTriggerDeadzone );
      Con::addVariable( "$Pref::Input::GamepadMappingFile", TypeString, &sMappingFile );
      Con::addVariable( "pref::Input::GamepadEnabled", TypeBool, &sGamepadEnabled );
      Con::addVariable( "pref::Input::JoystickEnabled", TypeBool, &sJoystickEnabled );
   }

   void loadDefaultMappingDatabase()
   {
      const char* mappingPath = Con::getVariable( "$Pref::Input::GamepadMappingFile" );
      if ( mappingPath == NULL || !mappingPath[0] )
         mappingPath = sMappingFile;

      if ( mappingPath && mappingPath[0] && Platform::isFile( mappingPath ) )
      {
         const S32 loaded = SDLGamepad::loadMappingsFromFile( mappingPath );
         if ( loaded >= 0 )
            Con::printf( "   Loaded %d SDL gamepad mapping(s) from %s.", loaded, mappingPath );
      }
   }

   void enumerateConnectedDevices()
   {
      int gamepadCount = 0;
      SDL_JoystickID* gamepads = SDL_GetGamepads( &gamepadCount );
      if ( gamepads )
      {
         for ( int i = 0; i < gamepadCount; ++i )
            openGamepadById( gamepads[i] );
         SDL_free( gamepads );
      }

      int joystickCount = 0;
      SDL_JoystickID* joysticks = SDL_GetJoysticks( &joystickCount );
      if ( joysticks )
      {
         for ( int i = 0; i < joystickCount; ++i )
         {
            if ( !SDL_IsGamepad( joysticks[i] ) )
               openJoystickById( joysticks[i] );
         }
         SDL_free( joysticks );
      }
   }
}

bool SDLGamepad::init()
{
   for ( S32 i = 0; i < MaxGamepads; ++i )
      clearGamepadSlot( i );
   for ( S32 i = 0; i < MaxJoysticks; ++i )
      clearJoystickSlot( i );

   registerVariables();

   // Ensure controller input stays active when Torque2D hosts its own Win32/Cocoa window
   // and SDL_VIDEO is disabled.
   SDL_SetHint( SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1" );

   if ( !SDL_InitSubSystem( SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK ) )
   {
      Con::warnf( "SDLGamepad::init: unable to initialize SDL3 gamepad/joystick subsystems: %s", SDL_GetError() );
      return false;
   }

   sInitialized = true;
   sPumpedThisFrame = false;

   loadDefaultMappingDatabase();
   enumerateConnectedDevices();

   Con::printf( "   SDL3 Gamepad/Joystick subsystem initialized (%d gamepad(s), %d raw joystick(s)).",
      getGamepadCount(), getJoystickCount() );
   return true;
}

void SDLGamepad::shutdown()
{
   if ( !sInitialized )
      return;

   for ( S32 i = 0; i < MaxGamepads; ++i )
   {
      if ( sGamepads[i].handle != NULL )
      {
         SDL_CloseGamepad( sGamepads[i].handle );
         clearGamepadSlot( i );
      }
   }

   for ( S32 i = 0; i < MaxJoysticks; ++i )
      closeJoystickSlot( i );

   SDL_QuitSubSystem( SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK );
   if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
      SDL_Quit();

   sInitialized = false;
   sActive = false;
   sPumpedThisFrame = false;
}

void SDLGamepad::activate()
{
   sActive = true;
   if ( sInitialized )
      resetGamepadStates();
}

void SDLGamepad::deactivate()
{
   sActive = false;
}

void SDLGamepad::pumpEvents()
{
   if ( !sInitialized )
      return;

   advanceFrameSnapshots();
   sPumpedThisFrame = true;

   SDL_PumpEvents();

   SDL_Event events[64];
   int count = 0;
   while ( ( count = SDL_PeepEvents(
      events,
      64,
      SDL_GETEVENT,
      SDL_EVENT_JOYSTICK_AXIS_MOTION,
      SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED ) ) > 0 )
   {
      for ( int i = 0; i < count; ++i )
         processEvent( events[i] );
   }
}

void SDLGamepad::process()
{
   if ( !sInitialized || !sActive )
      return;

#if defined(TORQUE_SDL)
   // On Linux and Emscripten, SDL_PollEvent in ProcessMessages() already pumped
   // and delivered controller events via SDLGamepad::processEvent().
   if ( !sPumpedThisFrame )
      advanceFrameSnapshots();
#else
   pumpEvents();
#endif
   sPumpedThisFrame = false;
}

void SDLGamepad::processEvent( const SDL_Event& event )
{
   if ( !sInitialized )
      return;

   switch ( event.type )
   {
      case SDL_EVENT_GAMEPAD_ADDED:
         openGamepadById( event.gdevice.which );
         break;

      case SDL_EVENT_GAMEPAD_REMOVED:
         closeGamepadById( event.gdevice.which );
         break;

      case SDL_EVENT_GAMEPAD_REMAPPED:
      {
         const S32 slot = findGamepadSlotByInstance( event.gdevice.which );
         if ( slot >= 0 )
            syncGamepadSlotFromSDL( slot, true );
         break;
      }

      case SDL_EVENT_GAMEPAD_AXIS_MOTION:
      {
         const S32 slot = findGamepadSlotByInstance( event.gaxis.which );
         if ( slot < 0 )
            break;

         const SDL_GamepadAxis axis = (SDL_GamepadAxis)event.gaxis.axis;
         if ( axis < 0 || axis >= SDL_GAMEPAD_AXIS_COUNT )
            break;

         sGamepads[slot].rawAxes[axis] = event.gaxis.value;
         if ( axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_LEFTY )
         {
            emitProcessedThumbstickPair( slot, SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY,
               XI_THUMBLX, XI_THUMBLY, false );
         }
         else if ( axis == SDL_GAMEPAD_AXIS_RIGHTX || axis == SDL_GAMEPAD_AXIS_RIGHTY )
         {
            emitProcessedThumbstickPair( slot, SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY,
               XI_THUMBRX, XI_THUMBRY, false );
         }
         else if ( axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER )
         {
            emitProcessedTrigger( slot, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, XI_LEFT_TRIGGER, false );
         }
         else if ( axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER )
         {
            emitProcessedTrigger( slot, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, XI_RIGHT_TRIGGER, false );
         }
         break;
      }

      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      case SDL_EVENT_GAMEPAD_BUTTON_UP:
      {
         const S32 slot = findGamepadSlotByInstance( event.gbutton.which );
         if ( slot < 0 )
            break;

         const SDL_GamepadButton btn = (SDL_GamepadButton)event.gbutton.button;
         if ( btn < 0 || btn >= SDL_GAMEPAD_BUTTON_COUNT )
            break;

         const bool pressed = event.gbutton.down;
         if ( sGamepads[slot].buttons[btn] == pressed )
            break;

         sGamepads[slot].buttons[btn] = pressed;
         const S32 torqueCode = mapGamepadButtonToTorque( btn );
         if ( torqueCode >= 0 )
         {
            postGamepadEvent( slot, XI_BUTTON, U16( torqueCode ),
               pressed ? XI_MAKE : XI_BREAK, pressed ? 1.0f : 0.0f );
         }
         break;
      }

      case SDL_EVENT_JOYSTICK_ADDED:
         openJoystickById( event.jdevice.which );
         break;

      case SDL_EVENT_JOYSTICK_REMOVED:
         if ( findGamepadSlotByInstance( event.jdevice.which ) < 0 )
            closeJoystickSlot( findJoystickSlotByInstance( event.jdevice.which ) );
         break;

      case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      {
         if ( findGamepadSlotByInstance( event.jaxis.which ) >= 0 || SDL_IsGamepad( event.jaxis.which ) )
            break;
         const S32 slot = findJoystickSlotByInstance( event.jaxis.which );
         if ( slot >= 0 )
            handleRawAxisMotion( slot, event.jaxis.axis, event.jaxis.value );
         break;
      }

      case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      case SDL_EVENT_JOYSTICK_BUTTON_UP:
      {
         if ( findGamepadSlotByInstance( event.jbutton.which ) >= 0 || SDL_IsGamepad( event.jbutton.which ) )
            break;
         const S32 slot = findJoystickSlotByInstance( event.jbutton.which );
         if ( slot >= 0 && event.jbutton.button < 32 )
         {
            const bool pressed = event.jbutton.down;
            if ( event.jbutton.button < sJoysticks[slot].buttonStates.size() )
               sJoysticks[slot].buttonStates[event.jbutton.button] = pressed;

            postRawJoystickEvent( slot, SI_BUTTON, U16( KEY_BUTTON0 + event.jbutton.button ),
               pressed ? SI_MAKE : SI_BREAK, pressed ? 1.0f : 0.0f );
         }
         break;
      }

      case SDL_EVENT_JOYSTICK_HAT_MOTION:
      {
         if ( findGamepadSlotByInstance( event.jhat.which ) >= 0 || SDL_IsGamepad( event.jhat.which ) )
            break;
         const S32 slot = findJoystickSlotByInstance( event.jhat.which );
         if ( slot >= 0 )
            handleRawHatMotion( slot, event.jhat.hat, event.jhat.value );
         break;
      }
   }
}

S32 SDLGamepad::loadMappingsFromFile( const char* path )
{
   if ( path == NULL || !path[0] )
      return -1;

   return SDL_AddGamepadMappingsFromFile( path );
}

bool SDLGamepad::isInitialized()
{
   return sInitialized;
}

bool SDLGamepad::isActive()
{
   return sActive;
}

bool SDLGamepad::isGamepadEnabled()
{
   return sGamepadEnabled;
}

void SDLGamepad::setGamepadEnabled( bool enabled )
{
   sGamepadEnabled = enabled;
}

bool SDLGamepad::enableJoystick()
{
   if ( !sInitialized )
      init();

   sJoystickEnabled = true;
   sGamepadEnabled = true;
   return sInitialized;
}

void SDLGamepad::disableJoystick()
{
   sJoystickEnabled = false;
}

bool SDLGamepad::isJoystickEnabled()
{
   return sJoystickEnabled;
}

void SDLGamepad::resetGamepadStates()
{
   if ( !sInitialized )
      return;

   for ( S32 i = 0; i < MaxGamepads; ++i )
   {
      if ( sGamepads[i].handle == NULL )
         continue;

      for ( S32 a = 0; a < SDL_GAMEPAD_AXIS_COUNT; ++a )
         sGamepads[i].lastSentAxes[a] = 0.0f;
      for ( S32 b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b )
         sGamepads[i].buttons[b] = false;

      postGamepadEvent( i, XI_BUTTON, XI_CONNECT, XI_MAKE, 1.0f );
      syncGamepadSlotFromSDL( i, true );
   }
}

void SDLGamepad::applyRadialDeadzone( F32 rawX, F32 rawY, F32 deadzone, F32& outX, F32& outY )
{
   if ( deadzone <= 0.0f )
   {
      outX = mClampF( rawX, -1.0f, 1.0f );
      outY = mClampF( rawY, -1.0f, 1.0f );
      return;
   }

   if ( deadzone >= 1.0f )
   {
      outX = 0.0f;
      outY = 0.0f;
      return;
   }

   const F32 mag = mSqrt( rawX * rawX + rawY * rawY );
   if ( mag <= deadzone || mag <= 0.0f )
   {
      outX = 0.0f;
      outY = 0.0f;
      return;
   }

   const F32 rescaled = mClampF( ( mag - deadzone ) / ( 1.0f - deadzone ), 0.0f, 1.0f );
   const F32 scale = rescaled / mag;
   outX = mClampF( rawX * scale, -1.0f, 1.0f );
   outY = mClampF( rawY * scale, -1.0f, 1.0f );
}

F32 SDLGamepad::applyTriggerDeadzone( F32 rawValue, F32 deadzone )
{
   const F32 clamped = mClampF( rawValue, 0.0f, 1.0f );
   if ( deadzone <= 0.0f )
      return clamped;
   if ( deadzone >= 1.0f || clamped <= deadzone )
      return 0.0f;

   return mClampF( ( clamped - deadzone ) / ( 1.0f - deadzone ), 0.0f, 1.0f );
}

S32 SDLGamepad::getGamepadCount()
{
   if ( !sInitialized )
      return 0;

   S32 count = 0;
   for ( S32 i = 0; i < MaxGamepads; ++i )
   {
      if ( sGamepads[i].handle != NULL )
         ++count;
   }
   return count;
}

bool SDLGamepad::isGamepadConnected( S32 slot )
{
   if ( !sInitialized || slot < 0 || slot >= MaxGamepads )
      return false;

   return sGamepads[slot].handle != NULL && SDL_GamepadConnected( sGamepads[slot].handle );
}

const char* SDLGamepad::getGamepadName( S32 slot )
{
   if ( !isGamepadConnected( slot ) )
      return "";

   const char* name = SDL_GetGamepadName( sGamepads[slot].handle );
   return name ? name : "";
}

const char* SDLGamepad::getGamepadType( S32 slot )
{
   if ( !isGamepadConnected( slot ) )
      return "unknown";

   return gamepadTypeToString( SDL_GetGamepadType( sGamepads[slot].handle ) );
}

const char* SDLGamepad::getRealGamepadType( S32 slot )
{
   if ( !isGamepadConnected( slot ) )
      return "unknown";

   return gamepadTypeToString( SDL_GetRealGamepadType( sGamepads[slot].handle ) );
}

const char* SDLGamepad::getGamepadButtonLabel( S32 slot, const char* buttonName )
{
   if ( !isGamepadConnected( slot ) )
      return "";

   const SDL_GamepadButton btn = mapTorqueTokenToGamepadButton( buttonName );
   if ( btn == SDL_GAMEPAD_BUTTON_INVALID )
      return "";

   return buttonLabelToString( SDL_GetGamepadButtonLabel( sGamepads[slot].handle, btn ) );
}

bool SDLGamepad::setGamepadRumble( S32 slot, F32 lowFreq, F32 highFreq, U32 durationMs )
{
   if ( !isGamepadConnected( slot ) )
      return false;

   const Uint16 low = Uint16( mClampF( lowFreq, 0.0f, 1.0f ) * 65535.0f );
   const Uint16 high = Uint16( mClampF( highFreq, 0.0f, 1.0f ) * 65535.0f );
   return SDL_RumbleGamepad( sGamepads[slot].handle, low, high, durationMs );
}

bool SDLGamepad::setGamepadTriggerRumble( S32 slot, F32 left, F32 right, U32 durationMs )
{
   if ( !isGamepadConnected( slot ) )
      return false;

   const Uint16 lVal = Uint16( mClampF( left, 0.0f, 1.0f ) * 65535.0f );
   const Uint16 rVal = Uint16( mClampF( right, 0.0f, 1.0f ) * 65535.0f );
   return SDL_RumbleGamepadTriggers( sGamepads[slot].handle, lVal, rVal, durationMs );
}

bool SDLGamepad::rumbleDevice( const char* deviceName, F32 xRumble, F32 yRumble )
{
   if ( !sInitialized || deviceName == NULL )
      return false;

   U32 deviceType = 0;
   U32 deviceInst = 0;
   if ( !ActionMap::getDeviceTypeAndInstance( deviceName, deviceType, deviceInst ) )
   {
      Con::printf( "SDLGamepad::rumbleDevice: unknown device: %s", deviceName );
      return false;
   }

   const F32 clampedX = mClampF( xRumble, 0.0f, 1.0f );
   const F32 clampedY = mClampF( yRumble, 0.0f, 1.0f );
   const Uint32 durationMs = ( clampedX == 0.0f && clampedY == 0.0f ) ? 0u : 0x7FFFFFFFu;

   if ( deviceType == GamepadDeviceType )
      return setGamepadRumble( S32( deviceInst ), clampedX, clampedY, durationMs );

   if ( deviceType == JoystickDeviceType )
   {
      const S32 slot = S32( deviceInst );
      if ( slot < 0 || slot >= MaxJoysticks || sJoysticks[slot].handle == NULL )
         return false;

      const Uint16 low = Uint16( clampedX * 65535.0f );
      const Uint16 high = Uint16( clampedY * 65535.0f );
      return SDL_RumbleJoystick( sJoysticks[slot].handle, low, high, durationMs );
   }

   Con::printf( "SDLGamepad::rumbleDevice: only supports gamepad and joystick devices." );
   return false;
}

S32 SDLGamepad::getXInputState( S32 controllerID, S32 property, bool current )
{
   if ( controllerID < 0 || controllerID >= MaxGamepads )
   {
      Con::warnf( "Invalid gamepad index: %d. Index should be between 0 and %d.",
         controllerID, MaxGamepads - 1 );
      return -1;
   }

   const GamepadSlot& gp = sGamepads[controllerID];
   if ( gp.handle == NULL )
      return -1;

   const Sint16* axes = current ? gp.rawAxes : gp.prevRawAxes;
   const bool* btns = current ? gp.buttons : gp.prevButtons;

   switch ( property )
   {
      case XI_THUMBLX:          return S32( axes[SDL_GAMEPAD_AXIS_LEFTX] );
      case XI_THUMBLY:          return S32( axes[SDL_GAMEPAD_AXIS_LEFTY] );
      case XI_THUMBRX:          return S32( axes[SDL_GAMEPAD_AXIS_RIGHTX] );
      case XI_THUMBRY:          return S32( axes[SDL_GAMEPAD_AXIS_RIGHTY] );
      case XI_LEFT_TRIGGER:     return ( S32( axes[SDL_GAMEPAD_AXIS_LEFT_TRIGGER] ) * 255 ) / 32767;
      case XI_RIGHT_TRIGGER:    return ( S32( axes[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER] ) * 255 ) / 32767;

      case SI_UPOV:             return btns[SDL_GAMEPAD_BUTTON_DPAD_UP] ? 1 : 0;
      case SI_DPOV:             return btns[SDL_GAMEPAD_BUTTON_DPAD_DOWN] ? 1 : 0;
      case SI_LPOV:             return btns[SDL_GAMEPAD_BUTTON_DPAD_LEFT] ? 1 : 0;
      case SI_RPOV:             return btns[SDL_GAMEPAD_BUTTON_DPAD_RIGHT] ? 1 : 0;
      case XI_START:            return btns[SDL_GAMEPAD_BUTTON_START] ? 1 : 0;
      case XI_BACK:             return btns[SDL_GAMEPAD_BUTTON_BACK] ? 1 : 0;
      case XI_GUIDE:            return btns[SDL_GAMEPAD_BUTTON_GUIDE] ? 1 : 0;
      case XI_MISC1:            return btns[SDL_GAMEPAD_BUTTON_MISC1] ? 1 : 0;
      case XI_TOUCHPAD:         return btns[SDL_GAMEPAD_BUTTON_TOUCHPAD] ? 1 : 0;
      case XI_LEFT_THUMB:       return btns[SDL_GAMEPAD_BUTTON_LEFT_STICK] ? 1 : 0;
      case XI_RIGHT_THUMB:      return btns[SDL_GAMEPAD_BUTTON_RIGHT_STICK] ? 1 : 0;
      case XI_LEFT_SHOULDER:    return btns[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] ? 1 : 0;
      case XI_RIGHT_SHOULDER:   return btns[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] ? 1 : 0;
      case XI_A:                return btns[SDL_GAMEPAD_BUTTON_SOUTH] ? 1 : 0;
      case XI_B:                return btns[SDL_GAMEPAD_BUTTON_EAST] ? 1 : 0;
      case XI_X:                return btns[SDL_GAMEPAD_BUTTON_WEST] ? 1 : 0;
      case XI_Y:                return btns[SDL_GAMEPAD_BUTTON_NORTH] ? 1 : 0;
      case XI_PADDLE1:          return btns[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1] ? 1 : 0;
      case XI_PADDLE2:          return btns[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2] ? 1 : 0;
      case XI_PADDLE3:          return btns[SDL_GAMEPAD_BUTTON_LEFT_PADDLE1] ? 1 : 0;
      case XI_PADDLE4:          return btns[SDL_GAMEPAD_BUTTON_LEFT_PADDLE2] ? 1 : 0;
      default:                  return -1;
   }
}

bool SDLGamepad::isJoystickDetected()
{
   return getJoystickCount() > 0 || getGamepadCount() > 0;
}

S32 SDLGamepad::getJoystickCount()
{
   if ( !sInitialized )
      return 0;

   S32 count = 0;
   for ( S32 i = 0; i < MaxJoysticks; ++i )
   {
      if ( sJoysticks[i].handle != NULL )
         ++count;
   }
   return count;
}

const char* SDLGamepad::getJoystickAxesString( U32 deviceID )
{
   if ( !sInitialized || deviceID >= MaxJoysticks || sJoysticks[deviceID].handle == NULL )
      return "";

   const JoystickSlot& js = sJoysticks[deviceID];
   char buf[128];
   dSprintf( buf, sizeof( buf ), "%d", js.axisList.size() );

   for ( S32 i = 0; i < js.axisList.size(); ++i )
   {
      switch ( js.axisList[i].type )
      {
         case SI_XAXIS:  dStrcat( buf, "\tX" ); break;
         case SI_YAXIS:  dStrcat( buf, "\tY" ); break;
         case SI_ZAXIS:  dStrcat( buf, "\tZ" ); break;
         case SI_RXAXIS: dStrcat( buf, "\tR" ); break;
         case SI_RYAXIS: dStrcat( buf, "\tU" ); break;
         case SI_RZAXIS: dStrcat( buf, "\tV" ); break;
         case SI_SLIDER: dStrcat( buf, "\tS" ); break;
         default:        break;
      }
   }

   char* returnString = Con::getReturnBuffer( dStrlen( buf ) + 1 );
   dStrcpy( returnString, buf );
   return returnString;
}

void SDLGamepad::echoInputState()
{
   Con::printf( "- SDL3 Gamepad/Joystick subsystem is %s, %sactive (%d gamepad(s), %d joystick(s)).",
      sInitialized ? "initialized" : "uninitialized",
      sActive ? "" : "in",
      getGamepadCount(),
      getJoystickCount() );
}

#endif // TORQUE_SDL || TORQUE_SDL_GAMEPAD
