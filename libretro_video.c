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
#include <stdint.h>

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
    
    // Safety check: ensure framebuffer_size is consistent with framebuffer pointer
    // If framebuffer is set but framebuffer_size is 0, something is wrong
    if (g_frontend->framebuffer && g_frontend->framebuffer_size == 0) {
        fprintf(stderr, "WARNING: framebuffer pointer set but size is 0, resetting\n");
        g_frontend->framebuffer = NULL;
    }
    
    if (width == 0 || height == 0) {
        fprintf(stderr, "WARNING: video_callback called with zero dimensions: %ux%u\n", width, height);
        return;
    }
    
    static int call_count = 0;
    static bool warned_black = false;
    static bool logged_format_info = false;
    call_count++;
    
    // Log format/size info on first frame
    if (!logged_format_info) {
        const char* format_name = "UNKNOWN";
        size_t bytes_per_pixel = 0;
        switch (g_frontend->pixel_format) {
            case RETRO_PIXEL_FORMAT_XRGB8888: format_name = "XRGB8888"; bytes_per_pixel = 4; break;
            case RETRO_PIXEL_FORMAT_RGB565: format_name = "RGB565"; bytes_per_pixel = 2; break;
            case RETRO_PIXEL_FORMAT_0RGB1555: format_name = "0RGB1555"; bytes_per_pixel = 2; break;
        }
        size_t expected_pitch = width * bytes_per_pixel;
        size_t pixels_per_row = pitch / bytes_per_pixel;
        fprintf(stderr, "Video callback: %ux%u, format=%s, pitch=%zu (expected=%zu, diff=%zu, pixels_per_row=%zu, AV_width=%u)\n",
                width, height, format_name, pitch, expected_pitch,
                (pitch > expected_pitch) ? (pitch - expected_pitch) : (expected_pitch - pitch),
                pixels_per_row, g_frontend->width);
        logged_format_info = true;
    }
    
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
    
    frame_count++;
    
    // Proper libretro way (matching RetroArch):
    // - Callback width/height = frame cache dimensions (actual frame buffer size)
    // - AV info base_width/base_height = display/canvas dimensions (what to render)
    // - Store frame cache dimensions separately from display dimensions
    
    // Store frame cache dimensions (from callback)
    g_frontend->frame_width = width;
    g_frontend->frame_height = height;
    
    // Use AV info dimensions for display (base_width/base_height)
    // If AV info not set yet, use frame dimensions as fallback
    unsigned display_width = (g_frontend->width > 0) ? g_frontend->width : width;
    unsigned display_height = (g_frontend->height > 0) ? g_frontend->height : height;
    
    // Allocate framebuffer based on display dimensions (what we'll render)
    // Only reallocate if dimensions actually changed
    size_t needed_size = display_width * display_height * 4;
    if (needed_size == 0) {
        fprintf(stderr, "ERROR: Invalid framebuffer size: %ux%u\n", display_width, display_height);
        return;
    }
    
    // Only reallocate if size changed or framebuffer is NULL
    // Safety: Only free if framebuffer_size > 0 (indicates it was allocated)
    if (needed_size != g_frontend->framebuffer_size || !g_frontend->framebuffer) {
        // Only free if we actually allocated it (framebuffer_size > 0)
        // Double-check that framebuffer is not NULL before freeing
        if (g_frontend->framebuffer != NULL && g_frontend->framebuffer_size > 0) {
            // Additional safety: verify the pointer looks valid (not obviously garbage)
            // This is a heuristic check - valid pointers are typically aligned
            uintptr_t ptr_val = (uintptr_t)g_frontend->framebuffer;
            if ((ptr_val % sizeof(void*)) == 0 && ptr_val > 0x1000) {
                free(g_frontend->framebuffer);
            } else {
                fprintf(stderr, "WARNING: Invalid framebuffer pointer detected (0x%lx), not freeing\n", (unsigned long)ptr_val);
            }
        }
        g_frontend->framebuffer = NULL;
        g_frontend->framebuffer_size = needed_size;
        g_frontend->framebuffer = calloc(1, g_frontend->framebuffer_size);
        if (!g_frontend->framebuffer) {
            fprintf(stderr, "Failed to allocate framebuffer in callback\n");
            g_frontend->framebuffer_size = 0;
            return;
        }
    }
    
    // Update display dimensions
    g_frontend->width = display_width;
    g_frontend->height = display_height;
    
    if (!g_frontend->framebuffer) return;
    
    // Safety check
    if (display_width * display_height * 4 > g_frontend->framebuffer_size) {
        fprintf(stderr, "Framebuffer size mismatch: %ux%u needs %zu bytes, have %zu\n",
                display_width, display_height, (size_t)(display_width * display_height * 4), g_frontend->framebuffer_size);
        return;
    }
    
    uint32_t* dst = (uint32_t*)g_frontend->framebuffer;
    
    // Handle different pixel formats
    switch (g_frontend->pixel_format) {
        case RETRO_PIXEL_FORMAT_XRGB8888: {
            // XRGB8888 handling - proper libretro way
            // Read exactly 'width' pixels per row from callback (frame cache dimensions)
            // Use 'pitch' to advance to next row (may be larger than width*4 due to alignment)
            // Scale to display_width if different (display/canvas dimensions from AV info)
            
            // If frame width matches display width, direct copy
            if (width == display_width && height == display_height) {
                for (unsigned y = 0; y < height; y++) {
                    const uint8_t* src_line = (const uint8_t*)data + y * pitch;
                    uint32_t* dst_line = dst + y * display_width;
                    
                    const uint32_t* src_pixels = (const uint32_t*)src_line;
                    for (unsigned x = 0; x < width; x++) {
                        uint32_t pixel = src_pixels[x];
                        uint8_t r = (pixel >> 16) & 0xFF;
                        uint8_t g = (pixel >> 8) & 0xFF;
                        uint8_t b = pixel & 0xFF;
                        dst_line[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            } else {
                // Scale from frame dimensions to display dimensions
                for (unsigned y = 0; y < display_height; y++) {
                    unsigned src_y = (y * height) / display_height;
                    const uint8_t* src_line = (const uint8_t*)data + src_y * pitch;
                    uint32_t* dst_line = dst + y * display_width;
                    
                    const uint32_t* src_pixels = (const uint32_t*)src_line;
                    for (unsigned x = 0; x < display_width; x++) {
                        unsigned src_x = (x * width) / display_width;
                        uint32_t pixel = src_pixels[src_x];
                        uint8_t r = (pixel >> 16) & 0xFF;
                        uint8_t g = (pixel >> 8) & 0xFF;
                        uint8_t b = pixel & 0xFF;
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

