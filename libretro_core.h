/*
 * libretro_core.h - Libretro Core Loading and Management
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#ifndef LIBRETRO_CORE_H
#define LIBRETRO_CORE_H

#include "libretro_api.h"
#include "libretro_frontend.h"
#include <stdbool.h>

/**
 * Load a libretro core from a dynamic library
 * @param frontend Frontend instance
 * @param core_path Path to the core .dylib file
 * @return true on success, false on failure
 */
bool libretro_core_load(libretro_frontend_t* frontend, const char* core_path);

/**
 * Initialize the loaded libretro core
 * @param frontend Frontend instance with loaded core
 * @return true on success, false on failure
 */
bool libretro_core_init(libretro_frontend_t* frontend);

/**
 * Load a ROM file into the core
 * @param frontend Frontend instance
 * @param rom_path Path to the ROM file (can be NULL for no-game mode)
 * @return true on success, false on failure
 */
bool libretro_core_load_rom(libretro_frontend_t* frontend, const char* rom_path);

/**
 * Update audio/video information from the core
 * @param frontend Frontend instance
 */
void libretro_core_update_av_info(libretro_frontend_t* frontend);

/**
 * Unload the core and cleanup resources
 * @param frontend Frontend instance
 */
void libretro_core_unload(libretro_frontend_t* frontend);

#endif // LIBRETRO_CORE_H

