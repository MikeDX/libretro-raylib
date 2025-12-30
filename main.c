/*
 * libretro_raylib - Libretro Frontend using Raylib
 *
 * Copyright (c) 2024 mikedx
 * GitHub: https://github.com/mikedx/libretro_raylib
 *
 * This file is part of libretro_raylib.
 *
 * libretro_raylib is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * This program uses the libretro API, which is available under the
 * BSD 3-Clause License. See https://www.libretro.com/ for details.
 *
 * This program uses raylib, which is available under the zlib/libpng license.
 * See https://github.com/raysan5/raylib for details.
 */

#include "libretro_frontend.h"
#include "../raylib/src/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//=============================================================================
// Input Mapping
//=============================================================================

/**
 * Maps raylib keyboard key to libretro keycode
 * @param raylib_key Raylib key code (KEY_*)
 * @return Libretro keycode (RETROK_*), or 0 (RETROK_UNKNOWN) if not mapped
 */
static unsigned map_raylib_to_retrok(int raylib_key) {
    // Map raylib keys to libretro keycodes
    // Raylib uses ASCII values for letters/numbers, which match RETROK codes
    if (raylib_key >= KEY_A && raylib_key <= KEY_Z) {
        return (unsigned)(97 + (raylib_key - KEY_A)); // RETROK_a = 97
    }
    if (raylib_key >= KEY_ZERO && raylib_key <= KEY_NINE) {
        return (unsigned)(48 + (raylib_key - KEY_ZERO)); // RETROK_0 = 48
    }
    
    // Special keys mapping
    switch (raylib_key) {
        case KEY_SPACE: return 32; // RETROK_SPACE
        case KEY_ENTER: return 13; // RETROK_RETURN
        case KEY_TAB: return 9; // RETROK_TAB
        case KEY_BACKSPACE: return 8; // RETROK_BACKSPACE
        case KEY_ESCAPE: return 27; // RETROK_ESCAPE
        case KEY_UP: return 273; // RETROK_UP
        case KEY_DOWN: return 274; // RETROK_DOWN
        case KEY_LEFT: return 276; // RETROK_LEFT
        case KEY_RIGHT: return 275; // RETROK_RIGHT
        case KEY_F1: return 282; // RETROK_F1
        case KEY_F2: return 283; // RETROK_F2
        case KEY_F3: return 284; // RETROK_F3
        case KEY_F4: return 285; // RETROK_F4
        case KEY_F5: return 286; // RETROK_F5
        case KEY_F6: return 287; // RETROK_F6
        case KEY_F7: return 288; // RETROK_F7
        case KEY_F8: return 289; // RETROK_F8
        case KEY_F9: return 290; // RETROK_F9
        case KEY_F10: return 291; // RETROK_F10
        case KEY_F11: return 292; // RETROK_F11
        case KEY_F12: return 293; // RETROK_F12
        case KEY_LEFT_SHIFT: return 304; // RETROK_LSHIFT
        case KEY_RIGHT_SHIFT: return 303; // RETROK_RSHIFT
        case KEY_LEFT_CONTROL: return 306; // RETROK_LCTRL
        case KEY_RIGHT_CONTROL: return 305; // RETROK_RCTRL
        case KEY_LEFT_ALT: return 308; // RETROK_LALT
        case KEY_RIGHT_ALT: return 307; // RETROK_RALT
        case KEY_LEFT_SUPER: return 311; // RETROK_LSUPER
        case KEY_RIGHT_SUPER: return 312; // RETROK_RSUPER
        case KEY_APOSTROPHE: return 39; // RETROK_QUOTE
        case KEY_COMMA: return 44; // RETROK_COMMA
        case KEY_MINUS: return 45; // RETROK_MINUS
        case KEY_PERIOD: return 46; // RETROK_PERIOD
        case KEY_SLASH: return 47; // RETROK_SLASH
        case KEY_SEMICOLON: return 59; // RETROK_SEMICOLON
        case KEY_EQUAL: return 61; // RETROK_EQUALS
        case KEY_LEFT_BRACKET: return 91; // RETROK_LEFTBRACKET
        case KEY_BACKSLASH: return 92; // RETROK_BACKSLASH
        case KEY_RIGHT_BRACKET: return 93; // RETROK_RIGHTBRACKET
        case KEY_GRAVE: return 96; // RETROK_BACKQUOTE
        case KEY_DELETE: return 127; // RETROK_DELETE
        case KEY_HOME: return 278; // RETROK_HOME
        case KEY_END: return 279; // RETROK_END
        case KEY_PAGE_UP: return 280; // RETROK_PAGEUP
        case KEY_PAGE_DOWN: return 281; // RETROK_PAGEDOWN
        case KEY_INSERT: return 277; // RETROK_INSERT
        default: return 0; // RETROK_UNKNOWN
    }
}

/**
 * Updates keyboard state for all keys
 * @param frontend Pointer to the libretro frontend instance
 */
static void update_keyboard_input(libretro_frontend_t* frontend) {
    // Update all keyboard keys
    for (int key = 0; key < 512; key++) { // Raylib has keys 0-511
        unsigned retrok = map_raylib_to_retrok(key);
        if (retrok != 0) { // Skip RETROK_UNKNOWN
            libretro_frontend_set_keyboard_key(frontend, retrok, IsKeyDown(key));
        }
    }
}

/**
 * Maps raylib keyboard input to libretro joypad buttons
 * @param frontend Pointer to the libretro frontend instance
 */
static void update_input(libretro_frontend_t* frontend) {
    // Update keyboard state (needed for VICE cores)
    update_keyboard_input(frontend);
    
    // Port 0, Joypad
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_UP, IsKeyDown(KEY_UP) || IsKeyDown(KEY_W));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_A, IsKeyDown(KEY_X));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_B, IsKeyDown(KEY_Z));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_X, IsKeyDown(KEY_C));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_Y, IsKeyDown(KEY_V));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_L, IsKeyDown(KEY_Q));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_R, IsKeyDown(KEY_E));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, IsKeyDown(KEY_TAB));
    libretro_frontend_set_input(frontend, 0, RETRO_DEVICE_ID_JOYPAD_START, IsKeyDown(KEY_ENTER));
}

//=============================================================================
// Main Entry Point
//=============================================================================

/**
 * Main entry point for the libretro frontend
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 on success, 1 on error)
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <path_to_libretro_core.dylib> [rom_file]\n", argv[0]);
        printf("\nExample:\n");
        printf("  %s mgba_libretro.dylib mike_test.gba\n", argv[0]);
        printf("  %s mgba_libretro.dylib\n", argv[0]);
        printf("\nNote: The first argument must be a libretro core (.dylib file), not the executable itself.\n");
        return 1;
    }
    
    const char* core_path = argv[1];
    const char* rom_path = (argc >= 3) ? argv[2] : NULL;
    
    // Initialize frontend
    libretro_frontend_t frontend;
    if (!libretro_frontend_init(&frontend)) {
        fprintf(stderr, "Failed to initialize frontend\n");
        return 1;
    }
    
    // Load core
    if (!libretro_frontend_load_core(&frontend, core_path)) {
        fprintf(stderr, "Failed to load core\n");
        libretro_frontend_deinit(&frontend);
        return 1;
    }
    
    // Initialize core
    if (!libretro_frontend_init_core(&frontend)) {
        fprintf(stderr, "Failed to initialize core\n");
        libretro_frontend_deinit(&frontend);
        return 1;
    }
    
    // Audio stream will be created after ROM loads (when we know the sample rate)
    AudioStream audio_stream = {0};
    bool audio_stream_created = false;
    
    // Load ROM if provided, or initialize in no-game mode
    if (rom_path) {
        if (!libretro_frontend_load_rom(&frontend, rom_path)) {
            fprintf(stderr, "Failed to load ROM\n");
            libretro_frontend_deinit(&frontend);
            return 1;
        }
        // Don't reset immediately - let VICE boot naturally
        // RetroArch doesn't reset after loading, cores handle their own initialization
        fprintf(stderr, "ROM loaded, letting core boot naturally...\n");
    } else {
        // Try to initialize in no-game mode
        // Use libretro_frontend_load_rom with NULL to start without a game
        if (!libretro_frontend_load_rom(&frontend, NULL)) {
            fprintf(stderr, "Failed to start without a game\n");
            libretro_frontend_deinit(&frontend);
            return 1;
        }
    }
    
    // Get video dimensions
    unsigned width, height;
    libretro_frontend_get_video_size(&frontend, &width, &height);
    
    // Initialize raylib window
    int window_width = width * 3;  // Scale up for visibility
    int window_height = height * 3;
    
    // Disable raylib debug output
    SetTraceLogLevel(LOG_NONE);
    
    InitWindow(window_width, window_height, "Libretro Player");
    
    // Set FPS based on core's reported FPS (updated after ROM load)
    // Use the FPS from the frontend which was set by update_av_info
    unsigned target_fps = (unsigned)(frontend.fps > 0 ? frontend.fps + 0.5 : 60.0);
    if (target_fps < 1) target_fps = 60;
    if (target_fps > 120) target_fps = 120; // Cap at reasonable maximum
    SetTargetFPS(target_fps);
    
    // Initialize audio device (must be done before creating streams)
    InitAudioDevice();
    
    // Set larger default buffer size for audio streams to prevent scratchiness
    // This gives more headroom and reduces underruns
    SetAudioStreamBufferSizeDefault(4096);
    
    // Wait a moment for audio device to be ready
    // Then create audio stream now that we know the sample rate
    // Some cores report non-standard rates (like 65536 Hz), so we'll use a standard rate
    // and let the audio system handle it, or clamp to a reasonable value
    // Handle 0 Hz sample rate (use default)
    if (!audio_stream_created) {
        unsigned sample_rate = frontend.audio_sample_rate;
        if (sample_rate == 0) {
            fprintf(stderr, "Warning: Sample rate is 0, using default 44100 Hz\n");
            sample_rate = 44100;
            frontend.audio_sample_rate = 44100;
        }
        
        // Clamp to reasonable values if the core reports something unusual
        if (sample_rate < 8000) sample_rate = 8000;
        if (sample_rate > 192000) sample_rate = 192000;
        
        // Try using the actual sample rate first - modern audio systems should handle it
        // If it's a power-of-2 rate like 65536 or 32768, try it directly
        // If that fails, fall back to 48000 Hz
        unsigned original_rate = sample_rate;
        bool try_original = (sample_rate == 65536 || sample_rate == 32768 || 
                            (sample_rate >= 44100 && sample_rate <= 96000));
        
        if (!try_original && (sample_rate == 65536 || sample_rate == 32768)) {
            sample_rate = 48000;
        }
        
        // Try creating the audio stream with the actual sample rate first
        // Use 32-bit sample size since we're providing float samples
        audio_stream = LoadAudioStream(
            sample_rate,  // Sample rate (try actual rate first)
            32,           // Sample size (bits) - 32 for float samples
            2             // Channels (stereo)
        );
        
        if (IsAudioStreamReady(audio_stream)) {
            PlayAudioStream(audio_stream);
            audio_stream_created = true;
            fprintf(stderr, "Audio initialized: %u Hz, stereo\n", sample_rate);
        } else {
            fprintf(stderr, "Failed to create audio stream at %u Hz\n", sample_rate);
            // If original rate failed and it was unusual, try 48000 Hz fallback
            if (original_rate == 65536 || original_rate == 32768) {
                audio_stream = LoadAudioStream(48000, 32, 2);
                if (IsAudioStreamReady(audio_stream)) {
                    PlayAudioStream(audio_stream);
                    audio_stream_created = true;
                }
            }
        }
    }
    
    // Create texture for rendering
    // First create an empty texture, we'll update it each frame
    Image img = {
        .data = frontend.framebuffer,
        .width = (int)width,
        .height = (int)height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    Texture2D texture = LoadTextureFromImage(img);
    
    // Ensure texture is properly initialized
    if (texture.id == 0) {
        fprintf(stderr, "Failed to create texture\n");
        CloseWindow();
        libretro_frontend_deinit(&frontend);
        return 1;
    }
    
    
    // Audio buffer for streaming
    float audio_buffer[4096 * 2]; // Stereo buffer
    
    // Main loop
    while (!WindowShouldClose()) {
        // Update input
        update_input(&frontend);
        
        // Reset core if R key is pressed (for debugging/recovery)
        if (IsKeyPressed(KEY_R)) {
            fprintf(stderr, "Resetting core...\n");
            libretro_frontend_reset(&frontend);
        }
        
        // Run one frame of the core
        libretro_frontend_run_frame(&frontend);
        
        // Update audio stream - feed audio aggressively to prevent underruns
        // This is critical for smooth audio playback
        if (audio_stream_created) {
            // Feed audio whenever the stream needs it - check multiple times per frame
            // to ensure buffer stays full
            while (IsAudioStreamProcessed(audio_stream)) {
                // Buffer needs refill - read as much as we can to fill it up
                size_t frames_read = libretro_frontend_get_audio_samples(&frontend, audio_buffer, 4096);
                if (frames_read > 0) {
                    UpdateAudioStream(audio_stream, audio_buffer, (int)frames_read);
                } else {
                    // No audio available, break to avoid busy loop
                    break;
                }
            }
        }
        
        
        // Update texture with new frame data
        UpdateTexture(texture, frontend.framebuffer);
        
        // Render
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Calculate centered position maintaining aspect ratio
        float scale = fminf((float)window_width / width, (float)window_height / height);
        int render_width = (int)(width * scale);
        int render_height = (int)(height * scale);
        int render_x = (window_width - render_width) / 2;
        int render_y = (window_height - render_height) / 2;
        
        DrawTexturePro(
            texture,
            (Rectangle){0, 0, (float)width, (float)height},
            (Rectangle){(float)render_x, (float)render_y, (float)render_width, (float)render_height},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
        
        // Draw FPS
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    // Cleanup
    if (audio_stream_created) {
        StopAudioStream(audio_stream);
        UnloadAudioStream(audio_stream);
    }
    CloseAudioDevice();
    UnloadTexture(texture);
    CloseWindow();
    libretro_frontend_deinit(&frontend);
    
    return 0;
}

