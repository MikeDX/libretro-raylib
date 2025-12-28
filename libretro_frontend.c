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
#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 8
#define RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS 9
#define RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK 10
#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE 11
#define RETRO_ENVIRONMENT_SET_HW_RENDER 12
#define RETRO_ENVIRONMENT_GET_VARIABLE 13
#define RETRO_ENVIRONMENT_SET_VARIABLES 14
#define RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE 15
#define RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME 16
#define RETRO_ENVIRONMENT_GET_LIBRETRO_PATH 17
#define RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK 18
#define RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK 19
#define RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE 20
#define RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES 21
#define RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE 22
#define RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE 23
#define RETRO_ENVIRONMENT_GET_LOG_INTERFACE 24
#define RETRO_ENVIRONMENT_GET_PERF_INTERFACE 25
#define RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE 26
#define RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY 27
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
#define RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY 47
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
#define RETRO_PIXEL_FORMAT_XRGB8888 1
#define RETRO_PIXEL_FORMAT_RGB565 2

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
    frontend->audio_buffer_size = 4096;
    frontend->audio_buffer = (float*)malloc(frontend->audio_buffer_size * sizeof(float) * 2);
    
    // Initialize audio ring buffer (enough for ~0.5 seconds to prevent underruns)
    // Larger buffer helps prevent scratchiness from buffer underruns
    frontend->audio_ring_buffer_size = frontend->audio_sample_rate / 2; // ~22050 frames at 44.1kHz = 0.5 seconds
    frontend->audio_ring_buffer = (float*)calloc(frontend->audio_ring_buffer_size * 2, sizeof(float)); // Stereo
    frontend->audio_ring_read_pos = 0;
    frontend->audio_ring_write_pos = 0;
    frontend->audio_ring_available = 0;
    
    frontend->pixel_format = RETRO_PIXEL_FORMAT_XRGB8888; // Default
    
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
    
    // Load symbols
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
    
    // Set callbacks
    set_env(retro_environment_callback);
    set_video(retro_video_refresh_callback);
    set_audio(retro_audio_sample_callback);
    set_audio_batch(retro_audio_sample_batch_callback);
    set_input_poll(retro_input_poll_callback);
    set_input_state(retro_input_state_callback);
    
    frontend->has_set_environment = true;
    frontend->has_set_video_refresh = true;
    frontend->has_set_audio_sample = true;
    frontend->has_set_audio_sample_batch = true;
    frontend->has_set_input_poll = true;
    frontend->has_set_input_state = true;
    
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
        printf("Core: %s %s\n", info.library_name, info.library_version);
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
        if (new_sample_rate != frontend->audio_sample_rate) {
            // Reallocate ring buffer if sample rate changed
            if (frontend->audio_ring_buffer) {
                free(frontend->audio_ring_buffer);
            }
            frontend->audio_sample_rate = new_sample_rate;
            frontend->audio_ring_buffer_size = frontend->audio_sample_rate / 2; // ~0.5 seconds
            frontend->audio_ring_buffer = (float*)calloc(frontend->audio_ring_buffer_size * 2, sizeof(float));
            frontend->audio_ring_read_pos = 0;
            frontend->audio_ring_write_pos = 0;
            frontend->audio_ring_available = 0;
        }
        
        printf("Video: %ux%u (aspect: %.2f)\n", frontend->width, frontend->height, frontend->aspect_ratio);
        printf("Audio: %.0f Hz\n", av_info.timing.sample_rate);
        
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

bool libretro_frontend_load_rom(libretro_frontend_t* frontend, const char* rom_path) {
    if (!frontend || !frontend->core || !rom_path) return false;
    
    if (!frontend->core->retro_load_game) {
        fprintf(stderr, "Core does not support loading games\n");
        return false;
    }
    
    // Read ROM file
    FILE* file = fopen(rom_path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open ROM file: %s\n", rom_path);
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fprintf(stderr, "Invalid ROM file size\n");
        fclose(file);
        return false;
    }
    
    // Allocate buffer and read ROM
    void* rom_data = malloc(file_size);
    if (!rom_data) {
        fprintf(stderr, "Failed to allocate memory for ROM\n");
        fclose(file);
        return false;
    }
    
    size_t bytes_read = fread(rom_data, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Failed to read ROM file completely\n");
        free(rom_data);
        return false;
    }
    
    // Prepare game info
    struct retro_game_info game_info = {
        .path = rom_path,
        .data = rom_data,
        .size = (size_t)file_size,
        .meta = NULL
    };
    
    // Load the game
    bool success = frontend->core->retro_load_game(&game_info);
    
    if (!success) {
        fprintf(stderr, "Failed to load ROM\n");
        free(rom_data);
        return false;
    }
    
    printf("Loaded ROM: %s (%ld bytes)\n", rom_path, file_size);
    
    // Update AV info after game is loaded (resolution may change)
    libretro_frontend_update_av_info(frontend);
    
    // Note: We don't free rom_data here because the core might need it
    // The core should handle cleanup when retro_unload_game is called
    
    return true;
}

/**
 * Run one frame of the core
 * Calls retro_run which generates one frame of video/audio
 */
void libretro_frontend_run_frame(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core || !frontend->initialized) return;
    
    if (frontend->core->retro_run) {
        frontend->core->retro_run();
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
 * Deinitialize and cleanup the frontend
 * Unloads the core and frees all allocated resources
 */
void libretro_frontend_deinit(libretro_frontend_t* frontend) {
    if (!frontend) return;
    
    // Unload game if loaded
    if (frontend->core && frontend->core->retro_unload_game) {
        frontend->core->retro_unload_game();
    }
    
    // Deinitialize core
    if (frontend->core && frontend->core->retro_deinit) {
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
    if (!g_frontend) return false;
    
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            unsigned* format = (unsigned*)data;
            if (g_frontend) {
                g_frontend->pixel_format = *format;
                const char* format_name = "Unknown";
                switch (*format) {
                    case RETRO_PIXEL_FORMAT_0RGB1555: format_name = "0RGB1555"; break;
                    case RETRO_PIXEL_FORMAT_XRGB8888: format_name = "XRGB8888"; break;
                    case RETRO_PIXEL_FORMAT_RGB565: format_name = "RGB565"; break;
                }
                printf("Core requested pixel format: %u (%s)\n", *format, format_name);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
            const char** dir = (const char**)data;
            *dir = "./";
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
            const char** dir = (const char**)data;
            *dir = "./";
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
            bool* support = (bool*)data;
            *support = true;
            return true;
        }
        default:
            return false;
    }
}

static void retro_video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!g_frontend || !data) return;
    
    static int frame_count = 0;
    if (frame_count++ < 3) {
        printf("Video refresh[%d]: %ux%u, pitch=%zu, format=%u, bytes_per_pixel=%d\n", 
               frame_count, width, height, pitch, g_frontend->pixel_format,
               g_frontend->pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 2 : 4);
        printf("  Expected pitch: %zu, actual: %zu, pixels_per_line: %zu\n",
               (size_t)(width * (g_frontend->pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 2 : 4)),
               pitch, pitch / (g_frontend->pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 2 : 4));
    }
    
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
            // If pitch=512 and width=240, check if it's actually RGB565
            // 512/2 = 256 pixels (close to 240), vs 512/4 = 128 pixels
            
            size_t pixels_from_pitch_16bit = pitch / 2;
            bool likely_rgb565 = (pixels_from_pitch_16bit >= width - 10 && pixels_from_pitch_16bit <= width + 20);
            
            if (likely_rgb565 && frame_count <= 3) {
                printf("  WARNING: Format says XRGB8888 but pitch suggests RGB565! Treating as RGB565.\n");
            }
            
            if (likely_rgb565) {
                // Treat as RGB565 even though format says XRGB8888
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint8_t* dst_line = (uint8_t*)(dst + y * width);
                    
                    for (unsigned x = 0; x < width; x++) {
                        const uint16_t* src_pixel = (const uint16_t*)(src_line + x * 2);
                        uint8_t* dst_pixel = dst_line + x * 4;
                        
                        uint16_t pixel = *src_pixel;
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        
                        dst_pixel[0] = r;   // R
                        dst_pixel[1] = g;   // G
                        dst_pixel[2] = b;   // B
                        dst_pixel[3] = 0xFF; // A
                    }
                }
            } else {
                // Normal XRGB8888 handling
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint8_t* dst_line = (uint8_t*)(dst + y * width);
                    
                    for (unsigned x = 0; x < width; x++) {
                        const uint8_t* src_pixel = src_line + x * 4;
                        uint8_t* dst_pixel = dst_line + x * 4;
                        
                        // XRGB8888: [B, G, R, X] -> RGBA8888: [R, G, B, A]
                        uint8_t b = src_pixel[0];
                        uint8_t g = src_pixel[1];
                        uint8_t r = src_pixel[2];
                        
                        dst_pixel[0] = r;   // R
                        dst_pixel[1] = g;   // G
                        dst_pixel[2] = b;   // B
                        dst_pixel[3] = 0xFF; // A
                    }
                }
            }
            break;
        }
        case RETRO_PIXEL_FORMAT_RGB565: {
            // RGB565: 16-bit, pitch is in bytes
            size_t expected_pitch = width * 2;
            for (unsigned y = 0; y < height; y++) {
                const uint8_t* src_bytes = (const uint8_t*)data + y * pitch;
                uint32_t* dst_line = dst + y * width;
                
                if (pitch == expected_pitch) {
                    // Fast path: pitch matches
                    const uint16_t* src_line = (const uint16_t*)src_bytes;
                    for (unsigned x = 0; x < width; x++) {
                        uint16_t pixel = src_line[x];
                        // Extract RGB565 components (little-endian)
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        // Convert to RGBA8888
                        dst_line[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                    }
                } else {
                    // Slow path: handle variable pitch
                    for (unsigned x = 0; x < width; x++) {
                        const uint16_t* pixel_ptr = (const uint16_t*)(src_bytes + x * 2);
                        uint16_t pixel = *pixel_ptr;
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        dst_line[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                    }
                }
            }
            break;
        }
        case RETRO_PIXEL_FORMAT_0RGB1555: {
            // 0RGB1555: 16-bit, pitch is in bytes
            for (unsigned y = 0; y < height; y++) {
                const uint16_t* src_line = (const uint16_t*)((const char*)data + y * pitch);
                uint32_t* dst_line = dst + y * width;
                for (unsigned x = 0; x < width; x++) {
                    uint16_t pixel = src_line[x];
                    // Extract 0RGB1555 components
                    uint8_t r = ((pixel >> 10) & 0x1F) << 3;
                    uint8_t g = ((pixel >> 5) & 0x1F) << 3;
                    uint8_t b = (pixel & 0x1F) << 3;
                    // Convert to RGBA8888
                    dst_line[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Unsupported pixel format: %u\n", g_frontend->pixel_format);
            break;
    }
}

static void retro_audio_sample_callback(int16_t left, int16_t right) {
    (void)left;
    (void)right;
    // Single sample callback - less efficient, but some cores use it
    // We'll handle audio in the batch callback instead
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
 * @param index Index (unused for joypad)
 * @param id Button/axis ID
 * @return Input state (1 for pressed, 0 for released)
 */
static int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id) {
    (void)index;
    if (!g_frontend) return 0;
    
    if (device == RETRO_DEVICE_JOYPAD && port < 16 && id < 16) {
        return g_frontend->input_state[port][id] ? 1 : 0;
    }
    
    return 0;
}

