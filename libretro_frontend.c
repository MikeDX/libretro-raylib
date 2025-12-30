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
#include "libretro.h"
#include "libretro_environment.h"
#include "libretro_video.h"
#include "libretro_audio.h"
#include "libretro_input.h"
#include "libretro_core.h"
#include "libretro_environment.h"  // For retro_environment_callback
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    
    // Initialize audio ring buffer
    frontend->audio_ring_buffer_size = frontend->audio_sample_rate / 4;
    frontend->audio_ring_buffer = (float*)calloc(frontend->audio_ring_buffer_size * 2, sizeof(float));
    frontend->audio_ring_read_pos = 0;
    frontend->audio_ring_write_pos = 0;
    frontend->audio_ring_available = 0;
    
    frontend->pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;
    frontend->pixel_format_raw = RETRO_PIXEL_FORMAT_XRGB8888;
    
    memset(frontend->keyboard_state, 0, sizeof(frontend->keyboard_state));
    
    // Set global frontend for callbacks
    g_frontend = frontend;
    libretro_environment_set_frontend(frontend);
    libretro_video_set_frontend(frontend);
    libretro_audio_set_frontend(frontend);
    libretro_input_set_frontend(frontend);
    
    return true;
}

bool libretro_frontend_load_core(libretro_frontend_t* frontend, const char* core_path) {
    if (!frontend || !core_path) return false;
    
    // Set frontend for callbacks before loading
    g_frontend = frontend;
    libretro_environment_set_frontend(frontend);
    
    return libretro_core_load(frontend, core_path);
}

bool libretro_frontend_init_core(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core) return false;
    return libretro_core_init(frontend);
}

bool libretro_frontend_load_rom(libretro_frontend_t* frontend, const char* rom_path) {
    if (!frontend || !frontend->core) return false;
    return libretro_core_load_rom(frontend, rom_path);
}

void libretro_frontend_update_av_info(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core) return;
    libretro_core_update_av_info(frontend);
}

void libretro_frontend_run_frame(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core || !frontend->initialized) return;
    
    // RetroArch polls input before retro_run
    if (frontend->has_set_input_poll && g_frontend) {
        retro_input_poll_callback();
    }
    
    if (frontend->core->retro_run) {
        frontend->core->retro_run();
    }
    
    // For VICE and similar cores: Call SET_SYSTEM_AV_INFO after first frame
    // VICE's update_geometry only calls SET_SYSTEM_AV_INFO when runstate > RUNSTATE_FIRST_START
    // So we need to call it after the first retro_run when runstate changes to RUNNING
    if (!frontend->av_info_sent_after_first_frame && frontend->core->retro_get_system_av_info) {
        struct retro_system_av_info av_info;
        frontend->core->retro_get_system_av_info(&av_info);
        // Removed debug logging - format/size info logged in video callback
        retro_environment_callback(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &av_info);
        frontend->av_info_sent_after_first_frame = true;
        
        // Update our AV info after sending it to the core
        libretro_frontend_update_av_info(frontend);
    }
    
    // Flush any accumulated single-sample audio after each frame
    libretro_audio_flush_buffer();
}

void libretro_frontend_reset(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core || !frontend->initialized) return;
    
    if (frontend->core->retro_reset) {
        frontend->core->retro_reset();
    }
}

void* libretro_frontend_get_framebuffer(libretro_frontend_t* frontend) {
    if (!frontend) return NULL;
    return frontend->framebuffer;
}

void libretro_frontend_get_video_size(libretro_frontend_t* frontend, unsigned* width, unsigned* height) {
    if (!frontend || !width || !height) return;
    *width = frontend->width;
    *height = frontend->height;
}

void libretro_frontend_set_input(libretro_frontend_t* frontend, unsigned port, unsigned button, bool pressed) {
    if (!frontend || port >= 16 || button >= 16) return;
    frontend->input_state[port][button] = pressed;
}

void libretro_frontend_set_keyboard_key(libretro_frontend_t* frontend, unsigned keycode, bool pressed) {
    if (!frontend || keycode >= RETROK_LAST) return;
    frontend->keyboard_state[keycode] = pressed;
}

size_t libretro_frontend_get_audio_samples(libretro_frontend_t* frontend, float* buffer, size_t max_frames) {
    if (!frontend || !buffer || max_frames == 0) return 0;
    
    if (!frontend->audio_ring_buffer || frontend->audio_ring_buffer_size == 0) {
        return 0;
    }
    
    size_t frames_to_read = (max_frames < frontend->audio_ring_available) ? max_frames : frontend->audio_ring_available;
    
    if (frames_to_read == 0) {
        // Underrun - fill with silence
        memset(buffer, 0, max_frames * 2 * sizeof(float));
        return max_frames;
    }
    
    // Read from ring buffer
    for (size_t i = 0; i < frames_to_read; i++) {
        size_t read_idx = (frontend->audio_ring_read_pos + i) % frontend->audio_ring_buffer_size;
        float* src = frontend->audio_ring_buffer + read_idx * 2;
        
        buffer[i * 2] = src[0];
        buffer[i * 2 + 1] = src[1];
    }
    
    // Fill remaining with silence if we read less than requested
    if (frames_to_read < max_frames) {
        memset(buffer + frames_to_read * 2, 0, (max_frames - frames_to_read) * 2 * sizeof(float));
    }
    
    frontend->audio_ring_read_pos = (frontend->audio_ring_read_pos + frames_to_read) % frontend->audio_ring_buffer_size;
    frontend->audio_ring_available -= frames_to_read;
    
    return max_frames; // Always return requested frames (filled with silence if needed)
}

void libretro_frontend_deinit(libretro_frontend_t* frontend) {
    if (!frontend) return;
    
    // Unload core
    libretro_core_unload(frontend);
    
    // Free allocated memory
    // Safety: Only free if framebuffer_size > 0 (indicates it was allocated)
    if (frontend->framebuffer && frontend->framebuffer_size > 0) {
        free(frontend->framebuffer);
        frontend->framebuffer = NULL;
        frontend->framebuffer_size = 0;
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
    
    // Clear global frontend
    if (g_frontend == frontend) {
        g_frontend = NULL;
        libretro_environment_set_frontend(NULL);
        libretro_video_set_frontend(NULL);
        libretro_audio_set_frontend(NULL);
        libretro_input_set_frontend(NULL);
    }
}
