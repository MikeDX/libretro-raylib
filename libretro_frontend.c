/*
 * libretro_frontend.c - Libretro Frontend Implementation
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * This implementation uses the libretro API, which is available under the
 * BSD 3-Clause License. See https://www.libretro.com/ for details.
 *
 * The libretro API specification is used here to interface with libretro cores.
 * This file implements the frontend side of the libretro API.
 */

#include "libretro_frontend.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <limits.h>

//=============================================================================
// Libretro API Constants
//=============================================================================

// Libretro API version
#define RETRO_API_VERSION 1
#define RETRO_ENVIRONMENT_EXEC 0
#define RETRO_ENVIRONMENT_SET_ROTATION 1
#define RETRO_ENVIRONMENT_GET_OVERSCAN 2
#define RETRO_ENVIRONMENT_GET_CAN_DUPE 3
#define RETRO_ENVIRONMENT_SET_MESSAGE 4
#define RETRO_ENVIRONMENT_SHUTDOWN 5
#define RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL 6
#define RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY 7
#define RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS 9
#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10  // VICE uses 10, not 8
#define RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK 12  // VICE header defines this as 12
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

#define RETRO_PIXEL_FORMAT_0RGB1555 0
#define RETRO_PIXEL_FORMAT_0RGB1555 0
#define RETRO_PIXEL_FORMAT_XRGB8888 1
#define RETRO_PIXEL_FORMAT_RGB565 2
#define RETRO_PIXEL_FORMAT_RGB555 12  // RGB555 format used by snes9x (Red-Green-Blue, 5-5-5 bits, bit 15 unused)

#define RETRO_DEVICE_NONE 0
#define RETRO_DEVICE_JOYPAD 1
#define RETRO_DEVICE_MOUSE 2
#define RETRO_DEVICE_KEYBOARD 3
#define RETRO_DEVICE_LIGHTGUN 4
#define RETRO_DEVICE_ANALOG 5
#define RETRO_DEVICE_POINTER 6

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

// Libretro keyboard keycodes (from libretro.h)
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

// Forward declaration
struct retro_core_t;

// Libretro API structures (must be defined before use in function pointers)
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

// Libretro core structure
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

// RETRO_CALLCONV macro (if not defined)
#ifndef RETRO_CALLCONV
#define RETRO_CALLCONV
#endif

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

// Forward declaration of log callback
static void RETRO_CALLCONV retro_log_callback(enum retro_log_level level, const char* fmt, ...);

//=============================================================================
// Core Symbol Names
//=============================================================================

// Function symbol names for dynamic loading from core libraries
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

//=============================================================================
// Callback Function Declarations
//=============================================================================

/**
 * Environment callback - handles core requests for system information
 */
static bool retro_environment_callback(unsigned cmd, void* data);

/**
 * Video refresh callback - receives rendered frames from the core
 */
static void retro_video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch);

/**
 * Audio sample callback - receives single audio samples (less efficient)
 */
static void retro_audio_sample_callback(int16_t left, int16_t right);

/**
 * Audio sample batch callback - receives batches of audio samples (preferred)
 */
static size_t retro_audio_sample_batch_callback(const int16_t* data, size_t frames);

/**
 * Input poll callback - called before input state queries
 */
static void retro_input_poll_callback(void);

/**
 * Input state callback - queries current input state
 */
static int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id);

/**
 * Flush any remaining samples in the single-sample audio buffer
 */
static void flush_single_sample_buffer(void);

//=============================================================================
// Global State
//=============================================================================

// Global frontend instance for callbacks (set during initialization)

// Video refresh callback implementation
static void retro_video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch);

// Audio sample callback implementation
static void retro_audio_sample_callback(int16_t left, int16_t right);

// Audio sample batch callback implementation
static size_t retro_audio_sample_batch_callback(const int16_t* data, size_t frames);

// Input poll callback implementation
static void retro_input_poll_callback(void);

// Input state callback implementation
static int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id);

// Global frontend instance for callbacks
static libretro_frontend_t* g_frontend = NULL;

bool libretro_frontend_init(libretro_frontend_t* frontend) {
    if (!frontend) return false;
    
    memset(frontend, 0, sizeof(libretro_frontend_t));
    frontend->width = 320;
    frontend->height = 240;
    frontend->aspect_ratio = 4.0f / 3.0f;
    frontend->audio_sample_rate = 44100;
    frontend->fps = 60.0;
    frontend->audio_buffer_size = 4096;
    frontend->audio_buffer = (float*)malloc(frontend->audio_buffer_size * sizeof(float) * 2);
    
    // Initialize audio ring buffer (enough for ~0.25 seconds to balance latency and underruns)
    // Smaller buffer reduces latency while still preventing scratchiness
    frontend->audio_ring_buffer_size = frontend->audio_sample_rate / 4; // ~5512 frames at 22kHz = 0.25 seconds
    frontend->audio_ring_buffer = (float*)calloc(frontend->audio_ring_buffer_size * 2, sizeof(float)); // Stereo
    frontend->audio_ring_read_pos = 0;
    frontend->audio_ring_write_pos = 0;
    frontend->audio_ring_available = 0;
    
    frontend->pixel_format = RETRO_PIXEL_FORMAT_XRGB8888; // Default
    frontend->pixel_format_raw = RETRO_PIXEL_FORMAT_XRGB8888;
    
    // Initialize keyboard state
    memset(frontend->keyboard_state, 0, sizeof(frontend->keyboard_state));
    
    g_frontend = frontend;
    
    return true;
}

/**
 * Load a libretro core from a dynamic library
 * Dynamically loads the core and resolves all required symbols
 */
bool libretro_frontend_load_core(libretro_frontend_t* frontend, const char* core_path) {
    if (!frontend || !core_path) return false;
    
    // Load the core DYLIB
    void* handle = dlopen(core_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load core: %s\n", dlerror());
        return false;
    }
    
    frontend->core_handle = handle;
    
    // Allocate core structure
    frontend->core = (struct retro_core_t*)calloc(1, sizeof(struct retro_core_t));
    if (!frontend->core) {
        dlclose(handle);
        frontend->core_handle = NULL;
        return false;
    }
    
    // Load symbols for callback setters (we'll set them later, matching RetroArch's sequence)
    typedef void (*retro_set_environment_t)(retro_environment_t);
    typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
    typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
    typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
    typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
    typedef void (*retro_set_input_state_t)(retro_input_state_t);
    
    retro_set_environment_t set_env = (retro_set_environment_t)dlsym(handle, SYM_RETRO_SET_ENVIRONMENT);
    retro_set_video_refresh_t set_video = (retro_set_video_refresh_t)dlsym(handle, SYM_RETRO_SET_VIDEO_REFRESH);
    retro_set_audio_sample_t set_audio = (retro_set_audio_sample_t)dlsym(handle, SYM_RETRO_SET_AUDIO_SAMPLE);
    retro_set_audio_sample_batch_t set_audio_batch = (retro_set_audio_sample_batch_t)dlsym(handle, SYM_RETRO_SET_AUDIO_SAMPLE_BATCH);
    retro_set_input_poll_t set_input_poll = (retro_set_input_poll_t)dlsym(handle, SYM_RETRO_SET_INPUT_POLL);
    retro_set_input_state_t set_input_state = (retro_set_input_state_t)dlsym(handle, SYM_RETRO_SET_INPUT_STATE);
    
    if (!set_env || !set_video || !set_audio || !set_audio_batch || !set_input_poll || !set_input_state) {
        fprintf(stderr, "Failed to load required symbols from core\n");
        free(frontend->core);
        frontend->core = NULL;
        dlclose(handle);
        frontend->core_handle = NULL;
        return false;
    }
    
    // Set environment callback early (like RetroArch does at line 4782)
    // IMPORTANT: Set g_frontend BEFORE setting callbacks, as cores may call them immediately
    g_frontend = frontend;
    fprintf(stderr, "Setting environment callback (matching RetroArch sequence)...\n");
    set_env(retro_environment_callback);
    frontend->has_set_environment = true;
    
    // Note: Video/audio/input callbacks will be set AFTER loading the game (matching RetroArch)
    // RetroArch sets them in runloop_event_load_core -> core_init_libretro_cbs (line 4636-4640)
    
    // Load core functions
    frontend->core->retro_init = (void (*)(void))dlsym(handle, SYM_RETRO_INIT);
    frontend->core->retro_deinit = (void (*)(void))dlsym(handle, SYM_RETRO_DEINIT);
    frontend->core->retro_api_version = (unsigned (*)(void))dlsym(handle, SYM_RETRO_API_VERSION);
    frontend->core->retro_get_system_info = (void (*)(struct retro_system_info*))dlsym(handle, SYM_RETRO_GET_SYSTEM_INFO);
    frontend->core->retro_get_system_av_info = (void (*)(struct retro_system_av_info*))dlsym(handle, SYM_RETRO_GET_SYSTEM_AV_INFO);
    frontend->core->retro_set_controller_port_device = (void (*)(unsigned, unsigned))dlsym(handle, SYM_RETRO_SET_CONTROLLER_PORT_DEVICE);
    frontend->core->retro_reset = (void (*)(void))dlsym(handle, SYM_RETRO_RESET);
    frontend->core->retro_run = (void (*)(void))dlsym(handle, SYM_RETRO_RUN);
    frontend->core->retro_serialize_size = (size_t (*)(void))dlsym(handle, SYM_RETRO_SERIALIZE_SIZE);
    frontend->core->retro_serialize = (bool (*)(void*, size_t))dlsym(handle, SYM_RETRO_SERIALIZE);
    frontend->core->retro_unserialize = (bool (*)(const void*, size_t))dlsym(handle, SYM_RETRO_UNSERIALIZE);
    frontend->core->retro_cheat_reset = (void (*)(void))dlsym(handle, SYM_RETRO_CHEAT_RESET);
    frontend->core->retro_cheat_set = (void (*)(unsigned, bool, const char*))dlsym(handle, SYM_RETRO_CHEAT_SET);
    frontend->core->retro_load_game = (bool (*)(const struct retro_game_info*))dlsym(handle, SYM_RETRO_LOAD_GAME);
    frontend->core->retro_unload_game = (void (*)(void))dlsym(handle, SYM_RETRO_UNLOAD_GAME);
    frontend->core->retro_get_region = (unsigned (*)(void))dlsym(handle, SYM_RETRO_GET_REGION);
    frontend->core->retro_get_memory_data = (void* (*)(unsigned))dlsym(handle, SYM_RETRO_GET_MEMORY_DATA);
    frontend->core->retro_get_memory_size = (size_t (*)(unsigned))dlsym(handle, SYM_RETRO_GET_MEMORY_SIZE);
    
    if (!frontend->core->retro_init || !frontend->core->retro_run) {
        fprintf(stderr, "Failed to load core functions\n");
        free(frontend->core);
        frontend->core = NULL;
        dlclose(handle);
        frontend->core_handle = NULL;
        return false;
    }
    
    return true;
}

/**
 * Initialize the loaded core
 * Calls retro_init and sets up controller devices
 */
bool libretro_frontend_init_core(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core) return false;
    
    // Get system info
    struct retro_system_info info;
    if (frontend->core->retro_get_system_info) {
        frontend->core->retro_get_system_info(&info);
        frontend->need_fullpath = info.need_fullpath;
        fprintf(stderr, "Core: %s %s\n", info.library_name, info.library_version);
    }
    
    // Initialize the core
    if (frontend->core->retro_init) {
        frontend->core->retro_init();
    }
    
    // Set controller device
    if (frontend->core->retro_set_controller_port_device) {
        frontend->core->retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    }
    
    // Set default dimensions (will be updated after game load if needed)
    frontend->width = 240;
    frontend->height = 160;
    frontend->aspect_ratio = 3.0f / 2.0f;
    
    frontend->initialized = true;
    return true;
}

/**
 * Update audio/video information from the core
 * Should be called after loading a game to get correct resolution and sample rate
 */
void libretro_frontend_update_av_info(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core) return;
    
    // Get AV info (must be called after game is loaded)
    struct retro_system_av_info av_info;
    if (frontend->core->retro_get_system_av_info) {
        frontend->core->retro_get_system_av_info(&av_info);
        frontend->width = av_info.geometry.base_width;
        frontend->height = av_info.geometry.base_height;
        frontend->aspect_ratio = av_info.geometry.aspect_ratio;
        
        unsigned new_sample_rate = (unsigned)av_info.timing.sample_rate;
        // Handle 0 Hz sample rate (some cores report 0 before they're ready)
        if (new_sample_rate == 0) {
            fprintf(stderr, "Warning: Core reported 0 Hz sample rate, using default 44100 Hz\n");
            new_sample_rate = 44100;
        }
        fprintf(stderr, "Video: %ux%u (aspect: %.2f, fps: %.2f)\n", 
                frontend->width, frontend->height, frontend->aspect_ratio, frontend->fps);
        fprintf(stderr, "Audio: %u Hz\n", new_sample_rate);
        if (new_sample_rate != frontend->audio_sample_rate) {
            // Reallocate ring buffer if sample rate changed
            if (frontend->audio_ring_buffer) {
                free(frontend->audio_ring_buffer);
            }
            frontend->audio_sample_rate = new_sample_rate;
            frontend->audio_ring_buffer_size = frontend->audio_sample_rate / 4; // ~0.25 seconds
            if (frontend->audio_ring_buffer_size == 0) {
                frontend->audio_ring_buffer_size = 11025; // Default to ~0.25 seconds at 44.1kHz
            }
            frontend->audio_ring_buffer = (float*)calloc(frontend->audio_ring_buffer_size * 2, sizeof(float));
            frontend->audio_ring_read_pos = 0;
            frontend->audio_ring_write_pos = 0;
            frontend->audio_ring_available = 0;
        }
        
        frontend->fps = av_info.timing.fps;
        
        // Allocate/reallocate framebuffer
        size_t new_size = frontend->width * frontend->height * 4; // RGBA
        if (new_size != frontend->framebuffer_size || !frontend->framebuffer) {
            if (frontend->framebuffer) {
                free(frontend->framebuffer);
                frontend->framebuffer = NULL;
            }
            frontend->framebuffer_size = new_size;
            frontend->framebuffer = calloc(1, frontend->framebuffer_size); // Zero-initialize
            if (!frontend->framebuffer) {
                fprintf(stderr, "Failed to allocate framebuffer\n");
                frontend->framebuffer_size = 0;
            }
        }
    }
}

// Forward declaration for debug functions

bool libretro_frontend_load_rom(libretro_frontend_t* frontend, const char* rom_path) {
    if (!frontend || !frontend->core) return false;
    
    // Handle no-game mode (rom_path is NULL)
    if (!rom_path) {
        // For no-game mode, call retro_load_game with NULL
        // According to libretro spec, cores that support no-game mode should accept NULL
        
        // RETROARCH SEQUENCE: Load game FIRST, then set callbacks
        bool success = frontend->core->retro_load_game(NULL);
        if (success) {
            // Set callbacks AFTER loading (matching RetroArch's runloop_event_load_core)
            if (!frontend->has_set_video_refresh) {
                fprintf(stderr, "Setting up video/audio/input callbacks after game load (matching RetroArch)...\n");
                typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
                typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
                typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
                typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
                typedef void (*retro_set_input_state_t)(retro_input_state_t);
                
                retro_set_video_refresh_t set_video = (retro_set_video_refresh_t)dlsym(frontend->core_handle, SYM_RETRO_SET_VIDEO_REFRESH);
                retro_set_audio_sample_t set_audio = (retro_set_audio_sample_t)dlsym(frontend->core_handle, SYM_RETRO_SET_AUDIO_SAMPLE);
                retro_set_audio_sample_batch_t set_audio_batch = (retro_set_audio_sample_batch_t)dlsym(frontend->core_handle, SYM_RETRO_SET_AUDIO_SAMPLE_BATCH);
                retro_set_input_poll_t set_input_poll = (retro_set_input_poll_t)dlsym(frontend->core_handle, SYM_RETRO_SET_INPUT_POLL);
                retro_set_input_state_t set_input_state = (retro_set_input_state_t)dlsym(frontend->core_handle, SYM_RETRO_SET_INPUT_STATE);
                
                if (set_video && set_audio && set_audio_batch && set_input_poll && set_input_state) {
                    set_video(retro_video_refresh_callback);
                    set_audio(retro_audio_sample_callback);
                    set_audio_batch(retro_audio_sample_batch_callback);
                    set_input_poll(retro_input_poll_callback);
                    set_input_state(retro_input_state_callback);
                    
                    frontend->has_set_video_refresh = true;
                    frontend->has_set_audio_sample = true;
                    frontend->has_set_audio_sample_batch = true;
                    frontend->has_set_input_poll = true;
                    frontend->has_set_input_state = true;
                } else {
                    fprintf(stderr, "Warning: Failed to set up video/audio/input callbacks\n");
                }
            }
            
            // Update AV info after setting callbacks (matching RetroArch)
            libretro_frontend_update_av_info(frontend);
            frontend->rom_path = NULL;
            frontend->rom_data = NULL;
            frontend->rom_data_size = 0;
        }
        return success;
    }
    
    if (!frontend->core->retro_load_game) {
        fprintf(stderr, "Core does not support loading games\n");
        return false;
    }
    
    // Get absolute path (needed for need_fullpath cores)
    char* abs_path = realpath(rom_path, NULL);
    if (!abs_path) {
        fprintf(stderr, "Failed to get absolute path for ROM: %s\n", rom_path);
        return false;
    }
    
    struct retro_game_info game_info = {0};
    void* rom_data = NULL;
    
    if (frontend->need_fullpath) {
        // Core wants path only, not data in memory
        game_info.path = abs_path;
        game_info.data = NULL;
        game_info.size = 0;
        game_info.meta = NULL;
    } else {
        // Core wants data in memory
        FILE* file = fopen(abs_path, "rb");
        if (!file) {
            fprintf(stderr, "Failed to open ROM file: %s\n", abs_path);
            free(abs_path);
            return false;
        }
        
        // Get file size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        if (file_size <= 0) {
            fprintf(stderr, "Invalid ROM file size\n");
            fclose(file);
            free(abs_path);
            return false;
        }
        
        // Allocate buffer and read ROM
        rom_data = malloc(file_size);
        if (!rom_data) {
            fprintf(stderr, "Failed to allocate memory for ROM\n");
            fclose(file);
            free(abs_path);
            return false;
        }
        
        size_t bytes_read = fread(rom_data, 1, file_size, file);
        fclose(file);
        
        if (bytes_read != (size_t)file_size) {
            fprintf(stderr, "Failed to read ROM file completely\n");
            free(rom_data);
            free(abs_path);
            return false;
        }
        
        game_info.path = abs_path;
        game_info.data = rom_data;
        game_info.size = (size_t)file_size;
        game_info.meta = NULL;
        
    }
    
    // RETROARCH SEQUENCE: Load game FIRST, then set callbacks
    // Load the game
    bool success = frontend->core->retro_load_game(&game_info);
    
    if (!success) {
        fprintf(stderr, "Failed to load ROM - core returned false\n");
        if (rom_data) {
            free(rom_data);
        }
        free(abs_path);
        return false;
    }
    
    // Store ROM data and path for cleanup
    frontend->rom_data = rom_data;
    frontend->rom_data_size = frontend->need_fullpath ? 0 : game_info.size;
    frontend->rom_path = abs_path;
    
    // Set callbacks AFTER loading game (matching RetroArch's runloop_event_load_core)
    if (!frontend->has_set_video_refresh) {
        fprintf(stderr, "Setting up video/audio/input callbacks after game load (matching RetroArch)...\n");
        typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
        typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
        typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
        typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
        typedef void (*retro_set_input_state_t)(retro_input_state_t);
        
        retro_set_video_refresh_t set_video = (retro_set_video_refresh_t)dlsym(frontend->core_handle, SYM_RETRO_SET_VIDEO_REFRESH);
        retro_set_audio_sample_t set_audio = (retro_set_audio_sample_t)dlsym(frontend->core_handle, SYM_RETRO_SET_AUDIO_SAMPLE);
        retro_set_audio_sample_batch_t set_audio_batch = (retro_set_audio_sample_batch_t)dlsym(frontend->core_handle, SYM_RETRO_SET_AUDIO_SAMPLE_BATCH);
        retro_set_input_poll_t set_input_poll = (retro_set_input_poll_t)dlsym(frontend->core_handle, SYM_RETRO_SET_INPUT_POLL);
        retro_set_input_state_t set_input_state = (retro_set_input_state_t)dlsym(frontend->core_handle, SYM_RETRO_SET_INPUT_STATE);
        
        if (set_video && set_audio && set_audio_batch && set_input_poll && set_input_state) {
            set_video(retro_video_refresh_callback);
            set_audio(retro_audio_sample_callback);
            set_audio_batch(retro_audio_sample_batch_callback);
            set_input_poll(retro_input_poll_callback);
            set_input_state(retro_input_state_callback);
            
            frontend->has_set_video_refresh = true;
            frontend->has_set_audio_sample = true;
            frontend->has_set_audio_sample_batch = true;
            frontend->has_set_input_poll = true;
            frontend->has_set_input_state = true;
        } else {
            fprintf(stderr, "Warning: Failed to set up video/audio/input callbacks\n");
        }
    }
    
    // Update AV info after setting callbacks (matching RetroArch)
    libretro_frontend_update_av_info(frontend);
    
    // DO NOT call retro_reset here - RetroArch doesn't do this
    // VICE handles its own initialization in retro_load_game
    // Calling retro_reset here would reset VICE's state AFTER it's initialized,
    // clearing screen memory and other state that VICE just set up
    
    return true;
}


// Debug: Check all available memory regions

static int frame_count = 0;

/**
 * Run one frame of the core
 * Calls retro_run which generates one frame of video/audio
 */
void libretro_frontend_run_frame(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core || !frontend->initialized) return;
    
    frame_count++;
    
    // RetroArch polls input before retro_run (for early polling cores)
    // VICE might need this to initialize properly
    if (frontend->has_set_input_poll && g_frontend) {
        // Call input poll callback if set (some cores need this)
        retro_input_poll_callback();
    }
    
    if (frontend->core->retro_run) {
        frontend->core->retro_run();
    }
    
    // Flush any accumulated single-sample audio after each frame
    // This ensures cores using retro_audio_sample_callback don't lose samples
    flush_single_sample_buffer();
}

/**
 * Reset the core
 * Calls retro_reset to reset the core state
 */
void libretro_frontend_reset(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core || !frontend->initialized) return;
    
    if (frontend->core->retro_reset) {
        frontend->core->retro_reset();
    }
}

/**
 * Get the current video framebuffer
 * Returns pointer to RGBA8888 formatted framebuffer data
 */
void* libretro_frontend_get_framebuffer(libretro_frontend_t* frontend) {
    if (!frontend) return NULL;
    return frontend->framebuffer;
}

/**
 * Get current video dimensions
 */
void libretro_frontend_get_video_size(libretro_frontend_t* frontend, unsigned* width, unsigned* height) {
    if (!frontend || !width || !height) return;
    *width = frontend->width;
    *height = frontend->height;
}

/**
 * Set input state for a joypad button
 */
void libretro_frontend_set_input(libretro_frontend_t* frontend, unsigned port, unsigned button, bool pressed) {
    if (!frontend || port >= 16 || button >= 16) return;
    frontend->input_state[port][button] = pressed;
}

/**
 * Set keyboard key state
 */
void libretro_frontend_set_keyboard_key(libretro_frontend_t* frontend, unsigned keycode, bool pressed) {
    if (!frontend || keycode >= RETROK_LAST) return;
    frontend->keyboard_state[keycode] = pressed;
}

/**
 * Deinitialize and cleanup the frontend
 * Unloads the core and frees all allocated resources
 */
void libretro_frontend_deinit(libretro_frontend_t* frontend) {
    if (!frontend) return;
    
    // Unload game if loaded (only if a game was actually loaded)
    // Check if rom_path is set to determine if a game was loaded
    if (frontend->core && frontend->core->retro_unload_game && frontend->rom_path) {
        frontend->core->retro_unload_game();
    }
    
    // Free ROM data and path (after retro_unload_game)
    if (frontend->rom_data) {
        free(frontend->rom_data);
        frontend->rom_data = NULL;
    }
    if (frontend->rom_path) {
        free(frontend->rom_path);
        frontend->rom_path = NULL;
    }
    
    // Deinitialize core
    // Note: Some cores (like VICE) may have issues during deinit if no game was loaded
    // We still need to call it to clean up resources, but it might crash
    // The alternative is to leak resources, which is worse
    if (frontend->core && frontend->core->retro_deinit && frontend->initialized) {
        // Only call deinit if core was initialized
        // VICE cores seem to crash during deinit if no game was loaded,
        // but we need to try anyway to avoid resource leaks
        frontend->core->retro_deinit();
    }
    
    // Close core library
    if (frontend->core_handle) {
        dlclose(frontend->core_handle);
        frontend->core_handle = NULL;
    }
    
    // Free allocated memory
    if (frontend->core) {
        free(frontend->core);
        frontend->core = NULL;
    }
    
    if (frontend->framebuffer) {
        free(frontend->framebuffer);
        frontend->framebuffer = NULL;
    }
    
    if (frontend->audio_buffer) {
        free(frontend->audio_buffer);
        frontend->audio_buffer = NULL;
    }
    
    if (frontend->audio_ring_buffer) {
        free(frontend->audio_ring_buffer);
        frontend->audio_ring_buffer = NULL;
    }
    
    memset(frontend, 0, sizeof(libretro_frontend_t));
}

/**
 * Get audio samples from the ring buffer
 * Used to feed raylib's audio stream
 */
size_t libretro_frontend_get_audio_samples(libretro_frontend_t* frontend, float* buffer, size_t max_frames) {
    if (!frontend || !buffer || max_frames == 0) return 0;
    
    size_t frames_to_read = (max_frames < frontend->audio_ring_available) ? max_frames : frontend->audio_ring_available;
    
    if (frames_to_read == 0) return 0;
    
    // Read from ring buffer - optimize for contiguous reads
    if (frames_to_read == frontend->audio_ring_available &&
        (frontend->audio_ring_read_pos + frames_to_read) <= frontend->audio_ring_buffer_size) {
        // Fast path: contiguous read, no wrap-around
        float* src = frontend->audio_ring_buffer + frontend->audio_ring_read_pos * 2;
        memcpy(buffer, src, frames_to_read * 2 * sizeof(float));
    } else {
        // Slow path: handle wrap-around
        for (size_t i = 0; i < frames_to_read; i++) {
            size_t read_idx = (frontend->audio_ring_read_pos + i) % frontend->audio_ring_buffer_size;
            float* src = frontend->audio_ring_buffer + read_idx * 2;
            
            buffer[i * 2] = src[0];     // Left
            buffer[i * 2 + 1] = src[1]; // Right
        }
    }
    
    frontend->audio_ring_read_pos = (frontend->audio_ring_read_pos + frames_to_read) % frontend->audio_ring_buffer_size;
    frontend->audio_ring_available -= frames_to_read;
    
    return frames_to_read;
}

//=============================================================================
// Libretro Callback Implementations
//=============================================================================

/**
 * Environment callback implementation
 * Handles requests from the core for system information and configuration
 */
static bool retro_environment_callback(unsigned cmd, void* data) {
    
    if (!g_frontend) {
        if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
            fprintf(stderr, "ERROR: SET_PIXEL_FORMAT called but g_frontend is NULL! cmd=%u\n", cmd);
        }
        // Log all callbacks when g_frontend is NULL to see what's happening
        fprintf(stderr, "WARNING: Environment callback called with cmd=%u but g_frontend is NULL\n", cmd);
        return false;
    }
    
    // Debug output for important callbacks
    // Map command numbers to names for debugging
    const char* cmd_name = "UNKNOWN";
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: cmd_name = "SET_PIXEL_FORMAT"; break;
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: cmd_name = "GET_SYSTEM_DIRECTORY"; break;
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: cmd_name = "SET_SUPPORT_NO_GAME"; break;
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: cmd_name = "GET_LOG_INTERFACE"; break;
        default: break;
    }
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT || cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY) {
        fprintf(stderr, "Environment callback: cmd=%u (%s)\n", cmd, cmd_name);
    }
    
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            unsigned* format = (unsigned*)data;
            if (!g_frontend) return false;  // Must return false if frontend not ready
            if (g_frontend) {
                switch (*format) {
                    case RETRO_PIXEL_FORMAT_0RGB1555: 
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_0RGB1555;
                        g_frontend->pixel_format_raw = *format;
                        fprintf(stderr, "Pixel format: 0RGB1555 (format 0) - R=bits 10-14, G=bits 5-9, B=bits 0-4\n");
                        break;
                    case RETRO_PIXEL_FORMAT_XRGB8888: 
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;
                        g_frontend->pixel_format_raw = *format;
                        fprintf(stderr, "Pixel format: XRGB8888 (format 1) - 32-bit\n");
                        break;
                    case RETRO_PIXEL_FORMAT_RGB565: 
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
                        g_frontend->pixel_format_raw = *format;
                        fprintf(stderr, "Pixel format: RGB565 (format 2) - R=bits 11-15, G=bits 5-10, B=bits 0-4\n");
                        break;
                    case RETRO_PIXEL_FORMAT_RGB555:
                        // Format 12: Treat as RGB565 (5-6-5 bits) used by snes9x
                        // snes9x uses RGB565 format but reports it as format 12
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
                        g_frontend->pixel_format_raw = 12; // Store original format
                        fprintf(stderr, "Pixel format: RGB565 (format 12 from snes9x) - R=bits 11-15, G=bits 5-10, B=bits 0-4\n");
                        break;
                    default:
                        // Unknown format - default to RGB565
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
                        g_frontend->pixel_format_raw = *format;
                        break;
                }
            }
            // Return true to indicate we support this format
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
            if (!data) return false;
            const char** dir = (const char**)data;
            // Use "." directory - VICE will append "/vice" to create "./vice"
            // ROM files should be in "./vice/PLUS4/" or "./system/vice/PLUS4/"
            static const char* system_dir = ".";
            *dir = system_dir;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
            if (!data) return false;
            const char** dir = (const char**)data;
            *dir = "./";
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
            if (!data) return false;
            bool* support = (bool*)data;
            *support = true;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY: {
            if (!data) return false;
            const char** dir = (const char**)data;
            *dir = "./";
            return true;
        }
        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_VARIABLES: {
            return true;
        }
        case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
            if (!data) return false;
            unsigned* enable = (unsigned*)data;
            *enable = 3; // Enable both audio and video (bit 0=video, bit 1=audio)
            return true;
        }
        case RETRO_ENVIRONMENT_SET_AUDIO_VIDEO_ENABLE: {
            // Core wants to enable/disable audio/video
            // Acknowledge the request (we always enable both)
            return true;
        }
        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
            // Some cores use this to set up custom audio callbacks
            // We don't support custom audio callbacks, but return true to indicate we handle it
            return true;
        }
        case RETRO_ENVIRONMENT_SET_FASTFORWARDING: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
            return true;
        }
        case 32: { // RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO (value 32)
            // Core wants to update AV info - this is important for VICE
            // VICE calls this after retro_load_game to set up video properly
            if (!data) return false;
            const struct retro_system_av_info* av_info = (const struct retro_system_av_info*)data;
            if (g_frontend && av_info) {
                // Update our internal AV info
                g_frontend->width = av_info->geometry.base_width;
                g_frontend->height = av_info->geometry.base_height;
                g_frontend->aspect_ratio = av_info->geometry.aspect_ratio;
                g_frontend->fps = av_info->timing.fps;
                unsigned new_sample_rate = (unsigned)av_info->timing.sample_rate;
                if (new_sample_rate > 0 && new_sample_rate != g_frontend->audio_sample_rate) {
                    g_frontend->audio_sample_rate = new_sample_rate;
                }
                fprintf(stderr, "SET_SYSTEM_AV_INFO: %ux%u (aspect: %.2f, fps: %.2f, sample rate: %.2f Hz)\n",
                        g_frontend->width, g_frontend->height, g_frontend->aspect_ratio, 
                        g_frontend->fps, (double)g_frontend->audio_sample_rate);
            }
            return true;
        }
        case 37: { // RETRO_ENVIRONMENT_SET_GEOMETRY (value 37)
            // Core wants to update geometry (similar to SET_SYSTEM_AV_INFO but without reinit)
            if (!data) return false;
            const struct retro_game_geometry* geom = (const struct retro_game_geometry*)data;
            if (g_frontend && geom) {
                g_frontend->width = geom->base_width;
                g_frontend->height = geom->base_height;
                g_frontend->aspect_ratio = geom->aspect_ratio;
                fprintf(stderr, "SET_GEOMETRY: %ux%u (aspect: %.2f)\n",
                        g_frontend->width, g_frontend->height, g_frontend->aspect_ratio);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
            // Provide log interface so cores can log safely
            // This prevents cores from trying to use invalid FILE* pointers
            if (!data) return false;
            struct retro_log_callback* log_cb = (struct retro_log_callback*)data;
            // Provide a log callback that writes to stderr
            log_cb->log = retro_log_callback;
            return true;
        }
        case 33: // RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
        case 34: // RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION
        case 35: // RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK
        case 36: // RETRO_ENVIRONMENT_GET_INPUT_BITMASKS
        case 38: { // RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS
            // VICE-specific callbacks - just return true without accessing data
            // This prevents crashes with cores that don't use these callbacks
            return true;
        }
        default:
            // For unknown/extended commands, return false (not supported)
            // Some cores may check return values, but returning true for unknown commands
            // can cause issues. Better to return false and let the core handle it.
            return false;
    }
}

static void retro_video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!g_frontend || !data) {
        fprintf(stderr, "ERROR: video_callback called with NULL frontend or data!\n");
        return;
    }
    
    // Skip processing if dimensions are invalid (VICE may call this during initialization)
    if (width == 0 || height == 0) {
        fprintf(stderr, "WARNING: video_callback called with zero dimensions: %ux%u\n", width, height);
        return;
    }
    
    static int call_count = 0;
    static bool warned_black = false;
    call_count++;
    
    // Debug: Check if data is all zeros (black screen) - only warn once
    if (!warned_black && call_count >= 10) {
        const uint16_t* pixels = (const uint16_t*)data;
        int non_zero_count = 0;
        int sample_size = (width * height < 100) ? width * height : 100;
        for (int i = 0; i < sample_size; i++) {
            if (pixels[i] != 0) {
                non_zero_count++;
                break;
            }
        }
        if (non_zero_count == 0) {
            fprintf(stderr, "WARNING: Video callback receiving all-zero (black) data after %d frames\n", call_count);
            warned_black = true;
        }
    }
    
    static bool format_auto_detected = false;
    
    // Auto-detect RGB565 format if SET_PIXEL_FORMAT was never called
    // VICE uses RGB565 (pitch = width * 2) but may not call SET_PIXEL_FORMAT
    if (!format_auto_detected && g_frontend->pixel_format == RETRO_PIXEL_FORMAT_XRGB8888) {
        size_t expected_pitch_32bit = width * 4;
        size_t expected_pitch_16bit = width * 2;
        if (pitch == expected_pitch_16bit || (pitch != expected_pitch_32bit && pitch % 2 == 0)) {
            fprintf(stderr, "Auto-detecting RGB565 format: pitch=%zu matches 16-bit (width*2=%zu), not 32-bit (width*4=%zu)\n",
                    pitch, expected_pitch_16bit, expected_pitch_32bit);
            g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
            format_auto_detected = true;
        }
    }
    
    frame_count++;
    
    // Update dimensions if changed
    if (width != g_frontend->width || height != g_frontend->height || !g_frontend->framebuffer) {
        g_frontend->width = width;
        g_frontend->height = height;
        g_frontend->framebuffer_size = width * height * 4;
        if (g_frontend->framebuffer) {
            free(g_frontend->framebuffer);
        }
        g_frontend->framebuffer = calloc(1, g_frontend->framebuffer_size); // Zero-initialize
        if (!g_frontend->framebuffer) {
            fprintf(stderr, "Failed to allocate framebuffer in callback\n");
            return;
        }
    }
    
    if (!g_frontend->framebuffer) return;
    
    // Safety check: ensure we don't write beyond buffer
    if (width * height * 4 > g_frontend->framebuffer_size) {
        fprintf(stderr, "Framebuffer size mismatch: %ux%u needs %zu bytes, have %zu\n",
                width, height, (size_t)(width * height * 4), g_frontend->framebuffer_size);
        return;
    }
    
    uint32_t* dst = (uint32_t*)g_frontend->framebuffer;
    
    // Handle different pixel formats
    switch (g_frontend->pixel_format) {
        case RETRO_PIXEL_FORMAT_XRGB8888: {
            // XRGB8888: 32-bit, pitch is in bytes
            // Some cores (like SNES, VICE) report XRGB8888 but actually send RGB565
            // Detect this by checking if pitch matches 16-bit format better than 32-bit
            
            size_t expected_pitch_32bit = width * 4;
            size_t expected_pitch_16bit = width * 2;
            size_t pitch_diff_32bit = (pitch > expected_pitch_32bit) ? (pitch - expected_pitch_32bit) : (expected_pitch_32bit - pitch);
            size_t pitch_diff_16bit = (pitch > expected_pitch_16bit) ? (pitch - expected_pitch_16bit) : (expected_pitch_16bit - pitch);
            
            // If pitch is much closer to 16-bit than 32-bit, treat as RGB565
            // Also check if pitch is exactly width*2 or close to it
            bool likely_rgb565 = (pitch_diff_16bit < pitch_diff_32bit && pitch_diff_16bit <= 32) || 
                                 (pitch == expected_pitch_16bit) ||
                                 (pitch % 2 == 0 && pitch / 2 >= width - 4 && pitch / 2 <= width + 4);
            
            
            
            if (likely_rgb565) {
                // Treat as RGB565 even though format says XRGB8888
                // VICE uses RGB565 format
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint32_t* dst_line = dst + y * width;
                    
                    for (unsigned x = 0; x < width; x++) {
                        const uint16_t* src_pixel = (const uint16_t*)(src_line + x * 2);
                        uint16_t pixel = *src_pixel;
                        
                        // Extract RGB565 components (little-endian byte order)
                        // RGB565 format: RRRRR GGGGGG BBBBB (bits 15-11=R, 10-5=G, 4-0=B)
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        
                        // Convert to RGBA8888 for raylib: [R, G, B, A] in memory (little-endian)
                        // Raylib expects RGBA8888 format
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            } else {
                // Normal XRGB8888 handling
                // On little-endian: bytes are [B, G, R, X] when read sequentially
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint32_t* dst_line = dst + y * width;
                    
                    for (unsigned x = 0; x < width; x++) {
                        const uint8_t* src_pixel = src_line + x * 4;
                        
                        // XRGB8888: bytes [B, G, R, X] -> RGBA8888: [R, G, B, A]
                        // Note: libretro XRGB8888 might be [R, G, B, X] instead
                        uint8_t byte0 = src_pixel[0];
                        uint8_t byte1 = src_pixel[1];
                        uint8_t byte2 = src_pixel[2];
                        
                        // Try swapping: if it's [B, G, R], swap to [R, G, B]
                        // If it's already [R, G, B], this will swap it back - test both
                        uint8_t r = byte2;  // Assume [B, G, R] format
                        uint8_t g = byte1;
                        uint8_t b = byte0;
                        
                        // Convert to RGBA8888 - swap red and blue channels
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            }
            break;
        }
        case RETRO_PIXEL_FORMAT_RGB565: {
            // RGB565: 16-bit, pitch is in bytes
            // VICE uses RGB565 format
            size_t expected_pitch = width * 2;
            
            
            for (unsigned y = 0; y < height; y++) {
                const uint8_t* src_bytes = (const uint8_t*)data + y * pitch;
                uint32_t* dst_line = dst + y * width;
                
                if (pitch == expected_pitch) {
                    // Fast path: pitch matches
                    const uint16_t* src_line = (const uint16_t*)src_bytes;
                    for (unsigned x = 0; x < width; x++) {
                        uint16_t pixel = src_line[x];
                        
                        // Extract RGB565 components (5-6-5 bits)
                        // RGB565 format: RRRRR GGGGGG BBBBB (bits 15-11=R, 10-5=G, 4-0=B)
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        // Convert to RGBA8888 for raylib: bytes [R, G, B, A] in memory (little-endian)
                        // Write as: (A << 24) | (B << 16) | (G << 8) | R
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                } else {
                    // Slow path: handle variable pitch
                    for (unsigned x = 0; x < width; x++) {
                        const uint16_t* pixel_ptr = (const uint16_t*)(src_bytes + x * 2);
                        uint16_t pixel = *pixel_ptr;
                        
                        // Extract RGB565 components (5-6-5 bits)
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        // Convert to RGBA8888 for raylib: bytes [R, G, B, A] in memory (little-endian)
                        // Write as: (A << 24) | (B << 16) | (G << 8) | R
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            }
            break;
        }
        case RETRO_PIXEL_FORMAT_0RGB1555: {
            // 0RGB1555: 16-bit, pitch is in bytes
            // Format: 0RGB1555 (5-5-5 bits, bit 15 unused)
            for (unsigned y = 0; y < height; y++) {
                const uint16_t* src_line = (const uint16_t*)((const char*)data + y * pitch);
                uint32_t* dst_line = dst + y * width;
                for (unsigned x = 0; x < width; x++) {
                    uint16_t pixel = src_line[x];
                    // Extract 0RGB1555 components: R=bits 10-14, G=bits 5-9, B=bits 0-4
                    uint8_t r = ((pixel >> 10) & 0x1F) << 3;
                    uint8_t g = ((pixel >> 5) & 0x1F) << 3;
                    uint8_t b = (pixel & 0x1F) << 3;
                    // Convert to RGBA8888 for raylib: bytes [R, G, B, A] in memory
                    dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Unsupported pixel format: %u\n", g_frontend->pixel_format);
            break;
    }
}

// Single-sample audio accumulator for cores that use retro_audio_sample_callback
#define SINGLE_SAMPLE_BUFFER_SIZE 512
#define SINGLE_SAMPLE_FLUSH_THRESHOLD 1  // Flush immediately - xrick may need immediate processing
static int16_t single_sample_buffer[SINGLE_SAMPLE_BUFFER_SIZE * 2]; // Stereo
static size_t single_sample_count = 0;

static void retro_audio_sample_callback(int16_t left, int16_t right) {
    if (!g_frontend) return;
    
    // Accumulate samples in buffer
    if (single_sample_count < SINGLE_SAMPLE_BUFFER_SIZE) {
        single_sample_buffer[single_sample_count * 2] = left;
        single_sample_buffer[single_sample_count * 2 + 1] = right;
        single_sample_count++;
    } else {
        // Buffer is full - flush it before adding more samples
        retro_audio_sample_batch_callback(single_sample_buffer, single_sample_count);
        single_sample_count = 0;
        // Add the new sample
        single_sample_buffer[0] = left;
        single_sample_buffer[1] = right;
        single_sample_count = 1;
    }
    
    // Flush when buffer reaches threshold to reduce latency
    // Use a lower threshold for better responsiveness
    if (single_sample_count >= SINGLE_SAMPLE_FLUSH_THRESHOLD) {
        size_t processed = retro_audio_sample_batch_callback(single_sample_buffer, single_sample_count);
        if (processed != single_sample_count) {
            static int drop_count = 0;
            if (drop_count++ < 3) {
                fprintf(stderr, "WARNING: Only processed %zu/%zu samples (buffer full?)\n", 
                        processed, single_sample_count);
            }
        }
        single_sample_count = 0;
    }
}

// Flush any remaining samples in the single-sample buffer
// Should be called periodically (e.g., after each frame)
static void flush_single_sample_buffer(void) {
    if (single_sample_count > 0 && g_frontend) {
        retro_audio_sample_batch_callback(single_sample_buffer, single_sample_count);
        single_sample_count = 0;
    }
}

/**
 * Audio sample batch callback (preferred method)
 * Receives batches of audio samples and writes them to the ring buffer
 * @param data Pointer to interleaved stereo int16_t samples
 * @param frames Number of frames (samples per channel)
 * @return Number of frames actually processed (may be less if buffer is full)
 */
static size_t retro_audio_sample_batch_callback(const int16_t* data, size_t frames) {
    if (!g_frontend || !data || frames == 0) return 0;
    
    // Safety check: ensure ring buffer is initialized
    if (!g_frontend->audio_ring_buffer || g_frontend->audio_ring_buffer_size == 0) {
        static int error_count = 0;
        if (error_count++ < 3) {
            fprintf(stderr, "ERROR: Audio ring buffer not initialized!\n");
        }
        return 0;
    }
    
    // Audio batch callback - samples are being processed correctly
    
    // IMPORTANT: According to libretro docs, we must return the number of frames we processed
    // If buffer is full, we should still try to write what we can, but return the actual count
    
    // Convert int16_t samples to float and add to ring buffer
    size_t available_space = g_frontend->audio_ring_buffer_size - g_frontend->audio_ring_available;
    size_t frames_to_write = (frames < available_space) ? frames : available_space;
    
    if (frames_to_write == 0) {
        // Buffer full - this is bad, means we're not reading fast enough
        // Return 0 to indicate we couldn't process any frames
        // The core will handle this (might slow down or skip audio)
        return 0;
    }
    
    // Write to ring buffer - use memcpy for better performance on contiguous writes
    if (frames_to_write == frames && 
        (g_frontend->audio_ring_write_pos + frames_to_write) <= g_frontend->audio_ring_buffer_size) {
        // Fast path: contiguous write, no wrap-around
        float* dst = g_frontend->audio_ring_buffer + g_frontend->audio_ring_write_pos * 2;
        for (size_t i = 0; i < frames_to_write; i++) {
            dst[i * 2] = (float)data[i * 2] / 32768.0f;
            dst[i * 2 + 1] = (float)data[i * 2 + 1] / 32768.0f;
        }
    } else {
        // Slow path: handle wrap-around
        for (size_t i = 0; i < frames_to_write; i++) {
            size_t write_idx = (g_frontend->audio_ring_write_pos + i) % g_frontend->audio_ring_buffer_size;
            float* buffer = g_frontend->audio_ring_buffer + write_idx * 2;
            
            // Convert int16_t (-32768 to 32767) to float (-1.0 to 1.0)
            buffer[0] = (float)data[i * 2] / 32768.0f;     // Left
            buffer[1] = (float)data[i * 2 + 1] / 32768.0f;   // Right
        }
    }
    
    g_frontend->audio_ring_write_pos = (g_frontend->audio_ring_write_pos + frames_to_write) % g_frontend->audio_ring_buffer_size;
    g_frontend->audio_ring_available += frames_to_write;
    
    // Return the number of frames we actually processed
    return frames_to_write;
}

// Log callback implementation
static void RETRO_CALLCONV retro_log_callback(enum retro_log_level level, const char* fmt, ...) {
    (void)level; // We'll log everything to stderr regardless of level
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/**
 * Input poll callback
 * Called before input state queries - currently unused as raylib handles polling
 */
static void retro_input_poll_callback(void) {
    // Input polling is handled by raylib in main.c
    // This is called before retro_input_state
}

/**
 * Input state callback
 * Queries the current state of an input button/axis
 * @param port Controller port (0-15)
 * @param device Device type (RETRO_DEVICE_*)
 * @param index Index (unused for joypad/keyboard)
 * @param id Button/axis ID or keyboard keycode
 * @return Input state (1 for pressed, 0 for released)
 */
static int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id) {
    (void)index;
    if (!g_frontend) return 0;
    
    if (device == RETRO_DEVICE_JOYPAD && port < 16 && id < 16) {
        return g_frontend->input_state[port][id] ? 1 : 0;
    }
    
    if (device == RETRO_DEVICE_KEYBOARD && port < 16 && id < RETROK_LAST) {
        return g_frontend->keyboard_state[id] ? 1 : 0;
    }
    
    return 0;
}

