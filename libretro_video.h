/*
 * libretro_video.h - Libretro Video Callback
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#ifndef LIBRETRO_VIDEO_H
#define LIBRETRO_VIDEO_H

#include "libretro_api.h"
#include "libretro_frontend.h"

/**
 * Video refresh callback - receives rendered frames from the core
 * @param data Pointer to pixel data
 * @param width Frame width
 * @param height Frame height
 * @param pitch Pitch (bytes per row)
 */
void retro_video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch);

/**
 * Set the frontend instance for callbacks
 * @param frontend Frontend instance (can be NULL)
 */
void libretro_video_set_frontend(libretro_frontend_t* frontend);

#endif // LIBRETRO_VIDEO_H

