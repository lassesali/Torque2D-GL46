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

#ifndef TORQUE_SHIPPING

#ifndef _UNIT_TESTING_H_
#include "testing/unitTesting.h"
#endif

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _EVENT_H_
#include "platform/event.h"
#endif

#ifndef _ACTIONMAP_H_
#include "input/actionMap.h"
#endif

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

#if defined(TORQUE_SDL) || defined(TORQUE_SDL_GAMEPAD)
#ifndef _SDLGAMEPAD_H_
#include "platformSDL/sdlGamepad.h"
#endif
#endif

//-----------------------------------------------------------------------------

TEST( SdlGamepadTests, ActionMapGamepadTokensResolveCrossPlatform )
{
   struct TokenCase
   {
      const char* token;
      U8 expectedType;
      U16 expectedCode;
   };

   static const TokenCase kCases[] =
   {
      { "connect",      XI_BUTTON, XI_CONNECT },
      { "thumblx",      XI_AXIS,   XI_THUMBLX },
      { "thumbly",      XI_AXIS,   XI_THUMBLY },
      { "thumbrx",      XI_AXIS,   XI_THUMBRX },
      { "thumbry",      XI_AXIS,   XI_THUMBRY },
      { "triggerl",     XI_AXIS,   XI_LEFT_TRIGGER },
      { "triggerr",     XI_AXIS,   XI_RIGHT_TRIGGER },
      { "dpadu",        XI_BUTTON, SI_UPOV },
      { "dpadd",        XI_BUTTON, SI_DPOV },
      { "dpadl",        XI_BUTTON, SI_LPOV },
      { "dpadr",        XI_BUTTON, SI_RPOV },
      { "btn_start",    XI_BUTTON, XI_START },
      { "btn_back",     XI_BUTTON, XI_BACK },
      { "btn_guide",    XI_BUTTON, XI_GUIDE },
      { "btn_misc1",    XI_BUTTON, XI_MISC1 },
      { "btn_touchpad", XI_BUTTON, XI_TOUCHPAD },
      { "btn_lt",       XI_BUTTON, XI_LEFT_THUMB },
      { "btn_rt",       XI_BUTTON, XI_RIGHT_THUMB },
      { "btn_l",        XI_BUTTON, XI_LEFT_SHOULDER },
      { "btn_r",        XI_BUTTON, XI_RIGHT_SHOULDER },
      { "btn_a",        XI_BUTTON, XI_A },
      { "btn_b",        XI_BUTTON, XI_B },
      { "btn_x",        XI_BUTTON, XI_X },
      { "btn_y",        XI_BUTTON, XI_Y },
      { "btn_paddle1",  XI_BUTTON, XI_PADDLE1 },
      { "btn_paddle2",  XI_BUTTON, XI_PADDLE2 },
      { "btn_paddle3",  XI_BUTTON, XI_PADDLE3 },
      { "btn_paddle4",  XI_BUTTON, XI_PADDLE4 }
   };

   for ( U32 i = 0; i < sizeof( kCases ) / sizeof( kCases[0] ); ++i )
   {
      EventDescriptor desc;
      dMemset( &desc, 0, sizeof( desc ) );
      ASSERT_TRUE( ActionMap::createEventDescriptor( kCases[i].token, &desc ) )
         << "Failed to resolve ActionMap token: " << kCases[i].token;
      EXPECT_EQ( kCases[i].expectedType, desc.eventType )
         << "Unexpected eventType for token: " << kCases[i].token;
      EXPECT_EQ( kCases[i].expectedCode, desc.eventCode )
         << "Unexpected eventCode for token: " << kCases[i].token;
   }
}

//-----------------------------------------------------------------------------

TEST( SdlGamepadTests, GamepadAndJoystickDeviceNamesRoundTrip )
{
   for ( U32 slot = 0; slot < 8; ++slot )
   {
      char name[32];
      U32 devType = 0;
      U32 devInst = 99;

      dSprintf( name, sizeof( name ), "gamepad%u", slot );
      ASSERT_TRUE( ActionMap::getDeviceTypeAndInstance( name, devType, devInst ) );
      EXPECT_EQ( (U32)GamepadDeviceType, devType );
      EXPECT_EQ( slot, devInst );

      char outName[32];
      ASSERT_TRUE( ActionMap::getDeviceName( GamepadDeviceType, slot, outName ) );
      EXPECT_STREQ( name, outName );

      dSprintf( name, sizeof( name ), "joystick%u", slot );
      ASSERT_TRUE( ActionMap::getDeviceTypeAndInstance( name, devType, devInst ) );
      EXPECT_EQ( (U32)JoystickDeviceType, devType );
      EXPECT_EQ( slot, devInst );

      ASSERT_TRUE( ActionMap::getDeviceName( JoystickDeviceType, slot, outName ) );
      EXPECT_STREQ( name, outName );
   }
}

#if defined(TORQUE_SDL) || defined(TORQUE_SDL_GAMEPAD)

//-----------------------------------------------------------------------------

TEST( SdlGamepadTests, RadialDeadzoneZeroesAndRescalesCorrectly )
{
   const F32 deadzone = 0.24f;
   F32 outX = -1.0f;
   F32 outY = -1.0f;

   SDLGamepad::applyRadialDeadzone( 0.0f, 0.0f, deadzone, outX, outY );
   EXPECT_FLOAT_EQ( 0.0f, outX );
   EXPECT_FLOAT_EQ( 0.0f, outY );

   // Diagonal magnitude is sqrt(0.15^2 + 0.15^2) ~= 0.2121 < 0.24: must snap to zero.
   SDLGamepad::applyRadialDeadzone( 0.15f, 0.15f, deadzone, outX, outY );
   EXPECT_FLOAT_EQ( 0.0f, outX );
   EXPECT_FLOAT_EQ( 0.0f, outY );

   // Exact boundary zeroes cleanly.
   SDLGamepad::applyRadialDeadzone( deadzone, 0.0f, deadzone, outX, outY );
   EXPECT_FLOAT_EQ( 0.0f, outX );
   EXPECT_FLOAT_EQ( 0.0f, outY );

   // Mid-point (0.62 - 0.24) / (1.0 - 0.24) == 0.5
   SDLGamepad::applyRadialDeadzone( 0.62f, 0.0f, deadzone, outX, outY );
   EXPECT_NEAR( 0.5f, outX, 1e-4f );
   EXPECT_FLOAT_EQ( 0.0f, outY );

   SDLGamepad::applyRadialDeadzone( -0.62f, 0.0f, deadzone, outX, outY );
   EXPECT_NEAR( -0.5f, outX, 1e-4f );
   EXPECT_FLOAT_EQ( 0.0f, outY );

   // Full range reaches unit circle without exceeding magnitude 1.
   SDLGamepad::applyRadialDeadzone( 1.0f, 0.0f, deadzone, outX, outY );
   EXPECT_NEAR( 1.0f, outX, 1e-4f );
   EXPECT_FLOAT_EQ( 0.0f, outY );

   SDLGamepad::applyRadialDeadzone( 0.9f, 0.9f, deadzone, outX, outY );
   const F32 diagMag = mSqrt( outX * outX + outY * outY );
   EXPECT_LE( diagMag, 1.0001f );
   EXPECT_NEAR( outX, outY, 1e-4f );
}

//-----------------------------------------------------------------------------

TEST( SdlGamepadTests, TriggerDeadzoneThresholdsAndRescales )
{
   const F32 deadzone = 0.12f;

   EXPECT_FLOAT_EQ( 0.0f, SDLGamepad::applyTriggerDeadzone( -0.5f, deadzone ) );
   EXPECT_FLOAT_EQ( 0.0f, SDLGamepad::applyTriggerDeadzone( 0.0f, deadzone ) );
   EXPECT_FLOAT_EQ( 0.0f, SDLGamepad::applyTriggerDeadzone( 0.12f, deadzone ) );
   EXPECT_NEAR( 0.5f, SDLGamepad::applyTriggerDeadzone( 0.56f, deadzone ), 1e-4f );
   EXPECT_FLOAT_EQ( 1.0f, SDLGamepad::applyTriggerDeadzone( 1.0f, deadzone ) );
   EXPECT_FLOAT_EQ( 1.0f, SDLGamepad::applyTriggerDeadzone( 1.5f, deadzone ) );
}

//-----------------------------------------------------------------------------

TEST( SdlGamepadTests, ConsoleScriptBindingsRegisteredAndSafe )
{
   static const char* kExpectedFunctions[] =
   {
      "getGamepadCount",
      "isGamepadConnected",
      "getGamepadName",
      "getGamepadType",
      "getGamepadButtonLabel",
      "setGamepadRumble",
      "setGamepadTriggerRumble",
      "enableJoystick",
      "disableJoystick",
      "isJoystickDetected",
      "getJoystickCount",
      "getJoystickAxes",
      "rumble",
      "enableXInput",
      "disableXInput",
      "resetXInput",
      "isXInputConnected",
      "getXInputState"
   };

   for ( U32 i = 0; i < sizeof( kExpectedFunctions ) / sizeof( kExpectedFunctions[0] ); ++i )
   {
      EXPECT_TRUE( Con::isFunction( kExpectedFunctions[i] ) )
         << "Missing expected TorqueScript function: " << kExpectedFunctions[i];
   }

   EXPECT_GE( dAtoi( Con::evaluate( "return getGamepadCount();" ) ), 0 );
   EXPECT_GE( dAtoi( Con::evaluate( "return getJoystickCount();" ) ), 0 );
   EXPECT_FALSE( dAtob( Con::evaluate( "return isGamepadConnected(99);" ) ) );
   EXPECT_STREQ( "unknown", Con::evaluate( "return getGamepadType(99);" ) );
   EXPECT_STREQ( "", Con::evaluate( "return getGamepadName(99);" ) );
   EXPECT_STREQ( "", Con::evaluate( "return getGamepadButtonLabel(99, \"btn_a\");" ) );
   EXPECT_EQ( -1, dAtoi( Con::evaluate( "return getXInputState(99, \"XI_GUIDE\", true);" ) ) );
   EXPECT_EQ( -1, dAtoi( Con::evaluate( "return getXInputState(99, \"XI_PADDLE1\", false);" ) ) );
}

#endif // TORQUE_SDL || TORQUE_SDL_GAMEPAD

#endif // TORQUE_SHIPPING
