# Vendored SDL3 for Torque2D

This directory contains SDL **3.2.30** (`release-3.2.30`, upstream commit
`f5e5f6588921eed3d7d048ce43d9eb1ff0da0ffc`) from
https://github.com/libsdl-org/SDL with `.git/` and `test/` removed.

## How Torque2D Uses SDL3

- **Linux (`TORQUE_SDL` + `TORQUE_SDL_GAMEPAD`)**: Builds `SDL3::SDL3-static`
  with video, events, joystick, gamepad, and haptic support enabled.
- **Windows (`TORQUE_SDL_GAMEPAD`)**: Keeps native Win32 windowing (`winWindow.cc`)
  and OpenGL context creation, and links `SDL3::SDL3-static` strictly for
  gamepad, joystick, HIDAPI, and haptic support (`SDL_VIDEO=OFF`,
  `SDL_AUDIO=OFF`, `SDL_RENDER=OFF`, `SDL_CAMERA=OFF`, `SDL_GPU=OFF`,
  `SDL_DIALOG=OFF`).
- **macOS (`TORQUE_SDL_GAMEPAD`)**: Keeps native Cocoa windowing
  (`osxTorqueView.mm`) and OpenGL context creation, and links
  `SDL3::SDL3-static` strictly for gamepad, joystick, HIDAPI, and haptic support.
- **Web / Emscripten (`TORQUE_SDL` + `TORQUE_SDL_GAMEPAD`)**: Uses Emscripten's
  SDL3 port (`-sUSE_SDL=3`) by default, or vendored `SDL3::SDL3-static` when
  `-DTORQUE_EMSCRIPTEN_VENDORED_SDL3=ON` is passed to CMake.

## Updating

To update to a newer SDL3 release:
1. Replace this directory with the clean upstream SDL3 release source tree
   (excluding `.git/` and `test/`).
2. Verify CMake static target configuration in `engine/lib/CMakeLists.txt`
   and `CMakeLists.txt`.
3. Rebuild and run unit tests (`tests/run-unit.ps1` on Windows, or CTest/unit
   runner on Linux/macOS).
4. Update this file with the new SDL3 version and upstream commit hash.
