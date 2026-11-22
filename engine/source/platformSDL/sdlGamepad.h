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

#ifndef _SDLGAMEPAD_H_
#define _SDLGAMEPAD_H_

#if defined(TORQUE_SDL) || defined(TORQUE_SDL_GAMEPAD)

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

union SDL_Event;

/// Shared SDL3 gamepad and raw joystick manager.
///
/// Compiled across Windows, macOS, Linux, and Web/Emscripten whenever
/// TORQUE_SDL or TORQUE_SDL_GAMEPAD is defined. Recognized controllers are
/// opened with SDL_Gamepad and routed through GamepadDeviceType
/// (gamepad0..gamepad7), while unmapped devices (HOTAS, flight sticks,
/// racing wheels, arcade sticks) fall back to SDL_Joystick on
/// JoystickDeviceType (joystick0..joystick7).
namespace SDLGamepad
{
   enum
   {
      MaxGamepads = 8,
      MaxJoysticks = 8
   };

   // Lifecycle & event loop integration
   bool init();
   void shutdown();
   void activate();
   void deactivate();
   void process();
   void pumpEvents();
   void processEvent( const SDL_Event& event );

   // Mapping database
   S32 loadMappingsFromFile( const char* path );

   // Enable / disable & state reset
   bool isInitialized();
   bool isActive();
   bool isGamepadEnabled();
   void setGamepadEnabled( bool enabled );
   bool enableJoystick();
   void disableJoystick();
   bool isJoystickEnabled();
   void resetGamepadStates();

   // Analog processing helpers (2D radial stick deadzone & axial trigger deadzone)
   void applyRadialDeadzone( F32 rawX, F32 rawY, F32 deadzone, F32& outX, F32& outY );
   F32 applyTriggerDeadzone( F32 rawValue, F32 deadzone );

   // Gamepad queries & haptics (slot 0..MaxGamepads-1)
   S32 getGamepadCount();
   bool isGamepadConnected( S32 slot );
   const char* getGamepadName( S32 slot );
   const char* getGamepadType( S32 slot );
   const char* getRealGamepadType( S32 slot );
   const char* getGamepadButtonLabel( S32 slot, const char* buttonName );
   bool setGamepadRumble( S32 slot, F32 lowFreq, F32 highFreq, U32 durationMs );
   bool setGamepadTriggerRumble( S32 slot, F32 left, F32 right, U32 durationMs );

   // Device-name / legacy XInput & Joystick compatibility
   bool rumbleDevice( const char* deviceName, F32 xRumble, F32 yRumble );
   S32 getXInputState( S32 controllerID, S32 property, bool current );
   bool isJoystickDetected();
   S32 getJoystickCount();
   const char* getJoystickAxesString( U32 deviceID );
   void echoInputState();
}

#endif // TORQUE_SDL || TORQUE_SDL_GAMEPAD

#endif // _SDLGAMEPAD_H_
