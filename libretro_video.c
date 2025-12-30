/*
 * libretro_video.c - Libretro Video Callback Implementation
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 */

#include "libretro_video.h"
#include "libretro_frontend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global frontend instance for callbacks
static libretro_frontend_t* g_frontend = NULL;

void libretro_video_set_frontend(libretro_frontend_t* frontend) {
    g_frontend = frontend;
}

static int frame_count = 0;

/**
 * Video refresh callback implementation
 */
void retro_video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!g_frontend || !data) {
        fprintf(stderr, "ERROR: video_callback called with NULL frontend or data!\n");
        return;
    }
    
    if (width == 0 || height == 0) {
        fprintf(stderr, "WARNING: video_callback called with zero dimensions: %ux%u\n", width, height);
        return;
    }
    
    static int call_count = 0;
    static bool warned_black = false;
    call_count++;
    
    // Debug: Check if data is all zeros (black screen) - only warn once
    if (!warned_black && call_count >= 10) {
        const uint16_t* pixels = (const uint16_t*)data;
        int non_zero_count = 0;
        int sample_size = (width * height < 100) ? width * height : 100;
        for (int i = 0; i < sample_size; i++) {
            if (pixels[i] != 0) {
                non_zero_count++;
                break;
            }
        }
        if (non_zero_count == 0) {
            fprintf(stderr, "WARNING: Video callback receiving all-zero (black) data after %d frames\n", call_count);
            warned_black = true;
        }
    }
    
    static bool format_auto_detected = false;
    
    // Auto-detect RGB565 format if SET_PIXEL_FORMAT was never called
    if (!format_auto_detected && g_frontend->pixel_format == RETRO_PIXEL_FORMAT_XRGB8888) {
        size_t expected_pitch_32bit = width * 4;
        size_t expected_pitch_16bit = width * 2;
        if (pitch == expected_pitch_16bit || (pitch != expected_pitch_32bit && pitch % 2 == 0)) {
            fprintf(stderr, "Auto-detecting RGB565 format: pitch=%zu matches 16-bit (width*2=%zu), not 32-bit (width*4=%zu)\n",
                    pitch, expected_pitch_16bit, expected_pitch_32bit);
            g_frontend->pixel_format = RETRO_PIXEL_FORMAT_RGB565;
            format_auto_detected = true;
        }
    }
    
    frame_count++;
    
    // Update dimensions if changed
    if (width != g_frontend->width || height != g_frontend->height || !g_frontend->framebuffer) {
        g_frontend->width = width;
        g_frontend->height = height;
        g_frontend->framebuffer_size = width * height * 4;
        if (g_frontend->framebuffer) {
            free(g_frontend->framebuffer);
        }
        g_frontend->framebuffer = calloc(1, g_frontend->framebuffer_size);
        if (!g_frontend->framebuffer) {
            fprintf(stderr, "Failed to allocate framebuffer in callback\n");
            return;
        }
    }
    
    if (!g_frontend->framebuffer) return;
    
    // Safety check
    if (width * height * 4 > g_frontend->framebuffer_size) {
        fprintf(stderr, "Framebuffer size mismatch: %ux%u needs %zu bytes, have %zu\n",
                width, height, (size_t)(width * height * 4), g_frontend->framebuffer_size);
        return;
    }
    
    uint32_t* dst = (uint32_t*)g_frontend->framebuffer;
    
    // Handle different pixel formats
    switch (g_frontend->pixel_format) {
        case RETRO_PIXEL_FORMAT_XRGB8888: {
            size_t expected_pitch_32bit = width * 4;
            size_t expected_pitch_16bit = width * 2;
            size_t pitch_diff_32bit = (pitch > expected_pitch_32bit) ? (pitch - expected_pitch_32bit) : (expected_pitch_32bit - pitch);
            size_t pitch_diff_16bit = (pitch > expected_pitch_16bit) ? (pitch - expected_pitch_16bit) : (expected_pitch_16bit - pitch);
            
            bool likely_rgb565 = (pitch_diff_16bit < pitch_diff_32bit && pitch_diff_16bit <= 32) || 
                                 (pitch == expected_pitch_16bit) ||
                                 (pitch % 2 == 0 && pitch / 2 >= width - 4 && pitch / 2 <= width + 4);
            
            if (likely_rgb565) {
                // Treat as RGB565 even though format says XRGB8888
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint32_t* dst_line = dst + y * width;
                    
                    for (unsigned x = 0; x < width; x++) {
                        const uint16_t* src_pixel = (const uint16_t*)(src_line + x * 2);
                        uint16_t pixel = *src_pixel;
                        
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            } else {
                // Normal XRGB8888 handling
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint32_t* dst_line = dst + y * width;
                    
                    for (unsigned x = 0; x < width; x++) {
                        const uint8_t* src_pixel = src_line + x * 4;
                        
                        uint8_t byte0 = src_pixel[0];
                        uint8_t byte1 = src_pixel[1];
                        uint8_t byte2 = src_pixel[2];
                        
                        uint8_t r = byte2;
                        uint8_t g = byte1;
                        uint8_t b = byte0;
                        
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            }
            break;
        }
        case RETRO_PIXEL_FORMAT_RGB565: {
            size_t expected_pitch = width * 2;
            
            for (unsigned y = 0; y < height; y++) {
                const uint8_t* src_bytes = (const uint8_t*)data + y * pitch;
                uint32_t* dst_line = dst + y * width;
                
                if (pitch == expected_pitch) {
                    // Fast path
                    const uint16_t* src_line = (const uint16_t*)src_bytes;
                    for (unsigned x = 0; x < width; x++) {
                        uint16_t pixel = src_line[x];
                        
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                } else {
                    // Slow path: handle variable pitch
                    for (unsigned x = 0; x < width; x++) {
                        const uint16_t* pixel_ptr = (const uint16_t*)(src_bytes + x * 2);
                        uint16_t pixel = *pixel_ptr;
                        
                        uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel & 0x1F) << 3;
                        
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            }
            break;
        }
        case RETRO_PIXEL_FORMAT_0RGB1555: {
            for (unsigned y = 0; y < height; y++) {
                const uint16_t* src_line = (const uint16_t*)((const char*)data + y * pitch);
                uint32_t* dst_line = dst + y * width;
                for (unsigned x = 0; x < width; x++) {
                    uint16_t pixel = src_line[x];
                    uint8_t r = ((pixel >> 10) & 0x1F) << 3;
                    uint8_t g = ((pixel >> 5) & 0x1F) << 3;
                    uint8_t b = (pixel & 0x1F) << 3;
                    dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Unsupported pixel format: %u\n", g_frontend->pixel_format);
            break;
    }
}

