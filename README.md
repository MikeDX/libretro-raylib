# libretro_raylib

A Libretro frontend implementation using Raylib for macOS.

Created by [mikedx](https://github.com/mikedx/)

## License

This project is licensed under the MIT License.

### Third-Party Licenses

This project uses the following third-party libraries:

- **libretro API**: The libretro API specification is used under the BSD 3-Clause License.
  See https://www.libretro.com/ for details.

- **raylib**: Used under the zlib/libpng license.
  See https://github.com/raysan5/raylib for details.

## Building

### Prerequisites

- macOS with Xcode Command Line Tools
- raylib installed in `../raylib/src`
- A libretro core (`.dylib` file) for your target platform

### Downloading Cores

You can use the included script to download cores:

```bash
./download_cores.sh
```

This will:
- Fetch the list of available cores from the libretro buildbot
- Display them in a menu
- Allow you to download individual cores or all cores
- Save them to the `cores/` directory

### Build Instructions

```bash
make
```

This will create the `libretro_raylib` executable.

## Usage

```bash
./libretro_raylib <path_to_core.dylib> [rom_file]
```

### Examples

```bash
# Load a core with a ROM file (using downloaded core)
./libretro_raylib cores/mgba_libretro.dylib mike_test.gba

# Load a core without a ROM (for cores that support no-game mode)
./libretro_raylib cores/mgba_libretro.dylib

# Or use a core from any location
./libretro_raylib /path/to/core.dylib /path/to/rom.gba
```

## Controls

- **Arrow Keys / WASD** - D-Pad
- **X** - A button
- **Z** - B button
- **C** - X button
- **V** - Y button
- **Q** - L button
- **E** - R button
- **TAB** - Select
- **ENTER** - Start
- **ESC** - Exit

## Features

- Dynamic loading of libretro cores
- Video rendering with pixel format conversion
- Audio playback with ring buffer management
- Keyboard input mapping
- Aspect ratio preservation
- Modular architecture for maintainability and extensibility

## Architecture

The frontend is organized into modular components:

### Core Modules

- **`libretro_api.h`** - Libretro API constants, structures, and type definitions
  - All libretro constants and enums
  - Core function pointer types
  - Callback type definitions

- **`libretro_frontend.h/c`** - Main public API
  - Coordinates all modules
  - Public API functions for core management
  - Frontend state management

### Callback Modules

- **`libretro_environment.h/c`** - Environment callback implementation
  - Handles core requests for system information
  - Pixel format configuration
  - Directory paths and system settings
  - Log interface support

- **`libretro_video.h/c`** - Video refresh callback
  - Receives rendered frames from cores
  - Pixel format conversion (RGB565, XRGB8888, 0RGB1555)
  - Framebuffer management
  - Auto-detection of pixel formats

- **`libretro_audio.h/c`** - Audio callbacks
  - Single-sample and batch audio callbacks
  - Ring buffer management for audio streaming
  - Audio format conversion (int16_t to float)
  - Handles cores that use single-sample callbacks (e.g., xrick)

- **`libretro_input.h/c`** - Input handling
  - Input poll callback
  - Input state queries (joypad and keyboard)
  - Keyboard keycode mapping

### Core Management

- **`libretro_core.h/c`** - Core loading and management
  - Dynamic library loading (dlopen)
  - Core symbol resolution
  - Core initialization
  - ROM loading (supports both fullpath and memory-based loading)
  - Audio/video info updates
  - Core cleanup and resource management

### Application

- **`main.c`** - Main entry point and raylib integration
  - Window management
  - Input handling (keyboard to libretro mapping)
  - Audio stream management
  - Frame rendering loop

The modular design provides:
- **Separation of concerns**: Each module handles a specific aspect of the libretro API
- **Maintainability**: Easier to locate and modify specific functionality
- **Testability**: Modules can be tested independently
- **Readability**: Smaller, focused files are easier to understand

## Notes

- The frontend supports pixel formats:
  - **XRGB8888** (format 1): 32-bit format, 8 bits per channel
  - **RGB565** (format 2): 16-bit format, 5-6-5 bits (R-G-B), used by mGBA
  - **0RGB1555** (format 0): 16-bit format, 5-5-5 bits (R-G-B), used by bsnes
  - **RGB555** (format 12): 16-bit format, 5-5-5 bits (R-G-B), used by snes9x
- Audio is converted from int16_t samples to float for raylib
- Video frames are converted to RGBA8888 format for rendering
- Audio uses a ring buffer to handle timing variations between core and playback
- Single-sample audio callbacks are supported for cores like xrick

## Known Issues

- **bsnes with Super Mario Kart**: bsnes crashes when loading Super Mario Kart (`smk.sfc`). This appears to be a core-specific issue. Super Mario Kart works correctly with snes9x. Other SNES games (e.g., Super Mario World) work fine with bsnes.
