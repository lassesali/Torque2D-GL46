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

#ifndef _X86UNIXINPUTMANAGER_H_
#define _X86UNIXINPUTMANAGER_H_

#include "collection/vector.h"
#include "platform/platformInput.h"
#include "platformX86UNIX/platformX86UNIX.h"

#include <SDL3/SDL.h>

//------------------------------------------------------------------------------
class UInputManager : public InputManager
{
   public:
      UInputManager();

      void init();
      bool enable();
      void disable();
      void activate();
      void deactivate();
      void setWindowLocked(bool locked);
      bool isActive()               { return( mActive ); }

      void onDeleteNotify( SimObject* object );
      bool onAdd();
      void onRemove();

      // Polls the SDL3 gamepad/joystick manager. Keyboard and mouse events
      // arrive through processEvent instead, from the one loop that empties SDL's queue.
      void process();

      // An input event from SDL's queue (x86UNIXWindow.cc): keys, typed text,
      // mouse motion, buttons and the wheel.
      void processEvent(const SDL_Event& event);

      bool enableKeyboard();
      void disableKeyboard();
      bool isKeyboardEnabled()      { return( mKeyboardEnabled ); }
      bool activateKeyboard();
      void deactivateKeyboard();
      bool isKeyboardActive()       { return( mKeyboardActive ); }

      bool enableMouse();
      void disableMouse();
      bool isMouseEnabled()         { return( mMouseEnabled ); }
      bool activateMouse();
      void deactivateMouse();
      bool isMouseActive()          { return( mMouseActive ); }

      bool enableJoystick();
      void disableJoystick();
      bool isJoystickEnabled();
      bool activateJoystick();
      void deactivateJoystick();
      bool isJoystickActive();

      void setLocking(bool enabled);
      bool getLocking() { return mLocking; }

      const char* getJoystickAxesString( U32 deviceID );
      bool joystickDetected();
   private:
      typedef SimGroup Parent;

      bool mKeyboardEnabled;
      bool mMouseEnabled;

      bool mKeyboardActive;
      bool mMouseActive;

      bool mActive;

      // Device state variables
      enum { NumMouseButtons = 5 }; // left, right, middle and the two side buttons
      bool mKeyboardState[256];
      bool mMouseButtonState[NumMouseButtons];

      // last mousex and y are maintained when window is unlocked
      S32 mLastMouseX;
      S32 mLastMouseY;

      void resetKeyboardState();
      void resetMouseState();
      void resetInputState();

      void lockInput();
      void unlockInput();
      bool mLocking;

      void mouseButtonEvent(const SDL_Event& event);
      void mouseMotionEvent(const SDL_Event& event);
      void mouseWheelEvent(const SDL_Event& event);
      void keyEvent(const SDL_Event& event);
};

#endif  // _H_X86UNIXINPUTMANAGER_
