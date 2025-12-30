/*
 * libretro_environment.h - Libretro Environment Callback
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#ifndef LIBRETRO_ENVIRONMENT_H
#define LIBRETRO_ENVIRONMENT_H

#include "libretro.h"
#include "libretro_frontend.h"

/**
 * Environment callback - handles core requests for system information
 * @param cmd Command ID
 * @param data Command-specific data
 * @return true if command was handled, false otherwise
 */
bool retro_environment_callback(unsigned cmd, void* data);

/**
 * Set the frontend instance for callbacks
 * @param frontend Frontend instance (can be NULL)
 */
void libretro_environment_set_frontend(libretro_frontend_t* frontend);

#endif // LIBRETRO_ENVIRONMENT_H

