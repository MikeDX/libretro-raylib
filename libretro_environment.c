/*
 * libretro_environment.c - Libretro Environment Callback Implementation
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#include "libretro_environment.h"
#include "libretro_frontend.h"
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

// Global frontend instance for callbacks
static libretro_frontend_t* g_frontend = NULL;

void libretro_environment_set_frontend(libretro_frontend_t* frontend) {
    g_frontend = frontend;
}

// Log callback implementation
static void RETRO_CALLCONV retro_log_callback(enum retro_log_level level, const char* fmt, ...) {
    (void)level; // We'll log everything to stderr regardless of level
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/**
 * Environment callback implementation
 * Handles requests from the core for system information and configuration
 */
bool retro_environment_callback(unsigned cmd, void* data) {
    if (!g_frontend) {
        if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
            fprintf(stderr, "ERROR: SET_PIXEL_FORMAT called but g_frontend is NULL! cmd=%u\n", cmd);
        }
        fprintf(stderr, "WARNING: Environment callback called with cmd=%u but g_frontend is NULL\n", cmd);
        return false;
    }
    
    // Debug output for important callbacks
    const char* cmd_name = "UNKNOWN";
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: cmd_name = "SET_PIXEL_FORMAT"; break;
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: cmd_name = "GET_SYSTEM_DIRECTORY"; break;
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: cmd_name = "SET_SUPPORT_NO_GAME"; break;
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: cmd_name = "GET_LOG_INTERFACE"; break;
        default: break;
    }
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT || cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY) {
        fprintf(stderr, "Environment callback: cmd=%u (%s)\n", cmd, cmd_name);
    }
    
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            unsigned* format = (unsigned*)data;
            if (!g_frontend) return false;
            if (g_frontend) {
                switch (*format) {
                    case RETRO_PIXEL_FORMAT_0RGB1555: 
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_0RGB1555;
                        g_frontend->pixel_format_raw = *format;
                        fprintf(stderr, "Pixel format: 0RGB1555 (format 0) - R=bits 10-14, G=bits 5-9, B=bits 0-4\n");
                        break;
                    case RETRO_PIXEL_FORMAT_XRGB8888: 
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;
                        g_frontend->pixel_format_raw = *format;
                        fprintf(stderr, "Pixel format: XRGB8888 (format 1) - 32-bit\n");
                        break;
                    case RETRO_PIXEL_FORMAT_RGB565: 
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
                        g_frontend->pixel_format_raw = *format;
                        fprintf(stderr, "Pixel format: RGB565 (format 2) - R=bits 11-15, G=bits 5-10, B=bits 0-4\n");
                        break;
                    case RETRO_PIXEL_FORMAT_RGB555:
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
                        g_frontend->pixel_format_raw = 12;
                        fprintf(stderr, "Pixel format: RGB565 (format 12 from snes9x) - R=bits 11-15, G=bits 5-10, B=bits 0-4\n");
                        break;
                    default:
                        g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
                        g_frontend->pixel_format_raw = *format;
                        break;
                }
            }
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
            if (!data) return false;
            const char** dir = (const char**)data;
            static const char* system_dir = ".";
            *dir = system_dir;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
            if (!data) return false;
            const char** dir = (const char**)data;
            *dir = "./";
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
            if (!data) return false;
            bool* support = (bool*)data;
            *support = true;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY: {
            if (!data) return false;
            const char** dir = (const char**)data;
            *dir = "./";
            return true;
        }
        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_VARIABLES: {
            return true;
        }
        case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
            if (!data) return false;
            unsigned* enable = (unsigned*)data;
            *enable = 3; // Enable both audio and video
            return true;
        }
        case RETRO_ENVIRONMENT_SET_AUDIO_VIDEO_ENABLE: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_FASTFORWARDING: {
            return true;
        }
        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
            return true;
        }
        case 32: { // RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO
            if (!data) return false;
            const struct retro_system_av_info* av_info = (const struct retro_system_av_info*)data;
            if (g_frontend && av_info) {
                g_frontend->width = av_info->geometry.base_width;
                g_frontend->height = av_info->geometry.base_height;
                g_frontend->aspect_ratio = av_info->geometry.aspect_ratio;
                g_frontend->fps = av_info->timing.fps;
                unsigned new_sample_rate = (unsigned)av_info->timing.sample_rate;
                if (new_sample_rate > 0 && new_sample_rate != g_frontend->audio_sample_rate) {
                    g_frontend->audio_sample_rate = new_sample_rate;
                }
                fprintf(stderr, "SET_SYSTEM_AV_INFO: %ux%u (aspect: %.2f, fps: %.2f, sample rate: %.2f Hz)\n",
                        g_frontend->width, g_frontend->height, g_frontend->aspect_ratio, 
                        g_frontend->fps, (double)g_frontend->audio_sample_rate);
            }
            return true;
        }
        case 37: { // RETRO_ENVIRONMENT_SET_GEOMETRY
            if (!data) return false;
            const struct retro_game_geometry* geom = (const struct retro_game_geometry*)data;
            if (g_frontend && geom) {
                g_frontend->width = geom->base_width;
                g_frontend->height = geom->base_height;
                g_frontend->aspect_ratio = geom->aspect_ratio;
                fprintf(stderr, "SET_GEOMETRY: %ux%u (aspect: %.2f)\n",
                        g_frontend->width, g_frontend->height, g_frontend->aspect_ratio);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
            if (!data) return false;
            struct retro_log_callback* log_cb = (struct retro_log_callback*)data;
            log_cb->log = retro_log_callback;
            return true;
        }
        case 33: // RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
        case 34: // RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION
        case 35: // RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK
        case 36: // RETRO_ENVIRONMENT_GET_INPUT_BITMASKS
        case 38: { // RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS
            return true;
        }
        default:
            return false;
    }
}

