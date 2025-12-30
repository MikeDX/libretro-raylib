/*
 * libretro_audio.c - Libretro Audio Callback Implementation
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#include "libretro_audio.h"
#include "libretro_frontend.h"
#include <stdio.h>
#include <string.h>

// Global frontend instance for callbacks
static libretro_frontend_t* g_frontend = NULL;

void libretro_audio_set_frontend(libretro_frontend_t* frontend) {
    g_frontend = frontend;
}

// Single-sample audio accumulator for cores that use retro_audio_sample_callback
#define SINGLE_SAMPLE_BUFFER_SIZE 512
#define SINGLE_SAMPLE_FLUSH_THRESHOLD 1
static int16_t single_sample_buffer[SINGLE_SAMPLE_BUFFER_SIZE * 2]; // Stereo
static size_t single_sample_count = 0;

/**
 * Audio sample callback implementation
 */
void retro_audio_sample_callback(int16_t left, int16_t right) {
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
        single_sample_buffer[0] = left;
        single_sample_buffer[1] = right;
        single_sample_count = 1;
    }
    
    // Flush when buffer reaches threshold
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

/**
 * Flush any remaining samples in the single-sample buffer
 */
void libretro_audio_flush_buffer(void) {
    if (single_sample_count > 0 && g_frontend) {
        retro_audio_sample_batch_callback(single_sample_buffer, single_sample_count);
        single_sample_count = 0;
    }
}

/**
 * Audio sample batch callback implementation (preferred method)
 */
size_t retro_audio_sample_batch_callback(const int16_t* data, size_t frames) {
    if (!g_frontend || !data || frames == 0) return 0;
    
    if (!g_frontend->audio_ring_buffer || g_frontend->audio_ring_buffer_size == 0) {
        static int error_count = 0;
        if (error_count++ < 3) {
            fprintf(stderr, "ERROR: Audio ring buffer not initialized!\n");
        }
        return 0;
    }
    
    // Convert int16_t samples to float and add to ring buffer
    // Proper ring buffer handling: drop samples if buffer is full (prevent overflow)
    size_t available_space = g_frontend->audio_ring_buffer_size - g_frontend->audio_ring_available;
    size_t frames_to_write = (frames < available_space) ? frames : available_space;
    
    if (frames_to_write == 0) {
        // Buffer full - drop samples to prevent overflow
        static int drop_warn_count = 0;
        if (drop_warn_count++ < 3) {
            fprintf(stderr, "Audio buffer full, dropping %zu frames\n", frames);
        }
        return 0;
    }
    
    // Write to ring buffer
    for (size_t i = 0; i < frames_to_write; i++) {
        size_t write_idx = (g_frontend->audio_ring_write_pos + i) % g_frontend->audio_ring_buffer_size;
        float* buffer = g_frontend->audio_ring_buffer + write_idx * 2;
        
        buffer[0] = (float)data[i * 2] / 32768.0f;
        buffer[1] = (float)data[i * 2 + 1] / 32768.0f;
    }
    
    g_frontend->audio_ring_write_pos = (g_frontend->audio_ring_write_pos + frames_to_write) % g_frontend->audio_ring_buffer_size;
    g_frontend->audio_ring_available += frames_to_write;
    
    return frames_to_write;
}

