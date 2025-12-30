/*
 * libretro_input.h - Libretro Input Callbacks
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#ifndef LIBRETRO_INPUT_H
#define LIBRETRO_INPUT_H

#include "libretro.h"
#include "libretro_frontend.h"

/**
 * Input poll callback - called before input state queries
 */
void retro_input_poll_callback(void);

/**
 * Input state callback - queries the current state of an input button/axis
 * @param port Controller port (0-15)
 * @param device Device type (RETRO_DEVICE_*)
 * @param index Index (unused for joypad/keyboard)
 * @param id Button/axis ID or keyboard keycode
 * @return Input state (1 for pressed, 0 for released)
 */
int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id);

/**
 * Set the frontend instance for callbacks
 * @param frontend Frontend instance (can be NULL)
 */
void libretro_input_set_frontend(libretro_frontend_t* frontend);

#endif // LIBRETRO_INPUT_H

