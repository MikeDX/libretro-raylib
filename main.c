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
 * Maps raylib keyboard input to libretro joypad buttons
 * @param frontend Pointer to the libretro frontend instance
 */
static void update_input(libretro_frontend_t* frontend) {
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
    printf("Loading core: %s\n", core_path);
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
    
    // Load ROM if provided
    if (rom_path) {
        if (!libretro_frontend_load_rom(&frontend, rom_path)) {
            fprintf(stderr, "Failed to load ROM\n");
            libretro_frontend_deinit(&frontend);
            return 1;
        }
    } else {
        printf("No ROM file provided. Core running without game.\n");
        // Update AV info even without a game
        libretro_frontend_update_av_info(&frontend);
    }
    
    // Get video dimensions
    unsigned width, height;
    libretro_frontend_get_video_size(&frontend, &width, &height);
    
    // Initialize raylib window
    int window_width = width * 3;  // Scale up for visibility
    int window_height = height * 3;
    
    InitWindow(window_width, window_height, "Libretro Player");
    SetTargetFPS(60);
    
    // Initialize audio device (must be done before creating streams)
    InitAudioDevice();
    
    // Set larger default buffer size for audio streams to prevent scratchiness
    // This gives more headroom and reduces underruns
    SetAudioStreamBufferSizeDefault(4096);
    
    // Wait a moment for audio device to be ready
    // Then create audio stream now that we know the sample rate
    // Some cores report non-standard rates (like 65536 Hz), so we'll use a standard rate
    // and let the audio system handle it, or clamp to a reasonable value
    if (!audio_stream_created && frontend.audio_sample_rate > 0) {
        unsigned sample_rate = frontend.audio_sample_rate;
        
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
            printf("Warning: Core reports unusual sample rate %u Hz, trying 48000 Hz instead\n", frontend.audio_sample_rate);
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
            printf("Audio initialized: %u Hz, stereo\n", sample_rate);
            audio_stream_created = true;
        } else {
            // If original rate failed and it was unusual, try 48000 Hz fallback
            if (original_rate == 65536 || original_rate == 32768) {
                fprintf(stderr, "Warning: Failed to initialize audio stream at %u Hz. Trying 48000 Hz instead.\n", sample_rate);
                audio_stream = LoadAudioStream(48000, 32, 2);
                if (IsAudioStreamReady(audio_stream)) {
                    PlayAudioStream(audio_stream);
                    printf("Audio initialized: 48000 Hz (fallback), stereo\n");
                    audio_stream_created = true;
                } else {
                    fprintf(stderr, "Error: Failed to initialize audio stream at any rate\n");
                }
            } else {
                fprintf(stderr, "Error: Failed to initialize audio stream at %u Hz\n", sample_rate);
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
    
    printf("\nControls:\n");
    printf("  Arrow Keys / WASD - D-Pad\n");
    printf("  X - A button\n");
    printf("  Z - B button\n");
    printf("  C - X button\n");
    printf("  V - Y button\n");
    printf("  Q - L button\n");
    printf("  E - R button\n");
    printf("  TAB - Select\n");
    printf("  ENTER - Start\n");
    printf("  ESC - Exit\n\n");
    
    // Audio buffer for streaming
    float audio_buffer[4096 * 2]; // Stereo buffer
    
    // Main loop
    while (!WindowShouldClose()) {
        // Update input
        update_input(&frontend);
        
        // Run one frame of the core
        libretro_frontend_run_frame(&frontend);
        
        // Update audio stream - feed audio every frame to prevent underruns
        // This is critical for smooth audio playback
        if (audio_stream_created) {
            // Feed audio whenever the stream needs it
            // UpdateAudioStream can be called even when not processed, but we check first
            // to avoid unnecessary work
            if (IsAudioStreamProcessed(audio_stream)) {
                // Buffer needs refill - read as much as we can to fill it up
                size_t frames_read = libretro_frontend_get_audio_samples(&frontend, audio_buffer, 4096);
                if (frames_read > 0) {
                    UpdateAudioStream(audio_stream, audio_buffer, (int)frames_read);
                }
            }
            // Note: We don't feed when not processed to avoid overfilling the stream buffer
            // The ring buffer provides enough buffering to handle timing variations
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

