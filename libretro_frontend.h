/*
 * libretro_frontend.h - Libretro Frontend API
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * This header defines the frontend API for interfacing with libretro cores.
 * The libretro API is available under the BSD 3-Clause License.
 * See https://www.libretro.com/ for details.
 */

#ifndef LIBRETRO_FRONTEND_H
#define LIBRETRO_FRONTEND_H

#include "libretro.h"
#include "libretro_core_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//=============================================================================
// Frontend Structure
//=============================================================================

/**
 * Main frontend structure containing all state for libretro core management
 */
typedef struct {
    void* core_handle;
    struct retro_core_t* core;
    
    // Video
    unsigned width;          // Display width (from AV info base_width)
    unsigned height;         // Display height (from AV info base_height)
    float aspect_ratio;
    void* framebuffer;
    size_t framebuffer_size;
    unsigned pixel_format; // RETRO_PIXEL_FORMAT_*
    unsigned pixel_format_raw; // Original format value (for format 12 detection)
    
    // Frame cache dimensions (from video callback)
    unsigned frame_width;   // Actual frame buffer width from callback
    unsigned frame_height;  // Actual frame buffer height from callback
    
    // Audio
    float* audio_buffer;
    size_t audio_buffer_size;
    unsigned audio_sample_rate;
    double fps;  // Core's reported FPS
    
    // Audio ring buffer for streaming
    float* audio_ring_buffer;
    size_t audio_ring_buffer_size;  // Total capacity in frames
    size_t audio_ring_read_pos;     // Read position (frames)
    size_t audio_ring_write_pos;    // Write position (frames)
    size_t audio_ring_available;     // Available frames to read
    
    // Input
    bool input_state[16][16]; // [port][button]
    bool keyboard_state[RETROK_LAST]; // Keyboard key states (RETROK_LAST defined in libretro.h)
    
    // Core state
    bool initialized;
    bool has_set_environment;
    bool has_set_video_refresh;
    bool has_set_audio_sample;
    bool has_set_audio_sample_batch;
    bool has_set_input_poll;
    bool has_set_input_state;
    bool av_info_sent_after_first_frame; // Track if SET_SYSTEM_AV_INFO was sent after first frame
    
    // System info (from retro_get_system_info)
    bool need_fullpath;
    
    // ROM data (must remain valid until retro_unload_game is called)
    void* rom_data;
    size_t rom_data_size;
    char* rom_path;  // Path string (must remain valid until retro_unload_game is called)
} libretro_frontend_t;

//=============================================================================
// Public API Functions
//=============================================================================

/**
 * Initialize the libretro frontend
 * @param frontend Pointer to frontend structure to initialize
 * @return true on success, false on failure
 */
bool libretro_frontend_init(libretro_frontend_t* frontend);

/**
 * Load a libretro core from a dynamic library file
 * @param frontend Pointer to initialized frontend structure
 * @param core_path Path to the core .dylib file
 * @return true on success, false on failure
 */
bool libretro_frontend_load_core(libretro_frontend_t* frontend, const char* core_path);

/**
 * Initialize the loaded libretro core
 * @param frontend Pointer to frontend with loaded core
 * @return true on success, false on failure
 */
bool libretro_frontend_init_core(libretro_frontend_t* frontend);

/**
 * Load a ROM file into the core
 * @param frontend Pointer to initialized frontend
 * @param rom_path Path to the ROM file (can be NULL for no-game mode)
 * @return true on success, false on failure
 */
bool libretro_frontend_load_rom(libretro_frontend_t* frontend, const char* rom_path);

/**
 * Update audio/video information from the core
 * Should be called after loading a game or when core is initialized
 * @param frontend Pointer to frontend structure
 */
void libretro_frontend_update_av_info(libretro_frontend_t* frontend);

/**
 * Run one frame of the core
 * @param frontend Pointer to frontend structure
 */
void libretro_frontend_run_frame(libretro_frontend_t* frontend);

/**
 * Reset the core
 * @param frontend Pointer to frontend structure
 */
void libretro_frontend_reset(libretro_frontend_t* frontend);

/**
 * Get the current video framebuffer
 * @param frontend Pointer to frontend structure
 * @return Pointer to framebuffer data (RGBA8888 format), or NULL if not available
 */
void* libretro_frontend_get_framebuffer(libretro_frontend_t* frontend);

/**
 * Get the current video dimensions
 * @param frontend Pointer to frontend structure
 * @param width Output parameter for width
 * @param height Output parameter for height
 */
void libretro_frontend_get_video_size(libretro_frontend_t* frontend, unsigned* width, unsigned* height);

/**
 * Set input state for a joypad button
 * @param frontend Pointer to frontend structure
 * @param port Controller port (0-15)
 * @param button Button ID (RETRO_DEVICE_ID_JOYPAD_* from libretro_api.h)
 * @param pressed true if pressed, false if released
 */
void libretro_frontend_set_input(libretro_frontend_t* frontend, unsigned port, unsigned button, bool pressed);

/**
 * Set keyboard key state
 * @param frontend Pointer to frontend structure
 * @param keycode Libretro keycode (RETROK_* from libretro_api.h)
 * @param pressed true if pressed, false if released
 */
void libretro_frontend_set_keyboard_key(libretro_frontend_t* frontend, unsigned keycode, bool pressed);

/**
 * Get audio samples from the ring buffer for playback
 * @param frontend Pointer to frontend structure
 * @param buffer Output buffer for audio samples (interleaved stereo float)
 * @param max_frames Maximum number of frames to read
 * @return Number of frames actually read
 */
size_t libretro_frontend_get_audio_samples(libretro_frontend_t* frontend, float* buffer, size_t max_frames);

/**
 * Deinitialize and cleanup the frontend
 * @param frontend Pointer to frontend structure to cleanup
 */
void libretro_frontend_deinit(libretro_frontend_t* frontend);

#endif // LIBRETRO_FRONTEND_H

