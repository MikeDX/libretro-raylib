/*
 * libretro_api.h - Libretro API Constants and Type Definitions
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * This header defines libretro API constants, structures, and types.
 * The libretro API is available under the BSD 3-Clause License.
 * See https://www.libretro.com/ for details.
 */

#ifndef LIBRETRO_API_H
#define LIBRETRO_API_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

//=============================================================================
// Libretro API Constants
//=============================================================================

// Libretro API version
#define RETRO_API_VERSION 1

// Environment commands
#define RETRO_ENVIRONMENT_EXEC 0
#define RETRO_ENVIRONMENT_SET_ROTATION 1
#define RETRO_ENVIRONMENT_GET_OVERSCAN 2
#define RETRO_ENVIRONMENT_GET_CAN_DUPE 3
#define RETRO_ENVIRONMENT_SET_MESSAGE 4
#define RETRO_ENVIRONMENT_SHUTDOWN 5
#define RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL 6
#define RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY 7
#define RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS 9
#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10
#define RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK 12
#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE 11
#define RETRO_ENVIRONMENT_SET_HW_RENDER 12
#define RETRO_ENVIRONMENT_GET_VARIABLE 13
#define RETRO_ENVIRONMENT_SET_VARIABLES 14
#define RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE 15
#define RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME 18
#define RETRO_ENVIRONMENT_GET_LIBRETRO_PATH 19
#define RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK 21
#define RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK 22
#define RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE 23
#define RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES 24
#define RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE 25
#define RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE 26
#define RETRO_ENVIRONMENT_GET_LOG_INTERFACE 27
#define RETRO_ENVIRONMENT_GET_PERF_INTERFACE 28
#define RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE 29
#define RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY 30
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS 28
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL 29
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY 30
#define RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER 31
#define RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION 32
#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE 33
#define RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION 34
#define RETRO_ENVIRONMENT_SET_MESSAGE_EXT 35
#define RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS 36
#define RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK 37
#define RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY 38
#define RETRO_ENVIRONMENT_SET_FASTFORWARDING 39
#define RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE 40
#define RETRO_ENVIRONMENT_GET_GAME_INFO_EXT 41
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 42
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL 43
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK 44
#define RETRO_ENVIRONMENT_SET_VARIABLE 45
#define RETRO_ENVIRONMENT_GET_THROTTLE_STATE 46
#define RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY 31
#define RETRO_ENVIRONMENT_SET_BLOCK_EXTract 48
#define RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT 49
#define RETRO_ENVIRONMENT_GET_VFS_INTERFACE 50
#define RETRO_ENVIRONMENT_GET_LED_INTERFACE 51
#define RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE 52
#define RETRO_ENVIRONMENT_SET_AUDIO_VIDEO_ENABLE 53
#define RETRO_ENVIRONMENT_GET_MIDI_INTERFACE 54
#define RETRO_ENVIRONMENT_GET_FASTFORWARDING 55
#define RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS 57
#define RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE 58
#define RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS 59
#define RETRO_ENVIRONMENT_SET_DESKTOP_MODE 60

// Pixel formats
#define RETRO_PIXEL_FORMAT_0RGB1555 0
#define RETRO_PIXEL_FORMAT_XRGB8888 1
#define RETRO_PIXEL_FORMAT_RGB565 2
#define RETRO_PIXEL_FORMAT_RGB555 12

// Device types
#define RETRO_DEVICE_NONE 0
#define RETRO_DEVICE_JOYPAD 1
#define RETRO_DEVICE_MOUSE 2
#define RETRO_DEVICE_KEYBOARD 3
#define RETRO_DEVICE_LIGHTGUN 4
#define RETRO_DEVICE_ANALOG 5
#define RETRO_DEVICE_POINTER 6

// Joypad button IDs
#define RETRO_DEVICE_ID_JOYPAD_B 0
#define RETRO_DEVICE_ID_JOYPAD_Y 1
#define RETRO_DEVICE_ID_JOYPAD_SELECT 2
#define RETRO_DEVICE_ID_JOYPAD_START 3
#define RETRO_DEVICE_ID_JOYPAD_UP 4
#define RETRO_DEVICE_ID_JOYPAD_DOWN 5
#define RETRO_DEVICE_ID_JOYPAD_LEFT 6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT 7
#define RETRO_DEVICE_ID_JOYPAD_A 8
#define RETRO_DEVICE_ID_JOYPAD_X 9
#define RETRO_DEVICE_ID_JOYPAD_L 10
#define RETRO_DEVICE_ID_JOYPAD_R 11
#define RETRO_DEVICE_ID_JOYPAD_L2 12
#define RETRO_DEVICE_ID_JOYPAD_R2 13
#define RETRO_DEVICE_ID_JOYPAD_L3 14
#define RETRO_DEVICE_ID_JOYPAD_R3 15

//=============================================================================
// Libretro Keyboard Keycodes
//=============================================================================

enum retro_key {
    RETROK_UNKNOWN        = 0,
    RETROK_BACKSPACE      = 8,
    RETROK_TAB            = 9,
    RETROK_CLEAR          = 12,
    RETROK_RETURN         = 13,
    RETROK_PAUSE          = 19,
    RETROK_ESCAPE         = 27,
    RETROK_SPACE          = 32,
    RETROK_EXCLAIM        = 33,
    RETROK_QUOTEDBL       = 34,
    RETROK_HASH           = 35,
    RETROK_DOLLAR         = 36,
    RETROK_AMPERSAND      = 38,
    RETROK_QUOTE          = 39,
    RETROK_LEFTPAREN      = 40,
    RETROK_RIGHTPAREN     = 41,
    RETROK_ASTERISK       = 42,
    RETROK_PLUS           = 43,
    RETROK_COMMA          = 44,
    RETROK_MINUS          = 45,
    RETROK_PERIOD         = 46,
    RETROK_SLASH          = 47,
    RETROK_0              = 48,
    RETROK_1              = 49,
    RETROK_2              = 50,
    RETROK_3              = 51,
    RETROK_4              = 52,
    RETROK_5              = 53,
    RETROK_6              = 54,
    RETROK_7              = 55,
    RETROK_8              = 56,
    RETROK_9              = 57,
    RETROK_COLON          = 58,
    RETROK_SEMICOLON      = 59,
    RETROK_LESS           = 60,
    RETROK_EQUALS         = 61,
    RETROK_GREATER        = 62,
    RETROK_QUESTION       = 63,
    RETROK_AT             = 64,
    RETROK_LEFTBRACKET    = 91,
    RETROK_BACKSLASH      = 92,
    RETROK_RIGHTBRACKET   = 93,
    RETROK_CARET          = 94,
    RETROK_UNDERSCORE     = 95,
    RETROK_BACKQUOTE      = 96,
    RETROK_a              = 97,
    RETROK_b              = 98,
    RETROK_c              = 99,
    RETROK_d              = 100,
    RETROK_e              = 101,
    RETROK_f              = 102,
    RETROK_g              = 103,
    RETROK_h              = 104,
    RETROK_i              = 105,
    RETROK_j              = 106,
    RETROK_k              = 107,
    RETROK_l              = 108,
    RETROK_m              = 109,
    RETROK_n              = 110,
    RETROK_o              = 111,
    RETROK_p              = 112,
    RETROK_q              = 113,
    RETROK_r              = 114,
    RETROK_s              = 115,
    RETROK_t              = 116,
    RETROK_u              = 117,
    RETROK_v              = 118,
    RETROK_w              = 119,
    RETROK_x              = 120,
    RETROK_y              = 121,
    RETROK_z              = 122,
    RETROK_LEFTBRACE      = 123,
    RETROK_BAR            = 124,
    RETROK_RIGHTBRACE     = 125,
    RETROK_TILDE          = 126,
    RETROK_DELETE         = 127,
    RETROK_KP0            = 256,
    RETROK_KP1            = 257,
    RETROK_KP2            = 258,
    RETROK_KP3            = 259,
    RETROK_KP4            = 260,
    RETROK_KP5            = 261,
    RETROK_KP6            = 262,
    RETROK_KP7            = 263,
    RETROK_KP8            = 264,
    RETROK_KP9            = 265,
    RETROK_KP_PERIOD      = 266,
    RETROK_KP_DIVIDE      = 267,
    RETROK_KP_MULTIPLY    = 268,
    RETROK_KP_MINUS       = 269,
    RETROK_KP_PLUS        = 270,
    RETROK_KP_ENTER       = 271,
    RETROK_KP_EQUALS      = 272,
    RETROK_UP             = 273,
    RETROK_DOWN           = 274,
    RETROK_RIGHT          = 275,
    RETROK_LEFT           = 276,
    RETROK_INSERT         = 277,
    RETROK_HOME           = 278,
    RETROK_END            = 279,
    RETROK_PAGEUP         = 280,
    RETROK_PAGEDOWN       = 281,
    RETROK_F1             = 282,
    RETROK_F2             = 283,
    RETROK_F3             = 284,
    RETROK_F4             = 285,
    RETROK_F5             = 286,
    RETROK_F6             = 287,
    RETROK_F7             = 288,
    RETROK_F8             = 289,
    RETROK_F9             = 290,
    RETROK_F10            = 291,
    RETROK_F11            = 292,
    RETROK_F12            = 293,
    RETROK_NUMLOCK        = 300,
    RETROK_CAPSLOCK       = 301,
    RETROK_SCROLLOCK      = 302,
    RETROK_RSHIFT         = 303,
    RETROK_LSHIFT         = 304,
    RETROK_RCTRL          = 305,
    RETROK_LCTRL          = 306,
    RETROK_RALT           = 307,
    RETROK_LALT           = 308,
    RETROK_LSUPER         = 311,
    RETROK_RSUPER         = 312,
    RETROK_MODE           = 313,
    RETROK_HELP           = 315,
    RETROK_PRINT          = 316,
    RETROK_SYSREQ         = 317,
    RETROK_BREAK          = 318,
    RETROK_MENU           = 319,
    RETROK_POWER          = 320
};

#define RETROK_LAST 321

//=============================================================================
// Libretro API Structures
//=============================================================================

struct retro_game_geometry {
    unsigned base_width;
    unsigned base_height;
    unsigned max_width;
    unsigned max_height;
    float aspect_ratio;
};

struct retro_system_timing {
    double fps;
    double sample_rate;
};

struct retro_system_info {
    const char* library_name;
    const char* library_version;
    const char* valid_extensions;
    bool need_fullpath;
    bool block_extract;
};

struct retro_system_av_info {
    struct retro_game_geometry geometry;
    struct retro_system_timing timing;
};

struct retro_game_info {
    const char* path;
    const void* data;
    size_t size;
    const char* meta;
};

struct retro_variable {
    const char* key;
    const char* value;
};

//=============================================================================
// Libretro Core Function Pointers
//=============================================================================

struct retro_core_t {
    unsigned api_version;
    
    void (*retro_init)(void);
    void (*retro_deinit)(void);
    unsigned (*retro_api_version)(void);
    void (*retro_get_system_info)(struct retro_system_info*);
    void (*retro_get_system_av_info)(struct retro_system_av_info*);
    void (*retro_set_controller_port_device)(unsigned port, unsigned device);
    void (*retro_reset)(void);
    void (*retro_run)(void);
    size_t (*retro_serialize_size)(void);
    bool (*retro_serialize)(void* data, size_t size);
    bool (*retro_unserialize)(const void* data, size_t size);
    void (*retro_cheat_reset)(void);
    void (*retro_cheat_set)(unsigned index, bool enabled, const char* code);
    bool (*retro_load_game)(const struct retro_game_info*);
    void (*retro_unload_game)(void);
    unsigned (*retro_get_region)(void);
    void* (*retro_get_memory_data)(unsigned id);
    size_t (*retro_get_memory_size)(unsigned id);
};

//=============================================================================
// Libretro Callback Types
//=============================================================================

// RETRO_CALLCONV macro (if not defined)
#ifndef RETRO_CALLCONV
#define RETRO_CALLCONV
#endif

// Environment callback function pointer
typedef bool (*retro_environment_t)(unsigned cmd, void* data);

// Video refresh callback
typedef void (*retro_video_refresh_t)(const void* data, unsigned width, unsigned height, size_t pitch);

// Audio sample callback
typedef void (*retro_audio_sample_t)(int16_t left, int16_t right);

// Audio sample batch callback
typedef size_t (*retro_audio_sample_batch_t)(const int16_t* data, size_t frames);

// Input poll callback
typedef void (*retro_input_poll_t)(void);

// Input state callback
typedef int16_t (*retro_input_state_t)(unsigned port, unsigned device, unsigned index, unsigned id);

// Log levels
enum retro_log_level {
    RETRO_LOG_DEBUG = 0,
    RETRO_LOG_INFO = 1,
    RETRO_LOG_WARN = 2,
    RETRO_LOG_ERROR = 3,
    RETRO_LOG_DUMMY = INT_MAX
};

// Log callback function pointer type
typedef void (RETRO_CALLCONV *retro_log_printf_t)(enum retro_log_level level, const char* fmt, ...);

// Log callback structure
struct retro_log_callback {
    retro_log_printf_t log;
};

//=============================================================================
// Core Symbol Names
//=============================================================================

#define SYM_RETRO_INIT "retro_init"
#define SYM_RETRO_DEINIT "retro_deinit"
#define SYM_RETRO_API_VERSION "retro_api_version"
#define SYM_RETRO_GET_SYSTEM_INFO "retro_get_system_info"
#define SYM_RETRO_GET_SYSTEM_AV_INFO "retro_get_system_av_info"
#define SYM_RETRO_SET_CONTROLLER_PORT_DEVICE "retro_set_controller_port_device"
#define SYM_RETRO_RESET "retro_reset"
#define SYM_RETRO_RUN "retro_run"
#define SYM_RETRO_SERIALIZE_SIZE "retro_serialize_size"
#define SYM_RETRO_SERIALIZE "retro_serialize"
#define SYM_RETRO_UNSERIALIZE "retro_unserialize"
#define SYM_RETRO_CHEAT_RESET "retro_cheat_reset"
#define SYM_RETRO_CHEAT_SET "retro_cheat_set"
#define SYM_RETRO_LOAD_GAME "retro_load_game"
#define SYM_RETRO_UNLOAD_GAME "retro_unload_game"
#define SYM_RETRO_GET_REGION "retro_get_region"
#define SYM_RETRO_GET_MEMORY_DATA "retro_get_memory_data"
#define SYM_RETRO_GET_MEMORY_SIZE "retro_get_memory_size"
#define SYM_RETRO_SET_ENVIRONMENT "retro_set_environment"
#define SYM_RETRO_SET_VIDEO_REFRESH "retro_set_video_refresh"
#define SYM_RETRO_SET_AUDIO_SAMPLE "retro_set_audio_sample"
#define SYM_RETRO_SET_AUDIO_SAMPLE_BATCH "retro_set_audio_sample_batch"
#define SYM_RETRO_SET_INPUT_POLL "retro_set_input_poll"
#define SYM_RETRO_SET_INPUT_STATE "retro_set_input_state"

#endif // LIBRETRO_API_H

