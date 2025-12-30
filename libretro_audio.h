/*
 * libretro_audio.h - Libretro Audio Callbacks
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#ifndef LIBRETRO_AUDIO_H
#define LIBRETRO_AUDIO_H

#include "libretro.h"
#include "libretro_frontend.h"

/**
 * Audio sample callback - receives single audio samples (less efficient)
 * @param left Left channel sample
 * @param right Right channel sample
 */
void retro_audio_sample_callback(int16_t left, int16_t right);

/**
 * Audio sample batch callback - receives batches of audio samples (preferred)
 * @param data Pointer to interleaved stereo int16_t samples
 * @param frames Number of frames (samples per channel)
 * @return Number of frames actually processed
 */
size_t retro_audio_sample_batch_callback(const int16_t* data, size_t frames);

/**
 * Flush any remaining samples in the single-sample audio buffer
 */
void libretro_audio_flush_buffer(void);

/**
 * Set the frontend instance for callbacks
 * @param frontend Frontend instance (can be NULL)
 */
void libretro_audio_set_frontend(libretro_frontend_t* frontend);

#endif // LIBRETRO_AUDIO_H

