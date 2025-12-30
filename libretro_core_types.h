/*
 * libretro_core_types.h - Custom Core Function Pointer Structure
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * This header defines our custom wrapper structure for holding libretro core
 * function pointers. The official libretro.h doesn't provide this structure
 * as cores export functions directly via dlsym.
 */

#ifndef LIBRETRO_CORE_TYPES_H
#define LIBRETRO_CORE_TYPES_H

#include "libretro.h"
#include <stdbool.h>
#include <stddef.h>

//=============================================================================
// Libretro Core Function Pointers Wrapper
//=============================================================================

/**
 * Wrapper structure for holding libretro core function pointers
 * This is our own structure - not part of the official libretro API
 */
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

//=============================================================================
// Core Symbol Names
//=============================================================================

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

#endif // LIBRETRO_CORE_TYPES_H

