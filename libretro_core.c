/*
 * libretro_core.c - Libretro Core Loading and Management Implementation
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#include "libretro_core.h"
#include "libretro_frontend.h"
#include "libretro_environment.h"
#include "libretro_video.h"
#include "libretro_audio.h"
#include "libretro_input.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/**
 * Set up callbacks after loading a game
 */
static void setup_callbacks_after_load(libretro_frontend_t* frontend) {
    if (!frontend->has_set_video_refresh) {
        fprintf(stderr, "Setting up video/audio/input callbacks after game load (matching RetroArch)...\n");
        typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
        typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
        typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
        typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
        typedef void (*retro_set_input_state_t)(retro_input_state_t);
        
        retro_set_video_refresh_t set_video = (retro_set_video_refresh_t)dlsym(frontend->core_handle, SYM_RETRO_SET_VIDEO_REFRESH);
        retro_set_audio_sample_t set_audio = (retro_set_audio_sample_t)dlsym(frontend->core_handle, SYM_RETRO_SET_AUDIO_SAMPLE);
        retro_set_audio_sample_batch_t set_audio_batch = (retro_set_audio_sample_batch_t)dlsym(frontend->core_handle, SYM_RETRO_SET_AUDIO_SAMPLE_BATCH);
        retro_set_input_poll_t set_input_poll = (retro_set_input_poll_t)dlsym(frontend->core_handle, SYM_RETRO_SET_INPUT_POLL);
        retro_set_input_state_t set_input_state = (retro_set_input_state_t)dlsym(frontend->core_handle, SYM_RETRO_SET_INPUT_STATE);
        
        if (set_video && set_audio && set_audio_batch && set_input_poll && set_input_state) {
            set_video(retro_video_refresh_callback);
            set_audio(retro_audio_sample_callback);
            set_audio_batch(retro_audio_sample_batch_callback);
            set_input_poll(retro_input_poll_callback);
            set_input_state(retro_input_state_callback);
            
            frontend->has_set_video_refresh = true;
            frontend->has_set_audio_sample = true;
            frontend->has_set_audio_sample_batch = true;
            frontend->has_set_input_poll = true;
            frontend->has_set_input_state = true;
        } else {
            fprintf(stderr, "Warning: Failed to set up video/audio/input callbacks\n");
        }
    }
}

/**
 * Load a libretro core from a dynamic library
 */
bool libretro_core_load(libretro_frontend_t* frontend, const char* core_path) {
    if (!frontend || !core_path) return false;
    
    void* handle = dlopen(core_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load core: %s\n", dlerror());
        return false;
    }
    
    frontend->core_handle = handle;
    
    frontend->core = (struct retro_core_t*)calloc(1, sizeof(struct retro_core_t));
    if (!frontend->core) {
        dlclose(handle);
        frontend->core_handle = NULL;
        return false;
    }
    
    // Load symbols for callback setters
    typedef void (*retro_set_environment_t)(retro_environment_t);
    
    retro_set_environment_t set_env = (retro_set_environment_t)dlsym(handle, SYM_RETRO_SET_ENVIRONMENT);
    
    if (!set_env) {
        fprintf(stderr, "Failed to load required symbols from core\n");
        free(frontend->core);
        frontend->core = NULL;
        dlclose(handle);
        frontend->core_handle = NULL;
        return false;
    }
    
    // Set environment callback early
    libretro_environment_set_frontend(frontend);
    fprintf(stderr, "Setting environment callback (matching RetroArch sequence)...\n");
    set_env(retro_environment_callback);
    frontend->has_set_environment = true;
    
    // Load core functions
    frontend->core->retro_init = (void (*)(void))dlsym(handle, SYM_RETRO_INIT);
    frontend->core->retro_deinit = (void (*)(void))dlsym(handle, SYM_RETRO_DEINIT);
    frontend->core->retro_api_version = (unsigned (*)(void))dlsym(handle, SYM_RETRO_API_VERSION);
    frontend->core->retro_get_system_info = (void (*)(struct retro_system_info*))dlsym(handle, SYM_RETRO_GET_SYSTEM_INFO);
    frontend->core->retro_get_system_av_info = (void (*)(struct retro_system_av_info*))dlsym(handle, SYM_RETRO_GET_SYSTEM_AV_INFO);
    frontend->core->retro_set_controller_port_device = (void (*)(unsigned, unsigned))dlsym(handle, SYM_RETRO_SET_CONTROLLER_PORT_DEVICE);
    frontend->core->retro_reset = (void (*)(void))dlsym(handle, SYM_RETRO_RESET);
    frontend->core->retro_run = (void (*)(void))dlsym(handle, SYM_RETRO_RUN);
    frontend->core->retro_serialize_size = (size_t (*)(void))dlsym(handle, SYM_RETRO_SERIALIZE_SIZE);
    frontend->core->retro_serialize = (bool (*)(void*, size_t))dlsym(handle, SYM_RETRO_SERIALIZE);
    frontend->core->retro_unserialize = (bool (*)(const void*, size_t))dlsym(handle, SYM_RETRO_UNSERIALIZE);
    frontend->core->retro_cheat_reset = (void (*)(void))dlsym(handle, SYM_RETRO_CHEAT_RESET);
    frontend->core->retro_cheat_set = (void (*)(unsigned, bool, const char*))dlsym(handle, SYM_RETRO_CHEAT_SET);
    frontend->core->retro_load_game = (bool (*)(const struct retro_game_info*))dlsym(handle, SYM_RETRO_LOAD_GAME);
    frontend->core->retro_unload_game = (void (*)(void))dlsym(handle, SYM_RETRO_UNLOAD_GAME);
    frontend->core->retro_get_region = (unsigned (*)(void))dlsym(handle, SYM_RETRO_GET_REGION);
    frontend->core->retro_get_memory_data = (void* (*)(unsigned))dlsym(handle, SYM_RETRO_GET_MEMORY_DATA);
    frontend->core->retro_get_memory_size = (size_t (*)(unsigned))dlsym(handle, SYM_RETRO_GET_MEMORY_SIZE);
    
    if (!frontend->core->retro_init || !frontend->core->retro_run) {
        fprintf(stderr, "Failed to load core functions\n");
        free(frontend->core);
        frontend->core = NULL;
        dlclose(handle);
        frontend->core_handle = NULL;
        return false;
    }
    
    return true;
}

/**
 * Initialize the loaded libretro core
 */
bool libretro_core_init(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core) return false;
    
    struct retro_system_info info;
    if (frontend->core->retro_get_system_info) {
        frontend->core->retro_get_system_info(&info);
        frontend->need_fullpath = info.need_fullpath;
        fprintf(stderr, "Core: %s %s\n", info.library_name, info.library_version);
    }
    
    if (frontend->core->retro_init) {
        frontend->core->retro_init();
    }
    
    if (frontend->core->retro_set_controller_port_device) {
        frontend->core->retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    }
    
    frontend->width = 240;
    frontend->height = 160;
    frontend->aspect_ratio = 3.0f / 2.0f;
    
    frontend->initialized = true;
    return true;
}

/**
 * Update audio/video information from the core
 */
void libretro_core_update_av_info(libretro_frontend_t* frontend) {
    if (!frontend || !frontend->core) return;
    
    struct retro_system_av_info av_info;
    if (frontend->core->retro_get_system_av_info) {
        frontend->core->retro_get_system_av_info(&av_info);
        frontend->width = av_info.geometry.base_width;
        frontend->height = av_info.geometry.base_height;
        frontend->aspect_ratio = av_info.geometry.aspect_ratio;
        
        unsigned new_sample_rate = (unsigned)av_info.timing.sample_rate;
        if (new_sample_rate == 0) {
            fprintf(stderr, "Warning: Core reported 0 Hz sample rate, using default 44100 Hz\n");
            new_sample_rate = 44100;
        }
        fprintf(stderr, "Video: %ux%u (aspect: %.2f, fps: %.2f)\n", 
                frontend->width, frontend->height, frontend->aspect_ratio, frontend->fps);
        fprintf(stderr, "Audio: %u Hz\n", new_sample_rate);
        if (new_sample_rate != frontend->audio_sample_rate) {
            if (frontend->audio_ring_buffer) {
                free(frontend->audio_ring_buffer);
            }
            frontend->audio_sample_rate = new_sample_rate;
            frontend->audio_ring_buffer_size = frontend->audio_sample_rate / 4;
            if (frontend->audio_ring_buffer_size == 0) {
                frontend->audio_ring_buffer_size = 11025;
            }
            frontend->audio_ring_buffer = (float*)calloc(frontend->audio_ring_buffer_size * 2, sizeof(float));
            frontend->audio_ring_read_pos = 0;
            frontend->audio_ring_write_pos = 0;
            frontend->audio_ring_available = 0;
        }
        
        frontend->fps = av_info.timing.fps;
        
        size_t new_size = frontend->width * frontend->height * 4;
        if (new_size != frontend->framebuffer_size || !frontend->framebuffer) {
            if (frontend->framebuffer) {
                free(frontend->framebuffer);
                frontend->framebuffer = NULL;
            }
            frontend->framebuffer_size = new_size;
            frontend->framebuffer = calloc(1, frontend->framebuffer_size);
            if (!frontend->framebuffer) {
                fprintf(stderr, "Failed to allocate framebuffer\n");
                frontend->framebuffer_size = 0;
            }
        }
    }
}

/**
 * Load a ROM file into the core
 */
bool libretro_core_load_rom(libretro_frontend_t* frontend, const char* rom_path) {
    if (!frontend || !frontend->core) return false;
    
    // Handle no-game mode
    if (!rom_path) {
        bool success = frontend->core->retro_load_game(NULL);
        if (success) {
            setup_callbacks_after_load(frontend);
            libretro_core_update_av_info(frontend);
            frontend->rom_path = NULL;
            frontend->rom_data = NULL;
            frontend->rom_data_size = 0;
        }
        return success;
    }
    
    if (!frontend->core->retro_load_game) {
        fprintf(stderr, "Core does not support loading games\n");
        return false;
    }
    
    char* abs_path = realpath(rom_path, NULL);
    if (!abs_path) {
        fprintf(stderr, "Failed to get absolute path for ROM: %s\n", rom_path);
        return false;
    }
    
    struct retro_game_info game_info = {0};
    void* rom_data = NULL;
    
    if (frontend->need_fullpath) {
        game_info.path = abs_path;
        game_info.data = NULL;
        game_info.size = 0;
        game_info.meta = NULL;
    } else {
        FILE* file = fopen(abs_path, "rb");
        if (!file) {
            fprintf(stderr, "Failed to open ROM file: %s\n", abs_path);
            free(abs_path);
            return false;
        }
        
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        if (file_size <= 0) {
            fprintf(stderr, "Invalid ROM file size\n");
            fclose(file);
            free(abs_path);
            return false;
        }
        
        rom_data = malloc(file_size);
        if (!rom_data) {
            fprintf(stderr, "Failed to allocate memory for ROM\n");
            fclose(file);
            free(abs_path);
            return false;
        }
        
        size_t bytes_read = fread(rom_data, 1, file_size, file);
        fclose(file);
        
        if (bytes_read != (size_t)file_size) {
            fprintf(stderr, "Failed to read ROM file completely\n");
            free(rom_data);
            free(abs_path);
            return false;
        }
        
        game_info.path = abs_path;
        game_info.data = rom_data;
        game_info.size = (size_t)file_size;
        game_info.meta = NULL;
    }
    
    bool success = frontend->core->retro_load_game(&game_info);
    
    if (!success) {
        fprintf(stderr, "Failed to load ROM - core returned false\n");
        if (rom_data) {
            free(rom_data);
        }
        free(abs_path);
        return false;
    }
    
    frontend->rom_data = rom_data;
    frontend->rom_data_size = frontend->need_fullpath ? 0 : game_info.size;
    frontend->rom_path = abs_path;
    
    setup_callbacks_after_load(frontend);
    libretro_core_update_av_info(frontend);
    
    return true;
}

/**
 * Unload the core and cleanup resources
 */
void libretro_core_unload(libretro_frontend_t* frontend) {
    if (!frontend) return;
    
    if (frontend->core && frontend->core->retro_unload_game && frontend->rom_path) {
        frontend->core->retro_unload_game();
    }
    
    if (frontend->rom_data) {
        free(frontend->rom_data);
        frontend->rom_data = NULL;
    }
    if (frontend->rom_path) {
        free(frontend->rom_path);
        frontend->rom_path = NULL;
    }
    
    if (frontend->core && frontend->core->retro_deinit && frontend->initialized) {
        frontend->core->retro_deinit();
    }
    
    if (frontend->core_handle) {
        dlclose(frontend->core_handle);
        frontend->core_handle = NULL;
    }
    
    if (frontend->core) {
        free(frontend->core);
        frontend->core = NULL;
    }
}

