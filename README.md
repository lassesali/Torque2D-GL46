![Torque Logo](images/banner1.png)
## Torque2D 4.0 Early Access 4

MIT Licensed Open Source version of Torque2D from GarageGames. Maintained by the Torque Game Engines team and contributions from the community.

Dedicated to 2D game development, Torque 2D is an extremely powerful, flexible, and fast C++ engine which has been used in hundreds of professional games. It is a true cross platform solution providing you access to Windows, OSX, Linux, iOS, Android, and the web - all from one codebase. It includes an OpenGL 4.6 Core / OpenGL ES 3.2 batched rendering system, Box2D physics, OpenAL audio, skeletal and spritesheet animation, automated asset management, a modular project structure, TAML object persistence, and a C-like scripting language.

### What's New?

Torque2D 4.0: Rocket Edition is currently in progress. The major change with 4.0 is the addition of editors! At this time there is a working Asset Manager and Project Manager. The Asset Manager allows a user to create, edit, and remove images, animations, particle effects, bitmap fonts, and audio assets. This represents a major step forward from editing xml files by hand. This is especially useful in the case of particle effects which are nearly impossible to create without an interactive tool. The Project Manager allows users to manager the modules in their game directly in a user interface. This is paired with a library of re-usable modules that can be imported into a game.

The managers can be reached by opening the console using the console button in the Toybox or by pressing Tilde(~) + Ctrl. You will then notice tabs along that top for the various tools currently available.

Early Access 4 builds out the **GUI Editor** that Early Access 3 introduced. You build a screen by dragging controls from an illustrated palette onto the canvas, or by clicking one to have it placed for you, and arrange them by dragging, by the arrow keys, or through the Layout menu's align and spacing commands. There is undo and redo, and cut, copy and paste work within a Gui and between them.

Around the canvas sit two more panels. The **Explorer** shows the whole control tree, with a picture of each control's class, columns for hiding and locking, and drag-to-reparent. The **properties pane** shows only the fields the selected control's class actually reads — a chain never draws its own text, so it is not offered nine text fields it will ignore — and gives the common ones purpose-built editors rather than text boxes: an anchor picker for sizing, color swatches, an image picker you choose by looking at it, and editors for the things that used to be unreachable from an editor at all, such as a list box's rows and a menu bar's items.

Guis are saved as either the classic `.gui` script or TAML, and the editor says which one loses what before you pick. A Gui you have changed is not thrown away without being asked about.

Controls take their appearance from a **theme** rather than from profiles you wire up by hand. The **Gui Profile Editor** is where a theme is authored — profiles, borders, fonts and colors, against a live preview — and a control dropped onto the canvas arrives already wearing the right one. Set Theme re-skins an entire Gui.

The Rocket Edition also features a revamped Gui System! Until now it has been a common practice among those seriously using T2D to avoid the Gui System as much as possible. We aim to fix that with the Rocket Edition. Explanation of how to use the updated Gui System can be found in the wiki in the [Gui Guide](https://github.com/TorqueGameEngines/Torque2D/wiki/GUI-Guide).

The rendering backend has been migrated from legacy fixed-function OpenGL 1.x / OpenGL ES 1.1 to **OpenGL 4.6 Core Profile** (`#version 460 core`) on desktop and **OpenGL ES 3.2** (`#version 320 es` with highp precision qualifiers) on Android/mobile, with WebGL 2 shader flags (`-sMIN_WEBGL_VERSION=2` / `-sMAX_WEBGL_VERSION=2`) integrated with SDL 2 on Emscripten. Rendering runs through `ShaderManager` programmable shaders with VAO/VBO/IBO streaming batching, `GLDSA` Direct State Access wrappers (with bind-based fallbacks for GLES 3.2 and ANGLE/macOS), and CPU `dgl` matrix stacks. See [opengl_migration_report.txt](opengl_migration_report.txt) (and [opengl_migration_report.pdf](opengl_migration_report.pdf)) for architecture, merged changes, and verification details.

More features and editors are coming before 4.0 is officially done, but new projects should make use of the Early Access version to avoid future conflicts.

### Branches

Here is an overview of the branches found in the Torque2D repository:

* **master:** this branch contains the current stable release code that can be used in a production environment.
* **development:** this branch is dedicated to active development. It contains the latest bug fixes, new features, and other updates. All pull requests need to go to the development branch. While we try our best to test all incoming changes, it is possible for mistakes to slip in therefore this branch should always be considered unstable.
* **gh-pages:** this branch currently contains the html pages generated from doxygen for the engine and TorqueScript references.

### Precompiled Version

If you do not wish to compile the source code yourself, precompiled binary files for Windows and OSX are available from the [Torque 2D Release Page](https://github.com/TorqueGameEngines/Torque2D/releases).

### Building the Source

**CMake is the single source of truth for the build.** You generate a project for your platform/toolchain from the root `CMakeLists.txt` and build it; the compiled executable is written to the repository root. Convenience generator scripts live at the repo root:

* **Windows:** `generate-vs2022.bat` (or `generate-vs2026.bat`) → a Visual Studio solution
* **macOS:** `generate-xcode.command` → an Xcode project
* **Linux:** `build-linux.sh` (configures and builds; 32- and 64-bit)
* **iOS:** `generate-xcode-ios.command` (simulator) or `generate-xcode-ios-device.command` (device)
* **Android:** open `engine/compilers/android-studio` in Android Studio — its Gradle build drives CMake via the NDK
* **Web:** `generate-emscripten.sh` → a WebAssembly build via `emcmake` (requires the Emscripten SDK)

The hand-maintained per-platform project files that used to live in `engine/compilers/` have been removed — CMake replaces them. For full step-by-step build instructions on every platform, see the [Torque2D wiki](https://github.com/TorqueGameEngines/Torque2D/wiki) (the *Building from Source* guide).

#### Generating a Visual Studio solution with CMake

Generating a fresh, always-up-to-date Visual Studio solution from CMake takes just a few steps. You do **not** need to know anything about CMake to do this.

1. **Install Visual Studio 2022 or Visual Studio 2026** (Community Edition is fine). In the Visual Studio Installer, make sure the **"Desktop development with C++"** workload is checked.
2. **Install CMake** from [cmake.org/download](https://cmake.org/download/) (CMake 4.x or newer is required for Visual Studio 2026). On the *Install Options* screen, choose **"Add CMake to the system PATH for all users"** (or for the current user). This one-time step is what lets the generator find CMake.
3. In the root of the repository, **double-click `generate-vs2022.bat` or `generate-vs2026.bat`**. It creates the solution under `build\vs2022\` or `build\vs2026\` and opens `Torque2D.sln` in Visual Studio. (If CMake or the C++ workload is missing, the script tells you what to fix.)
4. In Visual Studio, choose a configuration (**Debug** or **Release**) at the top, then build with **Build → Build Solution** (`Ctrl+Shift+B`), or build from CMake directly (`cmake --build build\vs2026 --config Release --parallel`).
5. The compiled executable is written to the repository root (`Torque2D_DEBUG.exe` for Debug, `Torque2D.exe` for Release). Run it from there (press **F5** in Visual Studio, which is already set to launch from the repo root).

Whenever the engine's source file list changes (for example after pulling new changes), just **re-run `generate-vs2022.bat` or `generate-vs2026.bat`** to regenerate the solution.

See the [wiki](https://github.com/TorqueGameEngines/Torque2D/wiki) for available guides on platform setup and development.

#### Running the Tests

After building `Torque2D_DEBUG.exe` (or `Torque2D.exe` for `-Release` unit runs) in the repository root, run the automated test suites with PowerShell 7 (`pwsh`):

* **C++ unit tests (GoogleTest, Debug):** `pwsh -ExecutionPolicy Bypass -File tests\run-unit.ps1`
* **C++ unit tests (GoogleTest, Release):** `pwsh -ExecutionPolicy Bypass -File tests\run-unit.ps1 -Release`
* **TorqueScript integration / smoke tests (Debug):** `pwsh -ExecutionPolicy Bypass -File tests\run.ps1` (`-Shots` for screenshot harnesses)

Logs are written under `tests\logs\`.

### Batteries Included

When you first run Torque2D, you'll have the option to create a project or open the Toybox. The Toybox is a collection of over 30 simple "toys" (or modules) which demonstrate various features in T2D. The default toy is a side scrolling level with a monster truck. To see a list of the available modules/toys to choose from, click on the `Show Tools` button in the lower right corner of the screen.

Naturally all of the script code and assets for each toy are available to you in the toybox folder to use as practical examples while learning T2D.

### Documentation

All documentation for the Torque2D can be found on our [Github wiki page](https://github.com/TorqueGameEngines/Torque2D/wiki). It contains many tutorials, detailed technical information on engine systems, a script reference guide automatically generated from the source code, and articles on how to contribute to our open source development.

### Community

Don't go it alone! Join the active Torque community. Ask questions, talk about T2D and general game development topics, learn the latest news, or post a blog promoting your game or showing off additional engine features in your T2D fork.

* [Torque 2D Forums on the Torque Game Engines Website](https://torque3d.org/forums/forum/26-general/)
* [Torque Game Engines on Discord](https://discord.com/invite/qdAZxT4)

Please note that the GarageGames website is gone. The options above both represent great ways to get help if you need it.

# License

Copyright (c) 2012 GarageGames, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
