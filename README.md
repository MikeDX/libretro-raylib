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
# Load a core with a ROM file
./libretro_raylib mgba_libretro.dylib mike_test.gba

# Load a core without a ROM (for cores that support no-game mode)
./libretro_raylib mgba_libretro.dylib
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

## Architecture

The frontend consists of:

- `main.c` - Main entry point and raylib integration
- `libretro_frontend.h` - Public API header
- `libretro_frontend.c` - Libretro API implementation

The frontend implements the libretro API callbacks for:
- Environment queries
- Video refresh
- Audio sample batching
- Input polling and state

## Notes

- The frontend supports pixel formats: XRGB8888, RGB565, and 0RGB1555
- Audio is converted from int16_t samples to float for raylib
- Video frames are converted to RGBA8888 format for rendering
- Audio uses a ring buffer to handle timing variations between core and playback
