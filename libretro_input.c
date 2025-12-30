/*
 * libretro_input.c - Libretro Input Callback Implementation
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#include "libretro_input.h"
#include "libretro_frontend.h"
#include "libretro_api.h"

// Global frontend instance for callbacks
static libretro_frontend_t* g_frontend = NULL;

void libretro_input_set_frontend(libretro_frontend_t* frontend) {
    g_frontend = frontend;
}

/**
 * Input poll callback implementation
 */
void retro_input_poll_callback(void) {
    // Input polling is handled by raylib in main.c
    // This is called before retro_input_state
}

/**
 * Input state callback implementation
 */
int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id) {
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

